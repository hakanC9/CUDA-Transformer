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

#include "devices/xpu_device.hpp"
#include "perf_counter_interfaces/xpu_perf_counter.hpp"
#include "../../../src/logging.hpp"

#include <cstdlib>
#include <cstring>
#include <tuple>

static constexpr double MICRO_W = 1e6;
static constexpr double MILI_W  = 1e3;

void XPUDevice::initL0()
{
    // If ZES_ENABLE_SYSMAN is not set then make it set
    if (std::getenv("ZES_ENABLE_SYSMAN") == nullptr)
    {
        setenv("ZES_ENABLE_SYSMAN", "1", 1);
    }
    if (std::getenv("ZET_ENABLE_METRICS") == nullptr)
    {
        setenv("ZET_ENABLE_METRICS", "1", 1);
    }

    auto result = zeInit(ZE_INIT_FLAG_GPU_ONLY);
    if (result == ZE_RESULT_SUCCESS)
    {
        LOG_DEBUG("Level Zero initialized");
    }
    else
    {
        throw std::runtime_error(errorMap.at(result));
    }
}

zes_driver_handle_t XPUDevice::initL0Driver()
{
    unsigned int count  = 0;
    auto         result = zeDriverGet(&count, nullptr);
    if (result == ZE_RESULT_SUCCESS)
    {
        LOG_DEBUG("Level Zero Number of Drivers: {}", count);
    }
    else
    {
        throw std::runtime_error(errorMap.at(result));
    }

    if (count == 0)
    {
        throw std::runtime_error("No Level Zero drivers found!");
    }

    std::vector<zes_driver_handle_t> phdrivers(count, nullptr);

    result = zeDriverGet(&count, phdrivers.data());

    if (result == ZE_RESULT_SUCCESS)
    {
        LOG_DEBUG("Level Zero Drivers initialized");
    }
    else
    {
        throw std::runtime_error(errorMap.at(result));
    }
    return *(phdrivers.begin());
}

zes_device_handle_t XPUDevice::getL0Device(zes_driver_handle_t& driver, int devID)
{
    unsigned int count  = 0;
    auto         result = zeDeviceGet(driver, &count, nullptr);
    if (result == ZE_RESULT_SUCCESS)
    {
        LOG_DEBUG("Level Zero Number of Devices: {}", count);
    }
    else
    {
        throw std::runtime_error(errorMap.at(result));
    }
    if (count == 0)
    {
        throw std::runtime_error("No Level Zero devices found!");
    }
    if (count <= devID)
    {
        throw std::runtime_error("No requested device found!");
    }

    std::vector<zes_device_handle_t> phdevices(count, nullptr);
    result = zeDeviceGet(driver, &count, phdevices.data());
    if (result == ZE_RESULT_SUCCESS)
    {
        LOG_DEBUG("Level Zero Devices acquired");
    }
    else
    {
        throw std::runtime_error(errorMap.at(result));
    }

    return phdevices[devID];
}

zes_device_properties_t XPUDevice::getDeviceProperties(zes_device_handle_t& device)
{
    zes_device_properties_t properties;
    auto                    result = zesDeviceGetProperties(device, &properties);

    if (result == ZE_RESULT_SUCCESS)
    {
        LOG_DEBUG("Level Zero Devices properties acquired");
    }
    else
    {
        throw std::runtime_error(errorMap.at(result));
    }

    return properties;
}

zes_power_domain_t XPUDevice::getPowerDomainProperties(zes_pwr_handle_t& domain)
{
    zes_power_properties_t     PowerProperties;
    zes_power_ext_properties_t ExtProps;
    std::memset(&PowerProperties, 0, sizeof(PowerProperties));
    std::memset(&ExtProps, 0, sizeof(ExtProps));
    PowerProperties.stype = ZES_STRUCTURE_TYPE_POWER_PROPERTIES;
    PowerProperties.pNext = &ExtProps;
    ExtProps.stype        = ZES_STRUCTURE_TYPE_POWER_EXT_PROPERTIES;

    auto result = zesPowerGetProperties(domain, &PowerProperties);
    if (result == ZE_RESULT_SUCCESS)
    {
        auto domain_type = ExtProps.domain;
        LOG_DEBUG("Level Zero Domain Type acquired: {}", domainTypeMap.at(domain_type));
        return domain_type;
    }

    throw std::runtime_error(errorMap.at(result));
}

