#include "eco.hpp"
#include "devices/xpu_device.hpp"
#include "../src/logging.hpp"
#include <cassert>
#include <cstdio>
#include <unordered_map>
#include <tuple>
#include <cstring>
#include <thread>
#include <type_traits>

#define CHECK(x)                                                                                                       \
    if (x != true)                                                                                                     \
    {                                                                                                                  \
        exit(-1);                                                                                                      \
    }

std::unordered_map<std::string, std::tuple<unsigned, unsigned>> known_devices_power;
std::unordered_map<std::string, std::tuple<unsigned, unsigned>> known_devices_current;

static void run_gemm(ze_kernel_handle_t        kernel,
                     ze_device_handle_t        device,
                     ze_context_handle_t       context,
                     const std::vector<float>& a,
                     const std::vector<float>& b,
                     std::vector<float>&       c,
                     unsigned                  size)
{
    ze_result_t status = ZE_RESULT_SUCCESS;

    uint32_t group_size[3] = {0};
    status = zeKernelSuggestGroupSize(kernel, size, size, 1, &(group_size[0]), &(group_size[1]), &(group_size[2]));
    assert(status == ZE_RESULT_SUCCESS);

    void*                      dev_a      = nullptr;
    ze_device_mem_alloc_desc_t alloc_desc = {ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC, nullptr, 0, 0};
    status = zeMemAllocDevice(context, &alloc_desc, size * size * sizeof(float), 64, device, &dev_a);
    assert(status == ZE_RESULT_SUCCESS);
    void* dev_b = nullptr;
    status      = zeMemAllocDevice(context, &alloc_desc, size * size * sizeof(float), 64, device, &dev_b);
    assert(status == ZE_RESULT_SUCCESS);
    void* dev_c = nullptr;
    status      = zeMemAllocDevice(context, &alloc_desc, size * size * sizeof(float), 64, device, &dev_c);
    assert(status == ZE_RESULT_SUCCESS);

    status = zeKernelSetGroupSize(kernel, group_size[0], group_size[1], group_size[2]);
    assert(status == ZE_RESULT_SUCCESS);

    status = zeKernelSetArgumentValue(kernel, 0, sizeof(dev_a), &dev_a);
    assert(status == ZE_RESULT_SUCCESS);
    status = zeKernelSetArgumentValue(kernel, 1, sizeof(dev_a), &dev_b);
    assert(status == ZE_RESULT_SUCCESS);
    status = zeKernelSetArgumentValue(kernel, 2, sizeof(dev_a), &dev_c);
    assert(status == ZE_RESULT_SUCCESS);
    status = zeKernelSetArgumentValue(kernel, 3, sizeof(size), &size);
    assert(status == ZE_RESULT_SUCCESS);

    ze_command_list_desc_t   cmd_list_desc = {ZE_STRUCTURE_TYPE_COMMAND_LIST_DESC, nullptr, 0, 0};
    ze_command_list_handle_t cmd_list      = nullptr;
    status                                 = zeCommandListCreate(context, device, &cmd_list_desc, &cmd_list);
    assert(status == ZE_RESULT_SUCCESS);

    status = zeCommandListAppendMemoryCopy(cmd_list, dev_a, a.data(), size * size * sizeof(float), nullptr, 0, nullptr);
    assert(status == ZE_RESULT_SUCCESS);
    status = zeCommandListAppendMemoryCopy(cmd_list, dev_b, b.data(), size * size * sizeof(float), nullptr, 0, nullptr);
    assert(status == ZE_RESULT_SUCCESS);

    status = zeCommandListAppendBarrier(cmd_list, nullptr, 0, nullptr);
    assert(status == ZE_RESULT_SUCCESS);

    ze_event_pool_desc_t   event_pool_desc = {ZE_STRUCTURE_TYPE_EVENT_POOL_DESC,
                                              nullptr,
                                              ZE_EVENT_POOL_FLAG_KERNEL_TIMESTAMP | ZE_EVENT_POOL_FLAG_HOST_VISIBLE,
                                              1};
    ze_event_pool_handle_t event_pool      = nullptr;
    status                                 = zeEventPoolCreate(context, &event_pool_desc, 0, nullptr, &event_pool);
    assert(status == ZE_RESULT_SUCCESS);

    ze_event_desc_t   event_desc = {ZE_STRUCTURE_TYPE_EVENT_DESC,
                                    nullptr,
                                    0,
                                    ZE_EVENT_SCOPE_FLAG_HOST,
                                    ZE_EVENT_SCOPE_FLAG_HOST};
    ze_event_handle_t event      = nullptr;
    zeEventCreate(event_pool, &event_desc, &event);
    assert(status == ZE_RESULT_SUCCESS);

    ze_group_count_t dim = {size / group_size[0], size / group_size[1], 1};
    status               = zeCommandListAppendLaunchKernel(cmd_list, kernel, &dim, event, 0, nullptr);
    assert(status == ZE_RESULT_SUCCESS);

    status = zeCommandListAppendBarrier(cmd_list, nullptr, 0, nullptr);
    assert(status == ZE_RESULT_SUCCESS);

    status = zeCommandListAppendMemoryCopy(cmd_list, c.data(), dev_c, size * size * sizeof(float), nullptr, 0, nullptr);
    assert(status == ZE_RESULT_SUCCESS);

    status = zeCommandListClose(cmd_list);
    assert(status == ZE_RESULT_SUCCESS);

    ze_command_queue_desc_t   cmd_queue_desc = {ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC,
                                                nullptr,
                                                0,
                                                0,
                                                0,
                                                ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS,
                                                ZE_COMMAND_QUEUE_PRIORITY_NORMAL};
    ze_command_queue_handle_t cmd_queue      = nullptr;
    status                                   = zeCommandQueueCreate(context, device, &cmd_queue_desc, &cmd_queue);
    assert(status == ZE_RESULT_SUCCESS && cmd_queue != nullptr);

    status = zeCommandQueueExecuteCommandLists(cmd_queue, 1, &cmd_list, nullptr);
    assert(status == ZE_RESULT_SUCCESS);

    status = zeCommandQueueSynchronize(cmd_queue, UINT32_MAX);
    assert(status == ZE_RESULT_SUCCESS);

    status = zeCommandQueueDestroy(cmd_queue);
    assert(status == ZE_RESULT_SUCCESS);

    status = zeCommandListDestroy(cmd_list);
    assert(status == ZE_RESULT_SUCCESS);

    status = zeMemFree(context, dev_a);
    assert(status == ZE_RESULT_SUCCESS);
    status = zeMemFree(context, dev_b);
    assert(status == ZE_RESULT_SUCCESS);
    status = zeMemFree(context, dev_c);
    assert(status == ZE_RESULT_SUCCESS);

    status = zeEventDestroy(event);
    assert(status == ZE_RESULT_SUCCESS);
    status = zeEventPoolDestroy(event_pool);
    assert(status == ZE_RESULT_SUCCESS);
}

