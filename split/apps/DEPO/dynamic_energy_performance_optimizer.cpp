/*
   Copyright 2022, Adam Krzywaniak.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "eco.hpp"
#include "devices/cuda_device.hpp"
#include "devices/intel_device.hpp"

#include "data_structures/results_container.hpp"
#include <boost/program_options.hpp>
#include <optional>

namespace po = boost::program_options;

inline std::pair<TargetMetric, SearchType>
parseArgs(po::variables_map& map)
{
    auto search = SearchType::LINEAR_SEARCH;
    auto metric = TargetMetric::MIN_E;

    if (map.count("no-tuning"))
    {
        std::cout << "Running application with power and energy consumption monitoring only.\n";
    }
    else
    {
        if (map.count("gss"))
        {
            map.erase("gss");
            std::cout << "Using Golden Section Search algorithm as selected.\n";
            search = SearchType::GOLDEN_SECTION_SEARCH;
        }
        else if (map.count("ls"))
        {
            map.erase("ls");
            std::cout << "Using Linear Search algorithm as selected.\n";
            search = SearchType::LINEAR_SEARCH;
        }
        else
        {
            std::cout << "Using Linear Search algorithm by default.\n";
        }
        if (map.count("en"))
        {
            map.erase("en");
            std::cout << "Using ENERGY metric as selected.\n";
            metric = TargetMetric::MIN_E;
        }
        else if (map.count("edp"))
        {
            map.erase("edp");
            std::cout << "Using ENERGY DELAY PRODUCT metric as selected.\n";
            metric = TargetMetric::MIN_E_X_T;
        }
        else if (map.count("eds"))
        {
            map.erase("eds");
            std::cout << "Using ENERGY DELAY SUM metric as selected.\n";
            metric = TargetMetric::MIN_M_PLUS;
        }
        else
        {
            std::cout << "Using ENERGY metric by default.\n";
        }
    }
    return std::make_pair(metric, search);
}

std::optional<int> checkIfDeviceTypeIsGPU(po::variables_map& map)
{
    std::optional<int> gpuID = std::nullopt;
    if (map.count("gpu"))
    {
        gpuID = map["gpu"].as<int>();
        map.erase("gpu");
        std::cout << "Using GPU with ID=" << gpuID.value() << " backend for NVIDIA optimization.\n";
    }
    return gpuID;
}

void cleanArgv(int& argc, char* argv[])
{
    for (int idx = 1; idx < argc;)
    {
        const auto flag = std::string(argv[idx]);
        if (flag == "--ls"  ||
            flag == "--gss" ||
            flag == "--en"  ||
            flag == "--edp" ||
            flag == "--eds" ||
            flag == "--no-tuning" ||
            std::string(flag).substr(0,6) == "--gpu="
            )
        {
            for (int i = 1; i < argc -1; i++)
            {
                argv[i] = argv[i+1];
            }
            argv[argc-1] = nullptr;
            argc--;
        }
        else if (flag == "--gpu")
        {
            // erase two args: the flag and the value
            for (int i = 1; i < argc -2; i++)
            {
                argv[i] = argv[i+2];
            }
            argv[argc-1] = nullptr;
            argv[argc-2] = nullptr;
            argc-=2;
        }
        else
        {
            idx++;
        }
    }
}


static inline std::string readPathInfo()
{
    std::fstream tmpfile;
    tmpfile.open("/tmp/depo_gpu_path", std::ios::in);
    if (tmpfile.is_open()) {
        std::string path;
        std::stringstream ss;
        while (getline(tmpfile, path))
        {
            ss << path << "\n";
        }
        tmpfile.close();
        std::string tmp = ss.str();
        tmp.erase(std::remove(tmp.begin(), tmp.end(), '\n'), tmp.cend());
        return tmp;
    }
    return "";
}


int main (int argc, char *argv[])
{
    // temporary fix
    std::ofstream watchdog ("/proc/sys/kernel/nmi_watchdog", std::ios::out | std::ios::trunc);
	watchdog << "0";
	watchdog.close();

    auto search = SearchType::LINEAR_SEARCH;
    auto metric = TargetMetric::MIN_E;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("gss", "use Golden Section Search algorithm")
        ("ls", "use Linear search algorithm")
        ("en", "use Energy metric")
        ("edp", "use Energy Delay Product metric")
        ("eds", "use Energy SumDelay  metric")
        ("no-tuning", "run app only checking the power and energy consumption")
        ("gpu", po::value<int>(), "use GPU backend for card with specified ID")
    ;
    po::variables_map optionsMap;
    po::parsed_options parsed = po::command_line_parser(argc, argv)
                                    .options(desc)
                                    .allow_unregistered()
                                    .run();

    po::store(parsed, optionsMap);
    po::notify(optionsMap);

    // Get the rest of the arguments
    std::vector<std::string> unrecognizedArgs = po::collect_unrecognized(parsed.options, po::include_positional);

    std::vector<std::string> fullArgs;
    for (const auto& arg : unrecognizedArgs) {
        fullArgs.push_back(arg);
    }

    // Copy to char* array
    std::vector<char*> newArgv;
    newArgv.push_back(argv[0]);  // Keep ./DEPO as argv[0]
    for (auto& arg : fullArgs) {
        newArgv.push_back(&arg[0]);
    }
    newArgv.push_back(nullptr);  // null-terminate

    argc = static_cast<int>(newArgv.size() - 1);
    argv = newArgv.data();

    // print help and exit
    if (optionsMap.count("help"))
    {
        std::cout << desc << "\n";
        return 1;
    }
    // read metric and search algorithm
    std::tie(metric, search) = parseArgs(optionsMap);
    std::optional<int> gpuID = checkIfDeviceTypeIsGPU(optionsMap);
    //cleanArgv(argc, argv);


    std::shared_ptr<Device> device;
    if (gpuID.has_value())
    {
        device = std::make_shared<CudaDevice>(gpuID.value());

        int e1 = setenv("INJECTION_KERNEL_COUNT", "1", 1);
        std::string path = readPathInfo();

        if (path == "")
        {
            std::cerr << "`/tmp/depo_gpu_path` is empty. You should probably run `./build.sh` in `split/profiling_injection` directory.";
            std::cerr << "\nClosing DEPO called for GPU backend.\n";
            return 1;
        }
        path = path + "/libinjection_2.so";

        int e2 = setenv("CUDA_INJECTION64_PATH", path.c_str(), 1);
        std::cout << "ENV1 status: " << e1 << ", value: " << getenv("INJECTION_KERNEL_COUNT")
                  << "\nENV2 status: " << e2 << ", value: " << getenv("CUDA_INJECTION64_PATH") << "\n";
    }
    else
    {
        device = std::make_shared<IntelDevice>();
    }

    std::unique_ptr<Eco> eco = std::make_unique<Eco>(device);
    std::stringstream ssout;
    std::stringstream applicationCommand;
    for (int i=1; i<argc; i++) {
        applicationCommand <<  argv[i] << " ";
        std::cout <<  argv[i] << " ";
    }
    ssout << "# " << applicationCommand.str() << "\n";

    FinalPowerAndPerfResult result;
    bool printPowerLogWithDynamicMetrics = true;
    if (optionsMap.count("no-tuning"))
    {
        result = eco->runAppWithSampling(argv, argc);
        printPowerLogWithDynamicMetrics = false;
    }
    else
    {
        result = eco->runAppWithSearch(argv, metric, search, argc);
    }
    ssout << std::fixed << std::setprecision(3)
         << "# Energy[J]\ttime[s]\tPower[W]\n"
         << result.energy << "\t" << result.time_.totalTime_ << "\t" << result.pkgPower <<"\n";

    eco->logToResultFile(ssout);
    eco->plotPowerLog(result, applicationCommand.str(), printPowerLogWithDynamicMetrics);

    if (gpuID.has_value())
    {
        unsetenv("INJECTION_KERNEL_COUNT");
        unsetenv("CUDA_INJECTION64_PATH");
    }

	return 0;
}