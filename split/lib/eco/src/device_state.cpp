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

#include "device_state.hpp"

#include <set>

DeviceStateAccumulator::DeviceStateAccumulator(std::shared_ptr<Device> d) :
    absoluteStartTime_(std::chrono::high_resolution_clock::now()),
    timeOfLastReset_(std::chrono::high_resolution_clock::now()),
    device_(d),
    prev_(0.0, 0, timeOfLastReset_),
    curr_(prev_),
    next_(prev_)
{
}

void DeviceStateAccumulator::resetState()
{
    device_->reset();
    timeOfLastReset_ = std::chrono::high_resolution_clock::now();
    totalEnergySinceReset_ = 0.0;
    sample();
    sample();
}

DeviceStateAccumulator& DeviceStateAccumulator::sample()
{
    prev_ = curr_;
    curr_ = next_;
    // ------------------------------------------------------------------
    // this is specific to Intel RAPL power/energy measurements:
    // in order to have any valid readings, RAPL must be sampled
    // preety frequently so that the energy counter reading is updated
    // before the couter overflow.
    device_->triggerPowerApiSample();
    // for other devices like NVIDIA it is handled by the API (e.g., NVML)
    // ------------------------------------------------------------------
    const auto  perfCounter = device_->getPerfCounter();

    next_ = PowerAndPerfState(
        device_->getCurrentPowerInWatts(std::nullopt),
        perfCounter,
        std::chrono::high_resolution_clock::now());

    auto timeDeltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(next_.time_ - curr_.time_).count();
    totalEnergySinceReset_ += next_.power_ * timeDeltaMs / 1000;
    return *this;
}

double DeviceStateAccumulator::getCurrentPower(Domain d)
{
    return device_->getCurrentPowerInWatts(d);
}

double DeviceStateAccumulator::getPerfCounterSinceReset()
{
    return device_->getPerfCounter();
}

double DeviceStateAccumulator::getEnergySinceReset() const
{
    return totalEnergySinceReset_;
}

PowAndPerfResult DeviceStateAccumulator::getCurrentPowerAndPerf(std::optional<std::reference_wrapper<Trigger>> trigger) const
{
    double perfCounterDelta = (double)(next_.kernelsCount_ - curr_.kernelsCount_);
    if (trigger.has_value())
    {
        trigger->get().appendPowerSampleToSmaFilter(next_.power_);
        trigger->get().updateComputeActivityFlag(perfCounterDelta > 0.0);
    }
    double timeDeltaMilliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(next_.time_ - curr_.time_).count();
    return PowAndPerfResult(
        perfCounterDelta,
        timeDeltaMilliSeconds / 1000,
        device_->getPowerLimitInWatts(),
        next_.power_ * timeDeltaMilliSeconds / 1000, // Watts x seconds
        next_.power_,
        0.0, // memory power - not available for GPU
        (trigger.has_value() ? trigger->get().getCurrentFilteredPowerInWatts() : -1.0) // TODO: this should be filtered power
        );
}

// Temporary disabled
// ----------------------------------------------------------------------------------------
// std::vector<double> DeviceStateAccumulator::getTotalEnergyVec(Domain d)
// {
//     std::vector<double> result;
//     for (auto&& rapl : raplVec_) {
//         result.push_back(rapl.getTotalEnergy()[d]);
//     }
//     return result;
// }
// ----------------------------------------------------------------------------------------
