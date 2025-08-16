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

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <memory>
#include <string>
#include <chrono>
#include <sys/stat.h>
#include "eco.hpp"
#include "plot_builder.hpp"
#include "params_config.hpp"
#include "logging/both_stream.hpp"
#include "data_structures/power_and_perf_result.hpp"
#include "logging/log.hpp"
#include "device_state.hpp"
#include "devices/abstract_device.hpp"

#include <cuda.h>
#include <nvml.h>


/**
 * This class represents single CUDA device pointed by the deviceID
 * during object construction. However, it stores all the device handles
 * and it is able to read power or write power limit to any existing
 * in the system CUDA device.
 * By design at the moment the get/set power methods use only deviceID_
 * which is a member of the object.
 *
 * FUTURE WORK:
 * In the future this may change when, e.g., DEPO or StEP would consider
 * multi-gpu support.
*/
class CudaDevice : public Device
{
  public:
    CudaDevice(int devID = 0);

    double getPowerLimitInWatts() const override;
    void setPowerLimitInMicroWatts(unsigned long limitInMicroW) override;
    std::string getName() const override;

    std::pair<unsigned, unsigned> getMinMaxLimitInWatts() const override;
    void reset() override;
    double getCurrentPowerInWatts(std::optional<Domain> = std::nullopt) const override;
    unsigned long long int getPerfCounter() const;
    void triggerPowerApiSample() override {}; // empty method since, NVIDIA GPU does not need to explicit trigger API sampling
    void restoreDefaultLimits() override;
    std::string getDeviceTypeString() const override { return "gpu"; };


  private:
    void initDeviceHandles();
    nvmlReturn_t nvResult_;
    unsigned int deviceCount_ {0};
    int deviceID_;
    std::vector<nvmlDevice_t> deviceHandles_;
    double defaultPowerLimitInWatts_;
};
