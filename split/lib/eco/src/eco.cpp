/*
   Copyright 2022-2024, Adam Krzywaniak.

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

#include <iostream>
#include <iomanip>
#include "eco.hpp"
#include <sys/wait.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include "plot_builder.hpp"
#include "logging/log.hpp"

#include <atomic>
#include <filesystem>

namespace fs = std::filesystem;
std::atomic<bool> external_trigger_flag(false);
std::atomic<bool> stop_flag(false);
const std::string trigger_file_path = "/tmp/trigger_file";

void monitor_trigger_file() {
    auto last_write_time = fs::last_write_time(trigger_file_path);

    while (!stop_flag.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (fs::exists(trigger_file_path)) {
            auto current_write_time = fs::last_write_time(trigger_file_path);
            if (current_write_time != last_write_time) {
                last_write_time = current_write_time;
                external_trigger_flag.store(true);
            }
        }
    }
}

static constexpr char FLUSH_AND_RETURN[] = "\r                                                                                     \r";

Eco::Eco(std::shared_ptr<Device> d) :
    device_(d), devStateGlobal_(d), trigger_(cfg_), logger_(d->getDeviceTypeString())
{
    defaultWatchdog = readWatchdog();
    if (defaultWatchdog == WatchdogStatus::ENABLED)
    {
        modifyWatchdog(WatchdogStatus::DISABLED);
    }
    device_->reset();
}

Eco::~Eco() {
    device_->restoreDefaultLimits();
    modifyWatchdog(defaultWatchdog);
}

static inline int readLimitFromFile (std::string fileName) {
    std::ifstream limitFile (fileName.c_str());
    std::string line;
    int limit = -1;
    if (limitFile.is_open()){
        while ( getline (limitFile, line) ){
            limit = atoi(line.c_str());
        }
        limitFile.close();
    } else {
        std::cerr << "cannot read the limit file: " << fileName << "\n"
                  << "file not open\n";
    }
    return limit;
}

static inline void writeLimitToFile (std::string fileName, int limit) {
    std::ofstream outfile (fileName.c_str(), std::ios::out | std::ios::trunc);
    if (outfile.is_open()){
        outfile << limit;
    } else {
        std::cerr << "cannot write the limit to file " << fileName << "\n"
                  << "file not open\n";
    }
    outfile.close();
}

std::vector<int> Eco::prepareListOfPowerCapsInMicroWatts(/*Domain dom*/) { // domain is unused, probably to be removed
    std::vector<int> powerLimitsVec;

    const auto minmax = device_->getMinMaxLimitInWatts();
    unsigned lowPowLimit_uW = minmax.first * 1000000;
    unsigned highPowLimit_uW = minmax.second * 1000000;

    int step = ((highPowLimit_uW - lowPowLimit_uW)/ 100) * cfg_.percentStep_;
    for (int limit_uW = highPowLimit_uW; limit_uW >= lowPowLimit_uW; limit_uW -= step) {
        powerLimitsVec.push_back(limit_uW);
    }
    std::cout << "Vector generated, length: " << powerLimitsVec.size() << "\n";
    return powerLimitsVec;
}

using MS = std::chrono::milliseconds;


