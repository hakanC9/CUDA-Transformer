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

#include "algorithms/abstract_search_algorithm.hpp"


class LinearSearchAlgorithm : public SearchAlgorithm
{
  public:
    unsigned operator() (
      std::shared_ptr<Device> device,
      DeviceStateAccumulator& deviceState,
      Trigger& trigger,
      TargetMetric metric,
      const PowAndPerfResult& reference,
      int& procStatus,
      int childProcID,
      int powerSamplingPeriodInMilliSeconds,
      int tuningTimeWindowInMilliSeconds,
      Logger& logger) const
    {
      const auto [minLimitInWatts, maxLimitInWatts] = device->getMinMaxLimitInWatts();
      const auto minLimitInMictoWatts = minLimitInWatts * 1e6;
      const auto maxLimitInMictoWatts = maxLimitInWatts * 1e6;
      int STEP = (maxLimitInMictoWatts - minLimitInMictoWatts) / 10;

      auto bestResultSoFar = reference;
      int currentLimitInMicroWatts = maxLimitInMictoWatts;

      while(procStatus)
      {
        device->setPowerLimitInMicroWatts(currentLimitInMicroWatts);
        auto&& currentResult = sampleAndAccumulatePowAndPerfForGivenPeriod(
          tuningTimeWindowInMilliSeconds * 1e3,
          powerSamplingPeriodInMilliSeconds,
          deviceState,
          trigger,
          procStatus,
          childProcID,
          logger);
        logger.logPowerLogLine(deviceState, currentResult, reference);
        if (bestResultSoFar.isRightBetter(currentResult, metric))
        {
            bestResultSoFar = std::move(currentResult);
        }
        if (currentLimitInMicroWatts == minLimitInMictoWatts)
        {
            break;
        }
        currentLimitInMicroWatts -= STEP;
        if (currentLimitInMicroWatts < minLimitInMictoWatts)
        {
          currentLimitInMicroWatts = minLimitInMictoWatts;
        }
        waitpid(childProcID, &procStatus, WNOHANG);
      }
      return (unsigned)(bestResultSoFar.appliedPowerCapInWatts_ * 1e6);
    }
};
