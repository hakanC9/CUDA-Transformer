/*
   Copyright 2024, Adam Krzywaniak.

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

#include <sys/wait.h>
#include "logging/both_stream.hpp"
#include "logging/log.hpp"


class SearchAlgorithm
{
  public:
    virtual unsigned operator() (
      std::shared_ptr<Device>,
      DeviceStateAccumulator&,
      Trigger&,
      TargetMetric,
      const PowAndPerfResult&,
      int&,
      int,
      int,
      int,
      Logger&) const = 0;

    static PowAndPerfResult sampleAndAccumulatePowAndPerfForGivenPeriod(
      int tuningTimeWindowInMicroSeconds,
      int powerSamplingPeriodInMilliSeconds,
      DeviceStateAccumulator& deviceState,
      Trigger& trigger,
      int& procStatus,
      int childProcID,
      Logger& logger)
    {
      auto pauseInMicroSeconds = powerSamplingPeriodInMilliSeconds * 1000;
      usleep(pauseInMicroSeconds);
      deviceState.sample();
      auto resultAccumulator = deviceState.getCurrentPowerAndPerf();

      while (tuningTimeWindowInMicroSeconds > pauseInMicroSeconds)
      {
        usleep(pauseInMicroSeconds);
        deviceState.sample();
        auto tmp = deviceState.getCurrentPowerAndPerf(trigger);
        logger.logPowerLogLine(deviceState, tmp);
        resultAccumulator += tmp;
        tuningTimeWindowInMicroSeconds -= pauseInMicroSeconds;

        waitpid(childProcID, &procStatus, WNOHANG);
        if (!procStatus) break;
      }

      return resultAccumulator;
    }
};