void XPUDevice::getPowerDomain(zes_device_handle_t& device)
{
    unsigned int count  = 0;
    auto         result = zesDeviceEnumPowerDomains(device, &count, nullptr);
    if (result == ZE_RESULT_SUCCESS)
    {
        LOG_DEBUG("Level Zero Number of Power domains: {}", count);
    }
    else
    {
        throw std::runtime_error(errorMap.at(result));
    }
    if (count == 0)
    {
        throw std::runtime_error("No Level Zero power domains found for selected device!");
    }
    std::vector<zes_pwr_handle_t> phdomains(count, nullptr);
    result = zesDeviceEnumPowerDomains(device, &count, phdomains.data());
    if (result == ZE_RESULT_SUCCESS)
    {
        LOG_DEBUG("Level Zero Power Domains acquired");
    }
    else
    {
        throw std::runtime_error(errorMap.at(result));
    }

    // Get first domain for whole XPU card
    for (auto& domain : phdomains)
    {
        auto domain_type = getPowerDomainProperties(domain);
        if (domain_type == ZES_POWER_DOMAIN_CARD)
        {
            this->power_handle = domain;
            return;
        }
    }

    throw std::runtime_error("No Level Zero power domain  of type "
                             "ZES_POWER_DOMAIN_CARD found for selected device!");
}

XPUDevice::XPUDevice(int devID, bool useAmperes)
: deviceID_(devID),
  device_properties {},
  power_handle(nullptr),
  zeResult_(ZE_RESULT_SUCCESS),
  energy_samples {{0, 0}, {0, 0}},
  useAmperes_(useAmperes)
{
    LOAD_ENV_LEVELS()

    LOG_DEBUG("XPUDevice constructor called");

    try
    {
        this->initL0();
        auto driver = this->initL0Driver();
        // Lets get selected device from first driver
        this->device = getL0Device(driver, devID);

        this->device_properties = getDeviceProperties(device);

        LOG_INFO("Device: {} initialized", this->device_properties.core.name);

        getPowerDomain(device);

        // XPU is by default having max set to power limits
        // temporary set useAmperes_ to true and false to set both limits
        this->useAmperes_               = false;
        auto minMaxPower                = calculateMinMaxLimitsinWatts();
        this->defaultPowerLimitInWatts_ = std::get<1>(minMaxPower);
        setPowerLimitInMicroWatts(MICRO_W * defaultPowerLimitInWatts_);
        this->useAmperes_                 = true;
        auto minMaxCurrent                = calculateMinMaxLimitsinWatts();
        this->defaultPowerLimitInAmperes_ = std::get<1>(minMaxCurrent);
        setPowerLimitInMicroWatts(MICRO_W * defaultPowerLimitInAmperes_);
        this->useAmperes_ = useAmperes;

        if (useAmperes)
        {
            this->minLimitValue = std::get<0>(minMaxCurrent);
            this->maxLimitValue = std::get<1>(minMaxCurrent);
        }
        else
        {
            this->minLimitValue = std::get<0>(minMaxPower);
            this->maxLimitValue = std::get<1>(minMaxPower);
        }

        metric_collector_ =
            ZeMetricCollector::Create((ze_driver_handle_t)driver, (ze_device_handle_t)device, "ComputeBasic");
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("XPU initialization error: {}", e.what());
        throw;
    }
}