void Eco::singleAppRunAndPowerSample(char* const* argv) {
    devStateGlobal_.resetState();

    int fd = open("EP_stdout.txt", O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if (fd < 0)
    {
        perror("open");
        std::abort();
    }
    pid_t childProcId = fork();

    if (childProcId < 0)
    {
        std::cerr << "fork failed\n";
        perror("fork");
        close(fd);
        return;
    }

    if (childProcId == 0) {
        // Child process
        int ret = mainAppProcess(argv, fd);
        std::exit(ret);
    }
    // Parent process
    int status;
    pid_t result;
    while (true) {
        result = waitpid(childProcId, &status, WNOHANG);
        if (result == 0) {
            // child alive - monitored app is running
            usleep(cfg_.msPause_ * 1000);
            devStateGlobal_.sample();
            logger_.logPowerLogLine(devStateGlobal_, devStateGlobal_.getCurrentPowerAndPerf());
        } else if (result == -1) {
            // waitpid error
            perror("waitpid");
            break;
        } else {
            // child finished
            if (WIFEXITED(status)) {
                int exitCode = WEXITSTATUS(status);
                if (exitCode != 0)
                {
                    // child process failed for some reason so we may stop the STEP application
                    std::cout << "Terminating StEP due to unsuccesful monitored app execution (exit code: " << exitCode << ")\n";
                    std::exit(exitCode);
                }
            } else if (WIFSIGNALED(status)) {
                int sig = WTERMSIG(status);
                std::cout << "Child was killed by signal " << sig << "\n";
            } else {
                std::cout << "Child ended unexpectedly\n";
            }
            break;
        }
    }

    close(fd);
}

PowAndPerfResult Eco::checkPowerAndPerformance(int usPeriod)
{
    auto pause = cfg_.msPause_ * 1000;
    usleep(pause);
    devStateGlobal_.sample();
    auto resultAccumulator = devStateGlobal_.getCurrentPowerAndPerf(trigger_);
    while (usPeriod > pause){
        usleep(pause);
        devStateGlobal_.sample();
        auto tmp = devStateGlobal_.getCurrentPowerAndPerf(trigger_);
        logger_.logPowerLogLine(devStateGlobal_, tmp);
        resultAccumulator += tmp;
        usPeriod -= pause;
    }

    return resultAccumulator;
}

// TODO: try to exclude printing from ECO
static inline
void printLine() {
    std::cout << "-----------------------------------------------"
              << "---------------------------------------------\n"; 
}

static inline
void printHeader() {
    printLine();
    std::cout << "Cap\tE\tP\tfilterP\tips\tmin(E)\tmin(Et)\tEDS\n";
    printLine();
}

static inline
void logCurrentRangeGSS(int a, int leftCandidate, int rightCandidate, int b) {
    std::cout << "--------------------------------\n"
              << "Current GSS range: |"
              << a/1000000 << " "
              << leftCandidate/1000000 << " "
              << rightCandidate/1000000 << " "
              << b/1000000 << "|\n"
              << "--------------------------------\n";
}

void Eco::reportResult(double waitTime, double testTime) {
    auto&& totalE = devStateGlobal_.getEnergySinceReset();
    auto&& totalTime = devStateGlobal_.getTimeSinceReset<std::chrono::milliseconds>() / 1000.0;
    std::cout << "Total E: " << totalE;
    // ----------------------------------------------------------------------------------------
    // Below code is temporary disabled as it uses method specific to some
    // class of systems with two Intel CPUs treated as a single device.
    // Ultimately the Device shall represent single CPU and the energy readings
    // shall be combined in DeviceStateAccumulator. If a system consist of
    // multiple CPUs used by the same application it shall be reflected as multiple
    // Device object instances within DeviceStateAccumulator.
    //
    //  || || || || || || || || || || || || || || || || ||
    //  \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/
    //
    // auto&& energyVec = devStateGlobal_.getTotalEnergyVec(Domain::PKG);
    // int pkgID = 0;
    // for (auto&& pkgE : energyVec) {
    //     pkgID++;
    //     std::cout << "\nPKG" << pkgID << ": " << pkgE << ", (" << (pkgE/totalE)*100 << "%)";
    // }
    // ----------------------------------------------------------------------------------------
    std::cout << "\nTotal P: " << totalE / totalTime <<
                 "\nTotal t: " << totalTime << "s\n";
    if (waitTime != 0.0 || testTime != 0.0) {
        std::cout << "Wait time: " << waitTime << "s, (" << (waitTime/totalTime)*100 << "%)\n"
                  << "Test time: " << testTime << "s, (" << (testTime/totalTime)*100 << "%)\n";
    }
}

void Eco::waitForTuningTrigger(int& status, int childPID) {
    waitpid(childPID, &status, WNOHANG);
    while ((!trigger_.isDeviceReadyForTuning()) && status)
    {
        auto papResult = checkPowerAndPerformance(cfg_.usTestPhasePeriod_);
        // std::cout << FLUSH_AND_RETURN
        //         << logCurrentResultLine(papResult, papResult, cfg_.k_, true /* no new line */);

    }
    // std::cout << "\n";
    printLine();
}

void Eco::execPhase(
    int powerCap_uW,
    int& status,
    int childPID,
    PowAndPerfResult& refResult)
{
    int repetitionPeriodInUs = cfg_.repeatTuningPeriodInSec_ * 1e6 + cfg_.usTestPhasePeriod_;
    device_->setPowerLimitInMicroWatts(powerCap_uW);
    printLine();
    while (status && repetitionPeriodInUs > 0)
    {
        auto papResult = checkPowerAndPerformance(cfg_.usTestPhasePeriod_);
        repetitionPeriodInUs = trigger_.isTuningPeriodic() ? repetitionPeriodInUs - cfg_.usTestPhasePeriod_ : repetitionPeriodInUs;

        logger_.logPowerLogLine(devStateGlobal_, papResult, refResult);
        waitpid(childPID, &status, WNOHANG);
        if (external_trigger_flag.load())
        {
            std::cout << "[INFO] External trigger received during execution phase. Re-tuning parameters...\n";
            external_trigger_flag.store(false);
            break;
        }
    }
    std::cout << "\n";
    printLine();
}

int& Eco::adjustHighPowLimit(PowAndPerfResult firstResult, int& currHighLimit_uW)
{
    // // check if default power cap is higher than max power cap (TDP)
    // if (currHighLimit_uW < dynamic_cast<IntelDevice*>(device_.get())->getDefaultCaps().defaultConstrPKG_->longPower)
    // {
    //     currHighLimit_uW = dynamic_cast<IntelDevice*>(device_.get())->getDefaultCaps().defaultConstrPKG_->longPower;
    // // if (currHighLimit_uW < defaultPowerLimitInWatts_) {
    // //     currHighLimit_uW = defaultPowerLimitInWatts_;
    // }
    // reduce starting high power limit according to first result's average power
    if ((firstResult.averageCorePowerInWatts_ * 1000000) < (currHighLimit_uW/2)) {
        currHighLimit_uW = (firstResult.averageCorePowerInWatts_ * 1000000) * 2.0;
    }
    return currHighLimit_uW;
}

int Eco::mainAppProcess(char* const* argv, int& stdoutFileDescriptor)
{
    if (dup2(stdoutFileDescriptor, 1) < 0) {
        perror("dup2");
        abort();
    }
    close(stdoutFileDescriptor);

    int execStatus = execvp(argv[1], argv+1);
    validateExecStatus(execStatus);
    return execStatus;
}

FinalPowerAndPerfResult Eco::runAppWithSearch(
    char* const* argv,
    TargetMetric targerMetric,
    SearchType searchType,
    int argc)
{
    // this is redirecting the original output of the tuned application to txt file
    int fd = open("redirected.txt", O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if (fd < 0) { perror("open"); abort(); }
    // ----------------------------------------------------------------------------
    if (!fs::exists(trigger_file_path))
    {
        std::ofstream trigger_file(trigger_file_path);
        trigger_file.close();
    }
    try
    {
        fs::permissions(trigger_file_path,  fs::perms::owner_read | fs::perms::owner_write |
                                            fs::perms::group_read | fs::perms::group_write |
                                            fs::perms::others_read | fs::perms::others_write);
    }
    catch (const fs::filesystem_error& e)
    {
        std::cerr << "Failed to change file permissions: " << e.what() << "\n";
    }
    std::thread monitor_thread(monitor_trigger_file);
    // ----------------------------------------------------------------------------
    devStateGlobal_.resetState();

    double waitTime = 0.0, testTime = 0.0;
    int bestResultCapInMicroWatts = -1;
    pid_t childProcId = fork();
    if (childProcId >= 0) //fork successful
    {
        if (childProcId == 0) // child process
        {
            mainAppProcess(argv, fd);
        }
        else  // parent process
        {
            int status = 1;
            printHeader();
            waitTime = measureDuration([&, this] {
                waitForTuningTrigger(status, childProcId);
            });
            //----------------------------------------------------------------------------
            Algorithm algorithm;
            if (searchType == SearchType::LINEAR_SEARCH)
            {
                algorithm = LinearSearchAlgorithm();
            }
            else
            {
                algorithm = GoldenSectionSearchAlgorithm();
            }
            //----------------------------------------------------------------------------
            PowAndPerfResult referenceRun;
            while (status)
            {
                testTime += measureDuration([&, this] {
                    referenceRun = checkPowerAndPerformance(cfg_.referenceRunMultiplier_ * cfg_.usTestPhasePeriod_);
                    logger_.logPowerLogLine(devStateGlobal_, referenceRun);
                    bestResultCapInMicroWatts = algorithm(device_, devStateGlobal_, trigger_, targerMetric, referenceRun, status, childProcId, cfg_.msPause_, cfg_.msTestPhasePeriod_, logger_);
                });
                execPhase(bestResultCapInMicroWatts, status, childProcId, referenceRun);
                device_->restoreDefaultLimits();
            }
        }
    }
    else
    {
        std::cerr << "fork failed" << std::endl;
        // TODO: handle errors
        // return 1;
    }
    reportResult(waitTime, testTime);
    double totalTimeInSeconds = devStateGlobal_.getTimeSinceReset<std::chrono::milliseconds>() / 1000.0;
    std::cout << "[INFO] actual total time " << totalTimeInSeconds << "\n";
    stop_flag.store(true);
    monitor_thread.join();


    return FinalPowerAndPerfResult(bestResultCapInMicroWatts / 1.0e6,
                                devStateGlobal_.getEnergySinceReset(),
                                devStateGlobal_.getEnergySinceReset() / totalTimeInSeconds,
                                // devStateGlobal_.getEnergySinceReset(Domain::PP0) / totalTimeInSeconds,
                                // devStateGlobal_.getEnergySinceReset(Domain::PP1) / totalTimeInSeconds,
                                // devStateGlobal_.getEnergySinceReset(Domain::DRAM) / totalTimeInSeconds,
                                0.0,
                                0.0,
                                0.0,
                                TimeResult(totalTimeInSeconds, waitTime, testTime),
                                0.0,
                                0.0);
}


FinalPowerAndPerfResult Eco::runAppWithSampling(char* const* argv, int argc) {
    singleAppRunAndPowerSample(argv);
    reportResult();
    double totalTimeInSeconds = devStateGlobal_.getTimeSinceReset<std::chrono::milliseconds>() / 1000.0;

    return FinalPowerAndPerfResult(device_->getPowerLimitInWatts(),
                                devStateGlobal_.getEnergySinceReset(),
                                devStateGlobal_.getEnergySinceReset() / totalTimeInSeconds,
                                // devStateGlobal_.getEnergySinceReset(Domain::PP0) / totalTimeInSeconds,
                                // devStateGlobal_.getEnergySinceReset(Domain::PP1) / totalTimeInSeconds,
                                // devStateGlobal_.getEnergySinceReset(Domain::DRAM) / totalTimeInSeconds,
                                0.0,
                                0.0,
                                0.0,
                                totalTimeInSeconds,
                                devStateGlobal_.getPerfCounterSinceReset(),
                                0.0 // num of cycles is not needed, so might be removed in the future
                                );
}

FinalPowerAndPerfResult Eco::multipleAppRunAndPowerSample(char* const* argv, int numIterations, std::optional<std::reference_wrapper<std::stringstream>> stream) {
    FinalPowerAndPerfResult sum;
    for(auto i = 0; i < cfg_.numIterations_; i++) {
        const auto tmp = runAppWithSampling(argv);
        if (stream.has_value())
        {
            stream.value().get() << "# " << std::fixed << std::setprecision(3) << tmp << "\n";
        }
        sum += tmp;
    }
    std::cout << FLUSH_AND_RETURN;
    sum /= cfg_.numIterations_;
    return sum;
}

WatchdogStatus Eco::readWatchdog() {
    if (readLimitFromFile("/proc/sys/kernel/nmi_watchdog") > 0) {
        return WatchdogStatus::ENABLED;
    } else {
        return WatchdogStatus::DISABLED;
    }
}

void Eco::modifyWatchdog(WatchdogStatus status) {
    std::ofstream watchdog ("/proc/sys/kernel/nmi_watchdog", std::ios::out | std::ios::trunc);
    watchdog << (status == WatchdogStatus::ENABLED) ? "1" : "0";
    watchdog.close();
}

void Eco::plotPowerLog(std::optional<FinalPowerAndPerfResult> results, std::string appCommand, bool plotDynamicMetrics)
{
    logger_.flush();
    const auto f = logger_.getPowerFileName();
    // std::string imgFileName = outPowerFileName_;
    std::cout << "Processing " << f << " file...\n";
    std::string imgFileName = f;
    PlotBuilder p(imgFileName.replace(imgFileName.end() - 3,
                                      imgFileName.end(),
                                      "png"));
    p.setPlotTitle(device_->getDeviceTypeString() + " power log: " + device_->getName() + " running " + appCommand, 16);
    std::stringstream ss;
    if(results)
    {
        ss << std::fixed << std::setprecision(3)
        << "Total E: " << results->energy << " J"
        << "    Total time: " << results->time_.totalTime_ << " s"
        << "    av. Power: " << results->pkgPower << " W";
        // << "    last powercap: " << results->powercap << " W";
    }
    Series powerCap (f, 1, 2, "P cap [W]");
    Series currPower (f, 1, 3, "P[W]");
    Series smaPower (f, 1, 4, "SMA P[W]");
    Series currENG (f, 1, 7, "ENG");
    Series currEDP (f, 1, 8, "EDP");
    Series currPERF (f, 1, 9, "PERF");

    if (!plotDynamicMetrics)
    {
        p.setSimpleSubtitle(ss.str(), 12);
        p.plotPowerLog({powerCap, currPower,smaPower});
    }
    else
    {
        Series dyn_en (f, 1, 11, "Dyn. EN");
        Series dyn_edp (f, 1, 12, "Dyn. EDP");
        Series dyn_eds (f, 1, 13, "Dyn. EDS");
        Series dyn_perf (f, 1, 10, "Dyn. Perf");
        p.setSimpleSubtitle(ss.str(), 12, 0.92);
        p.plotPowerLogWithDynamicMetrics(
            {powerCap, currPower,smaPower},
            {dyn_en, dyn_edp, dyn_eds, dyn_perf});
    }
}

void Eco::staticEnergyProfiler(char* const* argv, int argc)
{
    std::vector<FinalPowerAndPerfResult> resultsVec;
    const auto&& warmup = runAppWithSampling(argv);
    std::stringstream stream;
    stream << "# examined application: ";
    for (int i=1; i<argc; i++) {
        stream << argv[i] << " ";
    }
    stream << "\n";
    stream << "# P_cap\tE\tP_av\ttime\tEDP\tdE\tdt\t%dE\t%dt\tP/(cycl/s)\n";
    stream << "# [W]\t[J]\t[W]\t[s]\t[Js]\t[J]\t[s]\t[%J]\t[%s][(cycl)/J]\t[(cycl/s)^2/W)]\n";

    stream << "# " << std::fixed << std::setprecision(3) << warmup << "\n";
    stream << "# warmup done #\n";

    FinalPowerAndPerfResult reference;
    for(auto i = 0; i < cfg_.numIterations_; i++)
    {
        auto tmp = runAppWithSampling(argv);
        reference += tmp;
        stream << "# " << std::fixed << std::setprecision(3) << tmp << "\n";
    }
    reference /= cfg_.numIterations_;
    resultsVec.push_back(reference);
    stream << reference << "\n";
    auto powerLimitsVec = prepareListOfPowerCapsInMicroWatts();
    for (auto& currentLimit : powerLimitsVec) {
        device_->setPowerLimitInMicroWatts(currentLimit);
        auto avResult = multipleAppRunAndPowerSample(argv, cfg_.numIterations_, stream);
        auto k = getK();
        auto mPlus = EnergyTimeResult(avResult.energy,
                                        avResult.time_.totalTime_,
                                        avResult.pkgPower).checkPlusMetric(reference.getEnergyAndTime(), k);
        auto mPlusDynamic = (1.0/k) * (reference.getInstrPerSec() / avResult.getInstrPerSec()) *
                            ((k-1.0) * (avResult.getEnergyPerInstr() / reference.getEnergyPerInstr()) + 1.0);
        auto&& timeDelta = avResult.time_.totalTime_ - reference.time_.totalTime_;
        resultsVec.emplace_back((double)currentLimit / 1000000,
                                 avResult.energy,
                                 avResult.pkgPower,
                                 avResult.pp0power,
                                 avResult.pp1power,
                                 avResult.dramPower,
                                 avResult.time_.totalTime_,
                                 avResult.inst,
                                 avResult.cycl,
                                 avResult.energy - reference.energy,
                                 timeDelta,
                                 100 * (avResult.energy - reference.energy) / reference.energy,
                                 100 * (timeDelta) / reference.time_.totalTime_,
                                 mPlus);
        stream << resultsVec.back() << "\t" << mPlusDynamic << "\n";
        if (resultsVec.back().relativeDeltaT > (double)cfg_.perfDropStopCondition_) {
            break;
        }
    }
    stream << "# PowerCap for: min(E): "
            << std::min_element(resultsVec.begin(),
                                resultsVec.end(),
                                CompareFinalResultsForMinE())->powercap
            << " W, "
            << "min(Et): "
            << std::min_element(resultsVec.begin(),
                                resultsVec.end(),
                                CompareFinalResultsForMinEt())->powercap
            << " W, "
            << "min(M+): "
            << std::min_element(resultsVec.begin(),
                                resultsVec.end(),
                                CompareFinalResultsForMplus())->powercap
            << " W.\n";
    logger_.logToResultFile(stream);
}