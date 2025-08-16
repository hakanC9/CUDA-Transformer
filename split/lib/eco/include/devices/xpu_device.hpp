/*
   Copyright 2024, Intel Corporation

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
#include "perf_counter_interfaces/xpu_perf_counter.hpp"

#include <level_zero/ze_api.h>
#include <level_zero/zes_api.h>

/**
 * This class represents single XPU device pointed by the deviceID
 * during object construction. However, it stores all the device handles
 * and it is able to read power or write power limit to any existing
 * in the system XPU device.
 * By design at the moment the get/set power methods use only deviceID_
 * which is a member of the object.
 *
 * FUTURE WORK:
 * In the future this may change when, e.g., DEPO or StEP would consider
 * multi-gpu support.
 */
class XPUDevice : public Device
{
public:
    XPUDevice(int devID = 0, bool useAmperes = true);
    ~XPUDevice()
    {
        if (metric_collector_ != nullptr)
        {
            metric_collector_->DisableCollection();
        }
    }

    double getPowerLimitInWatts() const override;
    double getPowerLimitSustained() const;
    double getPowerLimitPeak() const;

    void        setPowerLimitInMicroWatts(unsigned long limitInMicroW) override;
    std::string getName() const override;

    std::pair<unsigned, unsigned> getMinMaxLimitInWatts() const override;
    void                          reset() override;
    double                        getCurrentPowerInWatts(std::optional<Domain> = std::nullopt) const override;
    unsigned long long int        getPerfCounter() const;
    void                          triggerPowerApiSample() override;
    void                          restoreDefaultLimits() override;
    std::string                   getDeviceTypeString() const override;

private:
    void                                    initL0();
    zes_driver_handle_t                     initL0Driver();
    zes_device_handle_t                     getL0Device(zes_driver_handle_t& driver, int devID);
    zes_device_properties_t                 getDeviceProperties(zes_device_handle_t& device);
    zes_power_domain_t                      getPowerDomainProperties(zes_pwr_handle_t& domain);
    void                                    getPowerDomain(zes_device_handle_t& device);
    std::vector<zes_power_limit_ext_desc_t> getLimits() const;
    zes_power_energy_counter_t              sampleEnergyCounter();
    std::tuple<unsigned, unsigned>          calculateMinMaxLimitsinWatts();
    double                                  getInnerPowerLimit(bool useAmperes) const;

    zes_device_handle_t     device;
    zes_device_properties_t device_properties;
    zes_pwr_handle_t        power_handle;

    ze_result_t                      zeResult_;
    unsigned int                     deviceCount_ {0};
    int                              deviceID_;
    std::vector<zes_driver_handle_t> deviceHandles_;
    double                           defaultPowerLimitInWatts_;
    double                           defaultPowerLimitInAmperes_;
    bool                             useAmperes_;        // If true, the power limit is in Amperes, otherwise in Watts
    unsigned                         minLimitValue = 0;  // Minimal value possible to set as a limit[uWatts]
    unsigned                         maxLimitValue = 0;  // Maximal value possible to set as a limit[uWatts]
    // We will store two energy samples just to compute current power usage
    // Energy is in microJoules
    // timestamp is in microseconds
    // energy_samples[1] -- newer sample
    // energy_sample[0] -- older sample
    zes_power_energy_counter_t energy_samples[2];
    ZeMetricCollector*         metric_collector_ = nullptr;
};