bool test_init_eco_xpu_device(int devID)
{
    std::shared_ptr<Device> xpu_device = std::make_unique<XPUDevice>(devID);
    auto                    eco        = std::make_unique<Eco>(xpu_device);

    return true;
}

bool test_xpu_min_max_limits(int devID, bool useAmperes)
{
    auto xpu_device = std::make_unique<XPUDevice>(devID, useAmperes);
    auto limits     = xpu_device->getMinMaxLimitInWatts();

    std::unordered_map<std::string, std::tuple<unsigned, unsigned>>& known_devices =
        useAmperes ? known_devices_current : known_devices_power;

    unsigned expectedMinLimit;
    unsigned expectedMaxLimit;

    auto key = xpu_device->getName();
    if (known_devices.find(key) == known_devices.end())
    {
        LOG_ERROR("Unsupported device: {}", key);
        return false;
    }
    std::tie(expectedMinLimit, expectedMaxLimit) = known_devices[key];

    if ((limits.first != expectedMinLimit) || (limits.second != expectedMaxLimit))
    {
        LOG_ERROR("Error: expected power limits differ from actual read ones");
        return false;
    }

    unsigned expectedMiddleLimit = (expectedMinLimit + expectedMaxLimit) / 2;

    xpu_device->setPowerLimitInMicroWatts(expectedMiddleLimit * 1000000);

    auto currentLimit = xpu_device->getPowerLimitInWatts();

    // Restore max limit
    xpu_device->setPowerLimitInMicroWatts(expectedMaxLimit * 1000000);
    if (currentLimit != expectedMiddleLimit)
    {
        LOG_ERROR("Expected max power limit({}) differ from actually read({})", expectedMiddleLimit, limits.second);
        return false;
    }

    return true;
}

bool test_xpu_power_limit_resetting(int devID)
{
    // Create XPU device and set power limit to half of its max value
    auto xpu_device = std::make_unique<XPUDevice>(devID, false);
    auto limits     = xpu_device->getMinMaxLimitInWatts();

    unsigned currentMinLimit;
    unsigned currentMaxLimit;

    auto key = xpu_device->getName();
    if (known_devices_power.find(key) == known_devices_power.end())
    {
        LOG_ERROR("Unsupported device: {}", key);
        return false;
    }
    std::tie(currentMinLimit, currentMaxLimit) = known_devices_power[key];

    unsigned currentMiddleLimit = (currentMinLimit + currentMaxLimit) / 2;

    xpu_device->setPowerLimitInMicroWatts(currentMiddleLimit * 1000000);

    auto currentLimit = xpu_device->getPowerLimitInWatts();

    if (currentLimit != currentMiddleLimit)
    {
        LOG_ERROR("Expected power limit({}) differ from actually read({})", currentMiddleLimit, currentLimit);
        return false;
    }
    // After setting sustained power limit we will create now xpu_device with power limit and already set sustained
    // power limit should be reverted to max available value
    xpu_device.reset();
    xpu_device          = std::make_unique<XPUDevice>(devID, true);
    auto sustainedLimit = xpu_device->getPowerLimitSustained();

    if (sustainedLimit != currentMaxLimit)
    {
        LOG_ERROR("Expected sustained power limit({}) differ from actually read({})", currentMaxLimit, sustainedLimit);
        return false;
    }
    return true;
}

