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

#include <string>
#include <vector>
#include <memory>
#include <set>
#include <optional>
#include <cpucounters.h>
#include "eco_constants.hpp"

class Device
{
public:
    Device() {}
    virtual ~Device() = default;
    virtual std::string getName() const = 0;
    virtual std::pair<unsigned, unsigned> getMinMaxLimitInWatts() const = 0;
    virtual double getPowerLimitInWatts() const = 0;
    virtual void setPowerLimitInMicroWatts(unsigned long limitInMicroW) = 0;
    virtual void reset() = 0;
    virtual unsigned long long int getPerfCounter() const = 0;
    virtual double getCurrentPowerInWatts(std::optional<Domain>) const = 0;
    virtual void restoreDefaultLimits() = 0;
    virtual std::string getDeviceTypeString() const = 0;
    /*
      triggerPowerApiSample - used to trigger next sample from Power Management API

      This method is required for generalization of DeviceStateAccumulator.
      Intel RAPL API depends on explicit triggering of next energy counter reads while
      other vendors (like, e.g., NVIDIA in NVML) do it automatically in the library.
      This makes defining this method OPTIONAL for most of vendors - the method definition
      may be just left empty. For Intel it needs to have Rapl::sample() method call.
    */
    virtual void triggerPowerApiSample() = 0;

private:
};
