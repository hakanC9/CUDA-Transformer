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
#include "devices/intel_device.hpp"
#include "plot_builder.hpp"
#ifdef WITH_XPU
#include "devices/xpu_device.hpp"
const char devcmd[] = "--xpu=";
using TargetDevice  = XPUDevice;
#else
#include "devices/cuda_device.hpp"
const char devcmd[] = "--gpu=";
using TargetDevice  = CudaDevice;
#endif

#include <cstdlib>

int main(int argc, char* argv[])
{
    // temporary fix
    std::ofstream watchdog("/proc/sys/kernel/nmi_watchdog", std::ios::out | std::ios::trunc);
    watchdog << "0";
    watchdog.close();

    bool isGpuOrXpu = false;
    int  gpuID = -1;
    if (argc >= 1)
    {
        if (std::string(argv[1]).substr(0, 6) == devcmd)
        {
            gpuID = stoi(std::string(argv[1]).substr(6, 1));
            // remove the --gpu flag from 1st arg
            for (int i = 1; i < argc - 1; i++)
            {
                argv[i] = argv[i + 1];
            }
            argc--;
            isGpuOrXpu = true;
        }
    }


    std::shared_ptr<Device> device;
    if (!isGpuOrXpu)
    {
        device = std::make_shared<IntelDevice>();
    }
    else
    {
        #ifdef WITH_XPU
        bool        useAmperes = true;
        const char* env_p      = std::getenv("USE_AMPERES");

        if (env_p != nullptr)
        {
            std::string env_value(env_p);
            if (env_value == "0" || env_value == "False" || env_value == "false")
            {
                useAmperes = false;
            }
        }
        device = std::make_shared<TargetDevice>(gpuID, useAmperes);
        #else //GPU
        device = std::make_shared<TargetDevice>(gpuID);
        #endif
    }
    std::unique_ptr<Eco> eco = std::make_unique<Eco>(device);

    eco->staticEnergyProfiler(argv, argc);

    eco->plotPowerLog(std::nullopt);

    // Plot result file automatically
    std::string imgFileName = eco->getResultFileName();
    PlotBuilder p(imgFileName.replace(imgFileName.end() - 4, imgFileName.end(), "_Et.png"));
    p.setPlotTitle(eco->getDeviceName());
    std::cout << "Processing " << eco->getResultFileName() << " file...\n";
    p.plotEPet(eco->getResultFileName());
    p.submitPlot();

    // Plot result file automatically
    std::string imgFileName2 = eco->getResultFileName();
    PlotBuilder p2(imgFileName2.replace(imgFileName2.end() - 3, imgFileName2.end(), "png"));
    p2.setPlotTitle(eco->getDeviceName());
    std::cout << "Processing " << eco->getResultFileName() << " file...\n";
    p2.plotEPall(eco->getResultFileName());
    p2.submitPlot();

    return 0;
}