bool test_xpu_reset_and_perf_counter(int devID)
{
    // This will initialize L0
    std::shared_ptr<Device> xpu_device = std::make_unique<XPUDevice>(devID);
    // We cannot query an underlying device, so we need to create it again
    // Create driver
    unsigned int count  = 0;
    auto         status = zeDriverGet(&count, nullptr);
    assert(status == ZE_RESULT_SUCCESS);
    assert(count != 0);
    std::vector<ze_driver_handle_t> phdrivers(count, nullptr);
    status = zeDriverGet(&count, phdrivers.data());
    assert(status == ZE_RESULT_SUCCESS);
    ze_driver_handle_t driver = *(phdrivers.begin());

    // Create device
    count  = 0;
    status = zeDeviceGet(driver, &count, nullptr);
    assert(status == ZE_RESULT_SUCCESS);
    assert(count != 0);
    assert(count > devID);
    std::vector<ze_device_handle_t> phdevices(count, nullptr);
    status                    = zeDeviceGet(driver, &count, phdevices.data());
    ze_device_handle_t device = phdevices[devID];

    // Prepare data for calculation
    unsigned           mat_size = 256;
    std::vector<float> a(mat_size * mat_size, 0.128f);
    std::vector<float> b(mat_size * mat_size, 0.256f);
    std::vector<float> c(mat_size * mat_size, 0.0f);

    // Load binary
    std::string          module_name = "gemm.spv";
    std::vector<uint8_t> binary;
    std::ifstream        stream(module_name, std::ios::in | std::ios::binary);
    assert(stream.good());
    stream.seekg(0, std::ifstream::end);
    size_t size = stream.tellg();
    stream.seekg(0, std::ifstream::beg);
    assert(size != 0);
    binary.resize(size);
    stream.read(reinterpret_cast<char*>(binary.data()), size);

    // Create context
    ze_context_handle_t context      = nullptr;
    ze_context_desc_t   context_desc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC, nullptr, 0};
    status                           = zeContextCreate(driver, &context_desc, &context);
    assert(status == ZE_RESULT_SUCCESS);

    // Create kernel
    ze_module_desc_t   module_desc = {ZE_STRUCTURE_TYPE_MODULE_DESC,
                                      nullptr,
                                      ZE_MODULE_FORMAT_IL_SPIRV,
                                      static_cast<uint32_t>(binary.size()),
                                      binary.data(),
                                      nullptr,
                                      nullptr};
    ze_module_handle_t module      = nullptr;
    status                         = zeModuleCreate(context, device, &module_desc, &module, nullptr);
    assert(status == ZE_RESULT_SUCCESS && module != nullptr);
    ze_kernel_desc_t   kernel_desc = {ZE_STRUCTURE_TYPE_KERNEL_DESC, nullptr, 0, "GEMM"};
    ze_kernel_handle_t kernel      = nullptr;
    status                         = zeKernelCreate(module, &kernel_desc, &kernel);
    assert(status == ZE_RESULT_SUCCESS && kernel != nullptr);

    // Check reset() and getPerfCounter()
    if (const auto cnt = xpu_device->getPerfCounter(); cnt != 0)
    {
        LOG_ERROR("Expected perf counter to equal 0, but got: {}", cnt);
        return false;
    }
    run_gemm(kernel, device, context, a, b, c, mat_size);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if (const auto cnt = xpu_device->getPerfCounter(); cnt == 0)
    {
        LOG_ERROR("Expected perf counter to be > 0, but got 0");
        return false;
    }
    else
    {
        LOG_INFO("Instructions (in millions): {}", cnt);
    }
    xpu_device->reset();
    if (const auto cnt = xpu_device->getPerfCounter(); cnt != 0)
    {
        LOG_ERROR("Expected perf counter to equal 0, but got: {}", cnt);
        return false;
    }
    run_gemm(kernel, device, context, a, b, c, mat_size);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if (const auto cnt = xpu_device->getPerfCounter(); cnt == 0)
    {
        LOG_ERROR("Expected perf counter to be > 0, but got 0");
        return false;
    }
    else
    {
        LOG_INFO("Instructions (in millions): {}", cnt);
    }

    return true;
}

void init_devices_data()
{
    known_devices_power["Intel(R) Data Center GPU Max 1100"]   = std::make_tuple(150, 300);
    known_devices_current["Intel(R) Data Center GPU Max 1100"] = std::make_tuple(14, 48);
}

int main()
{
    LOAD_ENV_LEVELS()
    init_devices_data();

    CHECK(test_init_eco_xpu_device(0));
    CHECK(test_xpu_min_max_limits(0, false));  // Power limit: sustained (watts)
    CHECK(test_xpu_min_max_limits(0, true));   // Power limit: peak (amperes)
    CHECK(test_xpu_power_limit_resetting(0));
    CHECK(test_xpu_reset_and_perf_counter(0));

    return 0;
}