std::vector<zes_power_limit_ext_desc_t> XPUDevice::getLimits() const
{
    unsigned int num_limits = 0;
    auto         result     = zesPowerGetLimitsExt(this->power_handle, &num_limits, nullptr);
    if (result == ZE_RESULT_SUCCESS)
    {
        LOG_DEBUG("Level Zero Number of Power limits: {}", num_limits);
    }
    else
    {
        throw std::runtime_error(errorMap.at(result));
    }

    std::vector<zes_power_limit_ext_desc_t> phlimits(num_limits);
    result = zesPowerGetLimitsExt(this->power_handle, &num_limits, phlimits.data());
    if (result != ZE_RESULT_SUCCESS)
    {
        throw std::runtime_error(errorMap.at(result));
    }

    return phlimits;
}

std::string XPUDevice::getDeviceTypeString() const
{
    return "xpu";
};

void XPUDevice::restoreDefaultLimits()
{
    if (this->useAmperes_)
    {
        setPowerLimitInMicroWatts(MICRO_W * defaultPowerLimitInAmperes_);
    }
    else
    {
        setPowerLimitInMicroWatts(MICRO_W * defaultPowerLimitInWatts_);
    }
}

double XPUDevice::getPowerLimitSustained() const
{
    return this->getInnerPowerLimit(false);
}

double XPUDevice::getPowerLimitPeak() const
{
    return this->getInnerPowerLimit(true);
}

// Return power limit in Watts or miliAmperes
double XPUDevice::getPowerLimitInWatts() const
{
    return this->getInnerPowerLimit(this->useAmperes_);
}

