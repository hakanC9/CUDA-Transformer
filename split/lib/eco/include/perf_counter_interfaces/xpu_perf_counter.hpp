/*
   Copyright 2025, Intel Corporation

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

#include "../../../src/logging.hpp"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <level_zero/ze_api.h>
#include <level_zero/zes_api.h>
#include <level_zero/zet_api.h>

inline const std::map<zes_power_domain_t, std::string> domainTypeMap = []
{
    std::map<zes_power_domain_t, std::string> map;

    map[ZES_POWER_DOMAIN_UNKNOWN]      = "ZES_POWER_DOMAIN_UNKNOWN";
    map[ZES_POWER_DOMAIN_CARD]         = "ZES_POWER_DOMAIN_CARD";
    map[ZES_POWER_DOMAIN_PACKAGE]      = "ZES_POWER_DOMAIN_PACKAGE";
    map[ZES_POWER_DOMAIN_STACK]        = "ZES_POWER_DOMAIN_STACK";
    map[ZES_POWER_DOMAIN_GPU]          = "ZES_POWER_DOMAIN_GPU";
    map[ZES_POWER_DOMAIN_FORCE_UINT32] = "ZES_POWER_DOMAIN_FORCE_UINT32";
    return map;
}();

inline const std::map<ze_result_t, std::string> errorMap = []
{
    std::map<ze_result_t, std::string> map;
    map[ZE_RESULT_SUCCESS]                              = "ZE_RESULT_SUCCESS";
    map[ZE_RESULT_NOT_READY]                            = "ZE_RESULT_NOT_READY";
    map[ZE_RESULT_ERROR_DEVICE_LOST]                    = "ZE_RESULT_ERROR_DEVICE_LOST";
    map[ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY]             = "ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY";
    map[ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY]           = "ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY";
    map[ZE_RESULT_ERROR_MODULE_BUILD_FAILURE]           = "ZE_RESULT_ERROR_MODULE_BUILD_FAILURE";
    map[ZE_RESULT_ERROR_MODULE_LINK_FAILURE]            = "ZE_RESULT_ERROR_MODULE_LINK_FAILURE";
    map[ZE_RESULT_ERROR_DEVICE_REQUIRES_RESET]          = "ZE_RESULT_ERROR_DEVICE_REQUIRES_RESET";
    map[ZE_RESULT_ERROR_DEVICE_IN_LOW_POWER_STATE]      = "ZE_RESULT_ERROR_DEVICE_IN_LOW_POWER_STATE";
    map[ZE_RESULT_EXP_ERROR_DEVICE_IS_NOT_VERTEX]       = "ZE_RESULT_EXP_ERROR_DEVICE_IS_NOT_VERTEX";
    map[ZE_RESULT_EXP_ERROR_VERTEX_IS_NOT_DEVICE]       = "ZE_RESULT_EXP_ERROR_VERTEX_IS_NOT_DEVICE";
    map[ZE_RESULT_EXP_ERROR_REMOTE_DEVICE]              = "ZE_RESULT_EXP_ERROR_REMOTE_DEVICE";
    map[ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS]       = "ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS";
    map[ZE_RESULT_ERROR_NOT_AVAILABLE]                  = "ZE_RESULT_ERROR_NOT_AVAILABLE";
    map[ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE]         = "ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE";
    map[ZE_RESULT_WARNING_DROPPED_DATA]                 = "ZE_RESULT_WARNING_DROPPED_DATA";
    map[ZE_RESULT_ERROR_UNINITIALIZED]                  = "ZE_RESULT_ERROR_UNINITIALIZED";
    map[ZE_RESULT_ERROR_UNSUPPORTED_VERSION]            = "ZE_RESULT_ERROR_UNSUPPORTED_VERSION";
    map[ZE_RESULT_ERROR_UNSUPPORTED_FEATURE]            = "ZE_RESULT_ERROR_UNSUPPORTED_FEATURE";
    map[ZE_RESULT_ERROR_INVALID_ARGUMENT]               = "ZE_RESULT_ERROR_INVALID_ARGUMENT";
    map[ZE_RESULT_ERROR_INVALID_NULL_HANDLE]            = "ZE_RESULT_ERROR_INVALID_NULL_HANDLE";
    map[ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE]           = "ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE";
    map[ZE_RESULT_ERROR_INVALID_NULL_POINTER]           = "ZE_RESULT_ERROR_INVALID_NULL_POINTER";
    map[ZE_RESULT_ERROR_INVALID_SIZE]                   = "ZE_RESULT_ERROR_INVALID_SIZE";
    map[ZE_RESULT_ERROR_UNSUPPORTED_SIZE]               = "ZE_RESULT_ERROR_UNSUPPORTED_SIZE";
    map[ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT]          = "ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT";
    map[ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT] = "ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT";
    map[ZE_RESULT_ERROR_INVALID_ENUMERATION]            = "ZE_RESULT_ERROR_INVALID_ENUMERATION";
    map[ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION]        = "ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION";
    map[ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT]       = "ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT";
    map[ZE_RESULT_ERROR_INVALID_NATIVE_BINARY]          = "ZE_RESULT_ERROR_INVALID_NATIVE_BINARY";
    map[ZE_RESULT_ERROR_INVALID_GLOBAL_NAME]            = "ZE_RESULT_ERROR_INVALID_GLOBAL_NAME";
    map[ZE_RESULT_ERROR_INVALID_KERNEL_NAME]            = "ZE_RESULT_ERROR_INVALID_KERNEL_NAME";
    map[ZE_RESULT_ERROR_INVALID_FUNCTION_NAME]          = "ZE_RESULT_ERROR_INVALID_FUNCTION_NAME";
    map[ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION]   = "ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION";
    map[ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION] = "ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION";
    map[ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX]  = "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX";
    map[ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE]   = "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE";
    map[ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE] = "ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE";
    map[ZE_RESULT_ERROR_INVALID_MODULE_UNLINKED]        = "ZE_RESULT_ERROR_INVALID_MODULE_UNLINKED";
    map[ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE]      = "ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE";
    map[ZE_RESULT_ERROR_OVERLAPPING_REGIONS]            = "ZE_RESULT_ERROR_OVERLAPPING_REGIONS";
    map[ZE_RESULT_WARNING_ACTION_REQUIRED]              = "ZE_RESULT_WARNING_ACTION_REQUIRED";
    map[ZE_RESULT_ERROR_UNKNOWN]                        = "ZE_RESULT_ERROR_UNKNOWN";
    map[ZE_RESULT_FORCE_UINT32]                         = "ZE_RESULT_FORCE_UINT32";

    return map;
}();

struct MetricResult
{
    uint64_t inst_alu0 = 0;
    uint64_t inst_alu1 = 0;
    uint64_t inst_xmx  = 0;
    uint64_t inst_send = 0;
    uint64_t inst_ctrl = 0;
};

enum CollectorState
{
    COLLECTOR_STATE_IDLE     = 0,
    COLLECTOR_STATE_ENABLED  = 1,
    COLLECTOR_STATE_DISABLED = 2
};

class ZeMetricCollector
{
public:
    static ZeMetricCollector* Create(ze_driver_handle_t driver, ze_device_handle_t device, std::string group_name)
    {
        if (driver == nullptr || device == nullptr)
        {
            throw std::invalid_argument("Invalid driver or device handle");
        }

        // TODO: put somewhere information that we need sufficient privileges:
        // sudo sh -c 'echo 0 > /proc/sys/dev/i915/perf_stream_paranoid'

        // TODO: put somewhere information that metrics discovery library should
        // be available in LD_LIBRARY_PATH

        // create metric group
        zet_metric_group_handle_t group       = nullptr;
        ze_result_t               status      = ZE_RESULT_SUCCESS;
        uint32_t                  group_count = 0;
        status                                = zetMetricGroupGet(device, &group_count, nullptr);
        if (status == ZE_RESULT_SUCCESS && group_count != 0)
        {
            std::vector<zet_metric_group_handle_t> group_list(group_count, nullptr);
            status = zetMetricGroupGet(device, &group_count, group_list.data());
            if (status != ZE_RESULT_SUCCESS)
            {
                throw std::runtime_error(errorMap.at(status));
            }
            zet_metric_group_handle_t target = nullptr;
            for (uint32_t i = 0; i < group_count; ++i)
            {
                zet_metric_group_properties_t group_props {};
                group_props.stype = ZET_STRUCTURE_TYPE_METRIC_GROUP_PROPERTIES;
                status            = zetMetricGroupGetProperties(group_list[i], &group_props);
                if (status != ZE_RESULT_SUCCESS)
                {
                    throw std::runtime_error(errorMap.at(status));
                }

                if (group_name == group_props.name &&
                    (group_props.samplingType & ZET_METRIC_GROUP_SAMPLING_TYPE_FLAG_TIME_BASED))
                {
                    target = group_list[i];
                    break;
                }
            }
            group = target;
            LOG_DEBUG("Level Zero Metric Group created");
        }
        else
        {
            throw std::runtime_error("Unable to find any metric groups");
        }

        // create context
        status                           = ZE_RESULT_SUCCESS;
        ze_context_handle_t context      = nullptr;
        ze_context_desc_t   context_desc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC, nullptr, 0};
        status                           = zeContextCreate(driver, &context_desc, &context);
        if (status != ZE_RESULT_SUCCESS)
        {
            throw std::runtime_error(errorMap.at(status));
        }
        LOG_DEBUG("Level Zero Context created");

        return new ZeMetricCollector(device, context, group);
    }

    ~ZeMetricCollector()
    {
        DisableCollection();

        ze_result_t status = ZE_RESULT_SUCCESS;
        status             = zeContextDestroy(context_);
        if (status != ZE_RESULT_SUCCESS)
        {
            LOG_ERROR("Failed to destroy Level Zero Context");
        }
    }

    void DisableCollection() { DisableMetrics(); }

    void resetAccumulatedMetrics()
    {
        accumulated_metrics_                               = 0;
        std::vector<MetricResult>* metric_results_to_reset = nullptr;
        {
            std::lock_guard<std::mutex> lock(metric_results_mutex_);
            metric_results_to_reset = current_metric_results_;
            current_metric_results_ =
                (current_metric_results_ == &metric_results1_) ? &metric_results2_ : &metric_results1_;
        }
        metric_results_to_reset->clear();
    }

    uint64_t getAccumulatedMetricsSinceLastReset()
    {
        std::vector<MetricResult>* metric_results_to_process = nullptr;
        {
            std::lock_guard<std::mutex> lock(metric_results_mutex_);
            metric_results_to_process = current_metric_results_;
            current_metric_results_ =
                (current_metric_results_ == &metric_results1_) ? &metric_results2_ : &metric_results1_;
        }

        uint64_t acc_sum = 0;
        for (const auto& metric : *metric_results_to_process)
        {
            acc_sum += metric.inst_alu0 + metric.inst_alu1 + metric.inst_xmx + metric.inst_send + metric.inst_ctrl;
        }

        metric_results_to_process->clear();

        accumulated_metrics_ += (acc_sum / 1000000UL);
        return accumulated_metrics_;
    }

private:
    ZeMetricCollector(ze_device_handle_t device, ze_context_handle_t context, zet_metric_group_handle_t group)
    : device_(device), context_(context), metric_group_(group)
    {
        if (device_ == nullptr || context_ == nullptr || metric_group_ == nullptr)
        {
            throw std::invalid_argument("Invalid device, context or metric group handle");
        }
        SetCollectionConfig();
        SetReportSize();
        SetMetricIndices();
        EnableMetrics();
    }

    void EnableMetrics()
    {
        if (collector_thread_ != nullptr || collector_state_ != COLLECTOR_STATE_IDLE)
        {
            throw std::runtime_error("Invalid collector state");
        }

        collector_state_.store(COLLECTOR_STATE_IDLE, std::memory_order_release);
        collector_thread_ = new std::thread(Collect, this);

        while (collector_state_.load(std::memory_order_acquire) != COLLECTOR_STATE_ENABLED)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void DisableMetrics()
    {
        if (collector_thread_ == nullptr)
        {
            throw std::runtime_error("Invalid collector state");
        }
        collector_state_.store(COLLECTOR_STATE_DISABLED, std::memory_order_release);
        collector_thread_->join();
        delete collector_thread_;
    }

    void SetCollectionConfig()
    {
        const auto cni = std::getenv("COLLECTOR_NOTIFY_INTERVAL");
        if (cni != nullptr)
            collector_notify_interval = std::stoul(cni);
        const auto csp = std::getenv("COLLECTOR_SAMPLING_PERIOD_NS");
        if (csp != nullptr)
            collector_sampling_period_ns = std::stoul(csp);
        const auto cd = std::getenv("COLLCETOR_DELAY_NS");
        if (cd != nullptr)
            collector_delay_ns = std::stoull(cd);
        LOG_DEBUG("COLLECTOR_NOTIFY_INTERVAL: {}", collector_notify_interval);
        LOG_DEBUG("COLLECTOR_SAMPLING_PERIOD_NS: {}", collector_sampling_period_ns);
        LOG_DEBUG("COLLECTOR_DELAY_NS: {}", collector_delay_ns);
    }

    void SetReportSize()
    {
        ze_result_t                   status = ZE_RESULT_SUCCESS;
        zet_metric_group_properties_t group_props {};
        group_props.stype = ZET_STRUCTURE_TYPE_METRIC_GROUP_PROPERTIES;
        status            = zetMetricGroupGetProperties(metric_group_, &group_props);
        if (status != ZE_RESULT_SUCCESS)
        {
            throw std::runtime_error(errorMap.at(status));
        }
        report_size_ = group_props.metricCount;
    }

    void SetMetricIndices()
    {
        auto GetMetricId = [&](zet_metric_group_handle_t group, const std::string& name) -> int
        {
            int         target       = -1;
            ze_result_t status       = ZE_RESULT_SUCCESS;
            uint32_t    metric_count = 0;
            status                   = zetMetricGet(group, &metric_count, nullptr);
            if (status != ZE_RESULT_SUCCESS || metric_count == 0)
            {
                return target;
            }

            std::vector<zet_metric_handle_t> metric_list(metric_count, nullptr);
            status = zetMetricGet(group, &metric_count, metric_list.data());
            if (status != ZE_RESULT_SUCCESS)
            {
                return target;
            }

            for (uint32_t i = 0; i < metric_count; ++i)
            {
                zet_metric_properties_t metric_props {};
                status = zetMetricGetProperties(metric_list[i], &metric_props);
                if (status != ZE_RESULT_SUCCESS)
                {
                    return target;
                }

                if (name == metric_props.name)
                {
                    target = i;
                    break;
                }
            }

            return target;
        };

        inst_alu0_id_ = GetMetricId(metric_group_, "XVE_INST_EXECUTED_ALU0_ALL");
        inst_alu1_id_ = GetMetricId(metric_group_, "XVE_INST_EXECUTED_ALU1_ALL");
        inst_xmx_id_  = GetMetricId(metric_group_, "XVE_INST_EXECUTED_XMX_ALL");
        inst_send_id_ = GetMetricId(metric_group_, "XVE_INST_EXECUTED_SEND_ALL");
        inst_ctrl_id_ = GetMetricId(metric_group_, "XVE_INST_EXECUTED_CONTROL_ALL");
        if (inst_alu0_id_ <= 0 || inst_alu1_id_ <= 0 || inst_xmx_id_ <= 0 || inst_send_id_ <= 0 || inst_ctrl_id_ <= 0)
        {
            throw std::runtime_error("Unable to find all required metrics");
        }
    }

    void AppendCalculatedMetrics(const std::vector<uint8_t>& storage)
    {
        if (storage.size() == 0)
        {
            return;
        }

        ze_result_t                    status = ZE_RESULT_SUCCESS;
        std::vector<zet_typed_value_t> report_list;

        uint32_t value_count = 0;
        status               = zetMetricGroupCalculateMetricValues(metric_group_,
                                                     ZET_METRIC_GROUP_CALCULATION_TYPE_METRIC_VALUES,
                                                     storage.size(),
                                                     storage.data(),
                                                     &value_count,
                                                     nullptr);
        if (status != ZE_RESULT_SUCCESS || value_count == 0)
        {
            LOG_ERROR("Some data was lost while trying to calculate metric values");
            return;
        }

        report_list.resize(value_count);
        status = zetMetricGroupCalculateMetricValues(metric_group_,
                                                     ZET_METRIC_GROUP_CALCULATION_TYPE_METRIC_VALUES,
                                                     storage.size(),
                                                     storage.data(),
                                                     &value_count,
                                                     report_list.data());
        if (status != ZE_RESULT_SUCCESS)
        {
            LOG_ERROR("Some data was lost while trying to calculate metric values");
            return;
        }
        report_list.resize(value_count);

        MetricResult metric_result;

        const zet_typed_value_t* report = report_list.data();
        while (report < report_list.data() + report_list.size())
        {
            const auto inst_alu0 = report[inst_alu0_id_].value.ui64;
            const auto inst_alu1 = report[inst_alu1_id_].value.ui64;
            const auto inst_xmx  = report[inst_xmx_id_].value.ui64;
            const auto inst_send = report[inst_send_id_].value.ui64;
            const auto inst_ctrl = report[inst_ctrl_id_].value.ui64;

            metric_result.inst_alu0 += inst_alu0;
            metric_result.inst_alu1 += inst_alu1;
            metric_result.inst_xmx += inst_xmx;
            metric_result.inst_send += inst_send;
            metric_result.inst_ctrl += inst_ctrl;

            report += report_size_;
        }

        {
            std::lock_guard<std::mutex> lock(metric_results_mutex_);
            current_metric_results_->push_back(metric_result);
        }
    }

    static void Collect(ZeMetricCollector* collector)
    {
        ze_result_t status = ZE_RESULT_SUCCESS;
        status = zetContextActivateMetricGroups(collector->context_, collector->device_, 1, &collector->metric_group_);
        if (status != ZE_RESULT_SUCCESS)
        {
            throw std::runtime_error(errorMap.at(status));
        }

        ze_event_pool_desc_t   event_pool_desc = {ZE_STRUCTURE_TYPE_EVENT_POOL_DESC,
                                                  nullptr,
                                                  ZE_EVENT_POOL_FLAG_HOST_VISIBLE,
                                                  1};
        ze_event_pool_handle_t event_pool      = nullptr;
        status = zeEventPoolCreate(collector->context_, &event_pool_desc, 0, nullptr, &event_pool);
        if (status != ZE_RESULT_SUCCESS)
        {
            throw std::runtime_error(errorMap.at(status));
        }

        ze_event_desc_t   event_desc = {ZE_STRUCTURE_TYPE_EVENT_DESC,
                                        nullptr,
                                        0,
                                        ZE_EVENT_SCOPE_FLAG_HOST,
                                        ZE_EVENT_SCOPE_FLAG_HOST};
        ze_event_handle_t event      = nullptr;
        status                       = zeEventCreate(event_pool, &event_desc, &event);
        if (status != ZE_RESULT_SUCCESS)
        {
            throw std::runtime_error(errorMap.at(status));
        }

        zet_metric_streamer_desc_t   metric_streamer_desc = {ZET_STRUCTURE_TYPE_METRIC_STREAMER_DESC,
                                                             nullptr,
                                                             collector->collector_notify_interval,
                                                             collector->collector_sampling_period_ns};
        zet_metric_streamer_handle_t metric_streamer      = nullptr;
        status                                            = zetMetricStreamerOpen(collector->context_,
                                       collector->device_,
                                       collector->metric_group_,
                                       &metric_streamer_desc,
                                       event,
                                       &metric_streamer);
        if (status != ZE_RESULT_SUCCESS)
        {
            throw std::runtime_error(errorMap.at(status));
        }

        collector->collector_state_.store(COLLECTOR_STATE_ENABLED, std::memory_order_release);

        std::vector<uint8_t> storage;
        while (collector->collector_state_.load(std::memory_order_acquire) != COLLECTOR_STATE_DISABLED)
        {
            status = zeEventHostSynchronize(event, collector->collector_delay_ns);
            if (status != ZE_RESULT_SUCCESS && status != ZE_RESULT_NOT_READY)
            {
                LOG_ERROR("Failed to synchronize event");
                continue;
            }

            size_t data_size = 0;
            status           = zetMetricStreamerReadData(metric_streamer, UINT32_MAX, &data_size, nullptr);
            if (status != ZE_RESULT_SUCCESS)
            {
                LOG_ERROR("Failed to read metric data size");
                continue;
            }

            if (data_size > 0)
            {
                storage.resize(data_size);
                status = zetMetricStreamerReadData(metric_streamer, UINT32_MAX, &data_size, storage.data());
                if (status != ZE_RESULT_SUCCESS)
                {
                    LOG_ERROR("Failed to read metric data");
                    continue;
                }
                storage.resize(data_size);
                if (storage.size() > 0)
                    collector->AppendCalculatedMetrics(storage);
            }
        }

        if (auto status = zetMetricStreamerClose(metric_streamer); status != ZE_RESULT_SUCCESS)
        {
            throw std::runtime_error(errorMap.at(status));
        }
        if (auto status = zeEventDestroy(event); status != ZE_RESULT_SUCCESS)
        {
            throw std::runtime_error(errorMap.at(status));
        }
        if (auto status = zeEventPoolDestroy(event_pool); status != ZE_RESULT_SUCCESS)
        {
            throw std::runtime_error(errorMap.at(status));
        }
        if (auto status = zetContextActivateMetricGroups(collector->context_, collector->device_, 0, nullptr);
            status != ZE_RESULT_SUCCESS)
        {
            throw std::runtime_error(errorMap.at(status));
        };
    }

private:
    ze_device_handle_t  device_  = nullptr;
    ze_context_handle_t context_ = nullptr;

    std::thread*                collector_thread_ = nullptr;
    std::atomic<CollectorState> collector_state_ {COLLECTOR_STATE_IDLE};

    zet_metric_group_handle_t metric_group_ = nullptr;
    // double buffering
    std::vector<MetricResult>  metric_results1_;
    std::vector<MetricResult>  metric_results2_;
    std::vector<MetricResult>* current_metric_results_ = &metric_results1_;
    std::mutex                 metric_results_mutex_;

    uint64_t accumulated_metrics_ = 0;
    uint32_t report_size_;
    int      inst_alu0_id_;
    int      inst_alu1_id_;
    int      inst_xmx_id_;
    int      inst_send_id_;
    int      inst_ctrl_id_;

public:
    uint32_t collector_notify_interval    = 32768;
    uint32_t collector_sampling_period_ns = 5000000;
    uint64_t collector_delay_ns           = 50000000;
};