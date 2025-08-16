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

#pragma once

#include <chrono>
#include <memory>
#include <set>
#include <optional>
#include <functional>
// Workaround: below two has to be included in such order to ensure no warnings
//             about macro redefinitions. Some of the macros in Rapl.hpp are already
//             defined in types.h included by cpucounters.h but not all of them.
#include "device_state.hpp"
//----------------------------------------------------------------------------------
#include "algorithms/linear_search.hpp"
#include "algorithms/golden_section_search.hpp"
#include "data_structures/power_and_perf_result.hpp"
#include "eco_constants.hpp"
#include "data_structures/final_power_and_perf_result.hpp"
#include "params_config.hpp"
#include "data_structures/data_filter.hpp"
#include "logging/both_stream.hpp"
#include "logging/log.hpp"
#include "trigger.hpp"


template <class F>
inline auto measureDuration(F&& fun)
{
    const auto start = std::chrono::high_resolution_clock::now();
    fun();
    return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
}

static inline
void validateExecStatus(int status) {
    if (status) {
        std::cerr << "execv failed with error " << errno
                  << " " << strerror(errno) << std::endl;
    }
}

using Algorithm = std::function<unsigned(
  std::shared_ptr<Device>,
  DeviceStateAccumulator&,
  Trigger&,
  TargetMetric,
  const PowAndPerfResult&,
  int&,
  int,
  int,
  int,
  Logger&)>;

class Eco
{
  public:
    FinalPowerAndPerfResult runAppWithSampling(char* const*, int = 1);
    FinalPowerAndPerfResult runAppWithSearch(
      char* const*,
      TargetMetric,
      SearchType,
      int = 1);
    void plotPowerLog(std::optional<FinalPowerAndPerfResult>, std::string = "", bool=false);
    std::string getDeviceName() const { return device_->getName(); }

    void staticEnergyProfiler(char* const* argv, int argc);

    Eco() = delete;
    Eco(std::shared_ptr<Device>);
    virtual ~Eco();
    std::string getResultFileName() const { return logger_.getResultFileName(); }
    void logToResultFile(std::stringstream& ss) { logger_.logToResultFile(ss); }

    double getK() { return cfg_.k_; } // temporary getter until Eco is reorganised
    void setCustomK(double k) { cfg_.k_ = k; } // temporary setter until Eco is reorganised
    int getNumIterations() { return cfg_.numIterations_; }

  protected:
  private:
    ParamsConfig cfg_; // stores defaults values of params or reads it from config.yaml
    Trigger trigger_;
    std::shared_ptr<Device> device_;
    CrossDomainQuantity idleAvPow_;

    DeviceStateAccumulator devStateGlobal_;
    std::vector<FinalPowerAndPerfResult> fullAppRunResultsContainer_;
    Logger logger_;

    WatchdogStatus defaultWatchdog;
    void modifyWatchdog(WatchdogStatus);
    WatchdogStatus readWatchdog();
    std::vector<int> prepareListOfPowerCapsInMicroWatts(/*Domain = PowerCapDomain::PKG*/);
    void singleAppRunAndPowerSample(char* const*);
    FinalPowerAndPerfResult multipleAppRunAndPowerSample(char* const*, int, std::optional<std::reference_wrapper<std::stringstream>> = std::nullopt);
    PowAndPerfResult checkPowerAndPerformance(int);
    void reportResult(double = 0.0, double = 0.0);
    void waitForTuningTrigger(int&, int);
    void execPhase(int, int&, int, PowAndPerfResult&);
    int mainAppProcess(char* const*, int&);
    int& adjustHighPowLimit(PowAndPerfResult, int&);

};