double XPUDevice::getInnerPowerLimit(bool useAmperes) const
{
    try
    {
        auto phlimits = getLimits();

        auto it = std::find_if(
            phlimits.begin(),
            phlimits.end(),
            [&](const zes_power_limit_ext_desc_t& limit)
            {
                if (useAmperes)
                {
                    return limit.level == ZES_POWER_LEVEL_PEAK && limit.limitUnit == ZES_LIMIT_UNIT_CURRENT;
                }
                else
                {
                    return limit.level == ZES_POWER_LEVEL_SUSTAINED && limit.limitUnit == ZES_LIMIT_UNIT_POWER;
                }
            });

        if (it == phlimits.end())
        {
            throw std::runtime_error("No requested power level limit found!");
        }

        if (useAmperes)
        {
            LOG_DEBUG("XPU power limit: {} Amperes", it->limit / 1000.0);
            return it->limit / 1000.0;
        }
        else
        {
            // ZES_LIMIT_UNIT_POWER returns values in miliwatts
            // so we need to convert it to watts.
            LOG_DEBUG("XPU power limit: {} Watts", it->limit / 1000.0);
            return it->limit / 1000.0;
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("XPU getPowerLimitInWatts error: {}", e.what());
        throw;  // Rethrow the exception to avoid undefined behavior
    }
}

std::string XPUDevice::getName() const
{
    return {this->device_properties.core.name};
}

// Get minimal and maximal limit value
// Make set 0 and then 4 x current limit as
// if requested limit is higher what is allowed then max will be set
// if requested limit is lower than what is allowed then minimum would be set
std::tuple<unsigned, unsigned> XPUDevice::calculateMinMaxLimitsinWatts()
{
    // Ok getting minimum is like
    // 1. gett limit value
    double curr_limit = this->getPowerLimitInWatts();

    // Get minimum limit to be set (Below 1 watt there is an error : UNKNOWN)
    this->setPowerLimitInMicroWatts(MICRO_W);
    auto minLimitValue = this->getPowerLimitInWatts();
    // Get maximal limit to be set
    this->setPowerLimitInMicroWatts(4.0 * curr_limit * MICRO_W);
    auto maxLimitValue = this->getPowerLimitInWatts();

    LOG_DEBUG("Level Zero Limit values range: <{},{}>", this->minLimitValue, this->maxLimitValue);

    // Restore default settings
    this->setPowerLimitInMicroWatts(static_cast<unsigned long>(curr_limit * MICRO_W));

    return std::make_tuple(minLimitValue, maxLimitValue);
}

std::pair<unsigned, unsigned> XPUDevice::getMinMaxLimitInWatts() const
{
    return std::make_pair(this->minLimitValue, this->maxLimitValue);
}

void XPUDevice::setPowerLimitInMicroWatts(unsigned long limitInMicroW)
{
    auto limitsInfo =
        this->useAmperes_
            ? std::make_tuple(ZES_POWER_LEVEL_PEAK, ZES_LIMIT_UNIT_CURRENT, "No Peak level limit found!")
            : std::make_tuple(ZES_POWER_LEVEL_SUSTAINED, ZES_LIMIT_UNIT_POWER, "No sustained level limit found!");

    auto power_limit_condition = [&](const zes_power_limit_ext_desc_t& limit)
    { return limit.level == std::get<0>(limitsInfo) && limit.limitUnit == std::get<1>(limitsInfo); };

    try
    {
        auto phlimits = getLimits();

        auto it = std::find_if(phlimits.begin(), phlimits.end(), power_limit_condition);

        if (it == phlimits.end())
        {
            throw std::runtime_error(std::get<2>(limitsInfo));
        }

        // modify selected limit
        auto limitInMilliWatts = static_cast<uint64_t>(static_cast<double>(limitInMicroW) / MILI_W);
        if (this->useAmperes_)
        {
            LOG_DEBUG("Setting a new limit [Amperes] {}", limitInMilliWatts);
        }
        else
        {
            LOG_DEBUG("Setting a new limit [Watts] {}", limitInMilliWatts);
        }
        it->limit         = static_cast<int32_t>(limitInMilliWatts);
        unsigned int size = phlimits.size();

        // Set limits with modified values
        auto result = zesPowerSetLimitsExt(this->power_handle, &size, phlimits.data());
        if (result != ZE_RESULT_SUCCESS)
        {
            throw std::runtime_error(errorMap.at(result));
        }

        auto limit = this->getPowerLimitInWatts();
        if (this->useAmperes_)
        {
            LOG_DEBUG("Successfuly set XPU power limit to {} Amperes", limit);
        }
        else
        {
            LOG_DEBUG("Successfuly set XPU power limit to {} Watts", limit);
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("XPU setPowerLimitInMicroWatts error: {}", e.what());
        throw std::runtime_error("XPU setPowerLimitInMicroWatts error: " + std::string(e.what()));
    }
}

void XPUDevice::reset()
{
    metric_collector_->resetAccumulatedMetrics();
}

double XPUDevice::getCurrentPowerInWatts(std::optional<Domain>) const
{
    // (t1,e1) and (t2,e2) = (e2 - e1)/(t2-t1)
    //
    auto delta_t = energy_samples[1].timestamp - energy_samples[0].timestamp;
    auto delta_e = energy_samples[1].energy - energy_samples[0].energy;

    double avg_power = static_cast<double>(delta_e) / static_cast<double>(delta_t);

    return avg_power;
}

zes_power_energy_counter_t XPUDevice::sampleEnergyCounter()
{
    zes_power_energy_counter_t energy_counter;
    auto                       result = zesPowerGetEnergyCounter(this->power_handle, &energy_counter);

    if (result != ZE_RESULT_SUCCESS)
    {
        throw std::runtime_error(errorMap.at(result));
    }
    return energy_counter;
}

void XPUDevice::triggerPowerApiSample()
{
    // Discard older sample and get current sample
    this->energy_samples[0] = this->energy_samples[1];
    try
    {
        this->energy_samples[1] = sampleEnergyCounter();
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("XPU energy counter sampling error: {}", e.what());
    }
}

unsigned long long int XPUDevice::getPerfCounter() const
{
    return metric_collector_->getAccumulatedMetricsSinceLastReset();
}
