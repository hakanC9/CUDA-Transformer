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

#include "devices/cuda_device.hpp"

static inline
void logCurrentRangeGSS(int a, int leftCandidateInMilliWatts, int rightCandidateInMilliWatts, int b)
{
    std::cout << "#--------------------------------\n"
              << "# Current GSS range: |"
              << a << " "
              << leftCandidateInMilliWatts << " "
              << rightCandidateInMilliWatts << " "
              << b << "|\n"
              << "#--------------------------------\n";
}

static inline
std::string parseArgvCommandToString(int argc, char** argv)
{
    std::stringstream str;
    str << "# ";
    for (int i=1; i < argc; i++) {
        str << argv[i] << " ";
    }
    str << "\n";
    return str.str();
}

static inline
std::string generateUniqueDir()
{
    std::string dir = "gpu_experiment_" +
        std::to_string(
            std::chrono::system_clock::to_time_t(
                  std::chrono::high_resolution_clock::now()));
    dir += "/";
    const int dir_err = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (-1 == dir_err) {
        printf("Error creating experiment result directory!\n");
        exit(1);
    }
    return dir;
}

static inline
int long long readValueFromFile (std::string fileName)
{
    std::ifstream file (fileName.c_str());
    std::string line;
    long long int value = -1;
    if (file.is_open())
    {
        while ( getline (file, line) )
        {
            value = atoi(line.c_str());
        }
        file.close();
    }
    else
    {
        std::cerr << "cannot read the value from file: " << fileName << "\n"
                  << "file not open\n";
    }
    return value;
}

CudaDevice::CudaDevice(int devID) :
    deviceID_(devID) // and then this field shall not be a member of this class as the API allows for access to any device
{
    std::cout << "[DEBUG]: CudaDevice constructor called!\n";
    int major;
    CUresult result;
    CUdevice device {deviceID_};

    result = cuInit(0);
    if (result != CUDA_SUCCESS)
    {
        printf("Error code %d on cuInit\n", result);
        exit(-1);
    }
    result = cuDeviceGet(&device,0);
    if (result != CUDA_SUCCESS)
    {
        printf("Error code %d on cuDeviceGet\n", result);
        exit(-1);
    }

    result = cuDeviceGetAttribute (&major, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, device);
    if (result != CUDA_SUCCESS)
    {
        printf("Error code %d on cuDeviceGetAttribute\n", result);
        exit(-1);
    }
    // NVML INITIALIZATIONS
    nvResult_ = nvmlInit();
    if (NVML_SUCCESS != nvResult_)
    {
        printf("Failed to initialize NVML: %s\n", nvmlErrorString(nvResult_));
        return;
    }
    nvResult_ = nvmlDeviceGetCount(&deviceCount_);
    if (NVML_SUCCESS != nvResult_)
    {
        printf("Failed to query device count: %s\n", nvmlErrorString(nvResult_));
        return;
    }
    printf("Found %d device%s\n\n", deviceCount_, deviceCount_ != 1 ? "s" : "");
    initDeviceHandles();
    std::cout << "DEBUG device handles initialized succesfully" << std::endl;
    defaultPowerLimitInWatts_ = this->getPowerLimitInWatts();
}

double CudaDevice::getPowerLimitInWatts() const
{
    unsigned currPowerLimitInMilliWatts = 0;
    nvmlReturn_t nvResult = nvmlDeviceGetEnforcedPowerLimit (deviceHandles_[deviceID_], &currPowerLimitInMilliWatts);
    if (NVML_SUCCESS != nvResult)
    {
        printf("Failed to GET current power limit: %s\n", nvmlErrorString(nvResult));
        return -1;
    }
    return (double)currPowerLimitInMilliWatts / 1000;
}

std::string CudaDevice::getName() const
{
    constexpr unsigned maxLength = 96;
    char name[maxLength];
    nvmlReturn_t nvResult = nvmlDeviceGetName (deviceHandles_[deviceID_], name, maxLength);
    if (NVML_SUCCESS != nvResult)
    {
        printf("Failed to GET device name: %s\n", nvmlErrorString(nvResult));
        return std::string("Unknown GPU");
    }
    return std::string(name);
}

std::pair<unsigned, unsigned> CudaDevice::getMinMaxLimitInWatts() const
{
    unsigned min = 0, max = 0;
    nvmlReturn_t nvResult = nvmlDeviceGetPowerManagementLimitConstraints (deviceHandles_[deviceID_], &min, &max);
    if (NVML_SUCCESS != nvResult)
    {
        printf("Failed to GET min/max power limit: %s\n", nvmlErrorString(nvResult));
    }
    return std::make_pair(min/1000, max/1000);
}

void CudaDevice::setPowerLimitInMicroWatts(unsigned long limitInMicroW)
{
    unsigned long limitInMilliWatts = limitInMicroW / 1e3;
    nvmlReturn_t nvResult = nvmlDeviceSetPowerManagementLimit (deviceHandles_[deviceID_], limitInMilliWatts);
    if (NVML_SUCCESS != nvResult)
    {
        printf("Failed to SET current power limit %ld [mW]: %s\n", limitInMilliWatts, nvmlErrorString(nvResult));
        return;
    }
}

void CudaDevice::reset()
{
    std::ofstream kernelCounterFile;
    kernelCounterFile.open("kernels_count", std::ios::out | std::ios::trunc);
    kernelCounterFile << "0";
    kernelCounterFile.close();
}

double CudaDevice::getCurrentPowerInWatts(std::optional<Domain>) const
{
    unsigned power;
    nvmlReturn_t nvResult = nvmlDeviceGetPowerUsage(deviceHandles_[deviceID_], &power);
    if (NVML_SUCCESS != nvResult)
    {
        printf("Failed to get power usage: %s\n", nvmlErrorString(nvResult));
        return -1.0;
    }
    return (double)power/1000.0;
}

void CudaDevice::initDeviceHandles()
{
    nvmlDevice_t nvDevice;
    nvmlReturn_t nvResult;
    deviceHandles_.resize(deviceCount_);
    for (unsigned i = 0; i < deviceCount_; i++)
    {
        nvResult = nvmlDeviceGetHandleByIndex(i, &nvDevice);
        if (NVML_SUCCESS != nvResult)
        {
            printf("Failed to get handle for device 1: %s\n", nvmlErrorString(nvResult));
            return;
        }
        deviceHandles_[i] = nvDevice;
    }
}

unsigned long long int CudaDevice::getPerfCounter() const
{
    long long int kernelsCountTmp {-1};
    do
    {
        kernelsCountTmp = readValueFromFile("./kernels_count");
    }
    while (kernelsCountTmp == -1);

    return (unsigned long long)kernelsCountTmp;
}

void CudaDevice::restoreDefaultLimits()
{
    setPowerLimitInMicroWatts(1e6 * defaultPowerLimitInWatts_);

}
