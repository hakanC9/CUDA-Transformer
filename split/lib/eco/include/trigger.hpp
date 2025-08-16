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

#include "data_structures/data_filter.hpp"
#include "params_config.hpp"

enum class TriggerType
{
  NO_TUNING,
  SINGLE_IMMEDIATE_TUNING,
  SINGLE_TUNING_WITH_WAIT,
  PERIODIC_IMMEDIATE_TUNING,
  PERIODIC_TUNING_WITH_WAIT,
  EXTERNAL_TRIGGER_FOR_TUNING,
};

class Trigger
{
  public:
    Trigger() = delete;
    Trigger(ParamsConfig cfg) :
      preFilter_(100), filter_(100)
      {
        if (cfg.repeatTuningPeriodInSec_ > 0)
        {
          type_ = cfg.doWaitPhase_ ? TriggerType::PERIODIC_TUNING_WITH_WAIT : TriggerType::PERIODIC_IMMEDIATE_TUNING;
          isTuningPeriodic_ = true;
        }
        else
        {
          type_ = cfg.doWaitPhase_ ? TriggerType::SINGLE_TUNING_WITH_WAIT : TriggerType::SINGLE_IMMEDIATE_TUNING;
        }
      }
    ~Trigger() = default;

    bool isDeviceReadyForTuning() const
    {
      switch(type_)
      {
        case TriggerType::SINGLE_TUNING_WITH_WAIT:
        case TriggerType::PERIODIC_TUNING_WITH_WAIT:
          return filter_.getCleanedRelativeError() < TRESHOLD;
        case TriggerType::SINGLE_IMMEDIATE_TUNING:
        case TriggerType::PERIODIC_IMMEDIATE_TUNING:
          return hasDeviceReportedAnyComputeActivityThroughPerfCounter_;
        case TriggerType::NO_TUNING:
        default:
          return false;
      }
    }

    double getCurrentFilteredPowerInWatts() const
    {
      return filter_.getSMA();
    }

    void appendPowerSampleToSmaFilter(double powerInWatts)
    {

      filter_.storeDataPoint(preFilter_.getSMA());
      preFilter_.storeDataPoint(powerInWatts);
    }

    void updateComputeActivityFlag(bool computeActivityOfDeviceCondition)
    {
      hasDeviceReportedAnyComputeActivityThroughPerfCounter_ =
          hasDeviceReportedAnyComputeActivityThroughPerfCounter_ || computeActivityOfDeviceCondition;
    }

    bool isTuningPeriodic() const
    {
      return isTuningPeriodic_;
    }

  private:
    TriggerType type_;
    DataFilter filter_;
    DataFilter preFilter_;
    double TRESHOLD {0.03};
    bool hasDeviceReportedAnyComputeActivityThroughPerfCounter_ {false};
    bool isTuningPeriodic_ {false};
};