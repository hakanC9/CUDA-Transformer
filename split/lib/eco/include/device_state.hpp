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

#include "devices/abstract_device.hpp"
#include "data_structures/power_and_perf_result.hpp"
#include "trigger.hpp"

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

struct PowerAndPerfState
{
    PowerAndPerfState() = delete;
    PowerAndPerfState(double pow, unsigned long long ker, TimePoint t) :
        power_(pow), kernelsCount_(ker), time_(t)
    {
    }
    double power_;
    unsigned long long kernelsCount_;
    TimePoint time_;
};

class DeviceStateAccumulator
{
public:
    DeviceStateAccumulator(std::shared_ptr<Device>);
    ~DeviceStateAccumulator() {}

    /*
      getCurrentPowerAndPerf - needed mostly (only?) for logging purposes

      returns the PowerAndPerfResult struct with the data based on the difference
      between next and current state. Such data is used for power log.
    */
    PowAndPerfResult getCurrentPowerAndPerf(std::optional<std::reference_wrapper<Trigger>> trigger = std::nullopt) const;

    /*
      getEnergySinceReset - is used for the final evaluation of energy consumed

      returns the integrate of energy sampled since last Accumulator reset.
    */
    double getEnergySinceReset() const;

    /*
      getTimeSinceReset - is used for the final evaluation of time spent on computations

      returns the time difference between now and last Accumulator reset.
    */
    template <class Resolution = std::chrono::milliseconds>
    double getTimeSinceReset() const
    {
        return std::chrono::duration_cast<Resolution>(
                    std::chrono::high_resolution_clock::now()  - timeOfLastReset_).count();
    }

    /*
      getTimeSinceObjectCreation - used only for logging purposes.

      returns the absolute time since the creation of the device state accumulator. Needed
      for logging data used for power log creation.
    */
    template <class Resolution = std::chrono::milliseconds>
    double getTimeSinceObjectCreation() const
    {
        return std::chrono::duration_cast<Resolution>(
                    std::chrono::high_resolution_clock::now()  - absoluteStartTime_).count();
    }

    // Temporary disabled
    // ----------------------------------------------------------------------------------------
    // std::vector<double> getTotalEnergyVec(Domain d);

    DeviceStateAccumulator& sample();
    void resetState();
    double getCurrentPower(Domain d);
    double getPerfCounterSinceReset();

private:
    TimePoint absoluteStartTime_;
    TimePoint timeOfLastReset_;
    std::shared_ptr<Device> device_;
    PowerAndPerfState prev_, curr_, next_;
    double totalEnergySinceReset_ {0.0};
};
