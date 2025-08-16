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

#include "devices/intel_device.hpp"
#include "devices/common_const_intel.hpp"

#include <cstring>
#include <sstream>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <chrono>


#define MAX_CPUS		1024

static inline int readLimitFromFile (std::string fileName) {
    std::ifstream limitFile (fileName.c_str());
    std::string line;
    int limit = -1;
    if (limitFile.is_open()){
        while ( getline (limitFile, line) ){
            limit = atoi(line.c_str());
        }
        limitFile.close();
    } else {
        std::cerr << "cannot read the limit file: " << fileName << "\n"
                  << "file not open\n";
    }
    return limit;
}

static inline void writeLimitToFile (std::string fileName, int limit) {
    std::ofstream outfile (fileName.c_str(), std::ios::out | std::ios::trunc);
    if (outfile.is_open()){
        outfile << limit;
    } else {
        std::cerr << "cannot write the limit to file " << fileName << "\n"
                  << "file not open\n";
    }
    outfile.close();
}

IntelDevice::IntelDevice()
{
    detectCPU();
    detectPackages();
    detectPowerCapsAvailability();
    prepareRaplDirsFromAvailableDomains();
    readAndStoreDefaultLimits();
    currentPowerLimitInWatts_ = totalPackages_ * raplDefaultCaps_.defaultConstrPKG_->longPower/ 1e6;
    initPerformanceCounters();
    initRaplObjectsForEachPKG();
    checkIdlePowerConsumption();
}

void IntelDevice::initRaplObjectsForEachPKG()
{
    for (auto&& cpuCore : this->getPkgToFirstCoreMap())
    {
        raplVec_.emplace_back(cpuCore, this->getAvailablePowerDomains());
        std::cout << "INFO: created RAPL object for core " << cpuCore << " in DeviceStateAccumulator.\n";
    }
}

std::pair<unsigned, unsigned> IntelDevice::getMinMaxLimitInWatts() const
{
    // This method assumes that the max power in Intel CPUs is identical to a default
    // power cap, which is generally true but the proper information about the max
    // device power might be read from RAPL::pkg_max_power() method.
    //
    // For the purpose of the SPLiT tools collection assuming that the maximal power
    // is identitical to the default power cap is good enough. On the other hand
    // max available power is sometimes up to 4x higher than the Thermal Design Power (TDP),
    // allowing for occasional power consumption spikes so maybe for other type of
    // research the actual max available power will be more useful. It needs to be noted
    // that CPU working above TDP would require much more cooling and would throttle much faster.
    //
    // For MIN power it returns idle power consumption mesured for the CPU PKG at the object creation.
    return std::make_pair(idlePowerConsumption_, (totalPackages_ * raplDefaultCaps_.defaultConstrPKG_->longPower) / 1000000);
}

std::string IntelDevice::getName() const
{
    return mapCpuFamilyName(model_);
}

AvailableRaplPowerDomains IntelDevice::getAvailablePowerDomains() {
    return devicePowerProfile_;
}

void IntelDevice::detectCPU() {

	FILE *fff;

	int family;
	char buffer[BUFSIZ];
    char *result;
	char vendor[BUFSIZ];

	fff = fopen("/proc/cpuinfo", "r");
// TODO: rework above check and below loop
	while(1) {
		result=fgets(buffer,BUFSIZ,fff);
		if (result==NULL) break;

		if (!strncmp(result,"vendor_id",8)) {
			sscanf(result,"%*s%*s%s", vendor);

			if (strncmp(vendor, "GenuineIntel", 12)) {
				printf("%s not an Intel chip\n", vendor);
			}
		}
		if (!strncmp(result,"cpu family",10)) {
			sscanf(result,"%*s%*s%*s%d", &family);
			if (family!=6) {
				printf("Wrong CPU family %d\n",family);
			}
		}
		if (!strncmp(result,"model",5)) {
			sscanf(result,"%*s%*s%d", &model_);
		}
	}
	fclose(fff);

	std::fstream cpuParams;
	std::string filename = "cpu_params.txt";
	cpuParams.open(filename.c_str());
	if (cpuParams.is_open()) {
		cpuParams.close();
	}
	else {
		std::ofstream outStream (filename.c_str(), std::ios::out | std::ios::trunc);
		outStream << "vendor: " << vendor << "\n";
		outStream << "Family:  " << family << "\n";
		outStream << "Family name:  " << mapCpuFamilyName(model_) << "\n";
		outStream.close();
	}
	if (family == 6) {
		std::cout << "Detected suported CPU family\nModel: " << mapCpuFamilyName(model_) << "\n";
	}
}

void IntelDevice::detectPackages() {

	FILE *fff;
	int package;
	std::stringstream filenameStream;

	printf("\t");
	for(int i = 0; i < MAX_CPUS; i++) {
		filenameStream << "/sys/devices/system/cpu/cpu" << i << "/topology/physical_package_id";
		fff = fopen(filenameStream.str().c_str(), "r");
		if (fff == NULL) break;
		fscanf(fff, "%d", &package);
		printf("%d (%d)", i, package);
		if (i % 8 == 7) printf("\n\t"); else printf(", ");
		fclose(fff);

		if (pkgToFirstCoreMap_.size() <= package) {
			totalPackages_++;
			pkgToFirstCoreMap_.push_back(i);
		}
		filenameStream.str("");
		totalCores_++;
	}

	printf("\n");

	printf("\tDetected %d cores in %d packages\n\n", totalCores_, totalPackages_);
}

std::string IntelDevice::mapCpuFamilyName(int model) const
{
	switch(model)
    {
		case CPU_SANDYBRIDGE:
			return std::string("Sandybridge");
		case CPU_SANDYBRIDGE_EP:
			return std::string("Sandybridge-EP");
		case CPU_IVYBRIDGE:
			return std::string("Ivybridge");
		case CPU_IVYBRIDGE_EP:
			return std::string("Ivybridge-EP");
		case CPU_HASWELL:
		case CPU_HASWELL_ULT:
		case CPU_HASWELL_GT3E:
			return std::string("Haswell");
		case CPU_HASWELL_EP:
			return std::string("Haswell-EP");
		case CPU_BROADWELL:
		case CPU_BROADWELL_GT3E:
			return std::string("Broadwell");
		case CPU_BROADWELL_EP:
			return std::string("Broadwell-EP");
		case CPU_SKYLAKE:
		case CPU_SKYLAKE_HS:
			return std::string("Skylake");
		case CPU_SKYLAKE_X:
			return std::string("Skylake-X");
		case CPU_ICELAKE_U:
		case CPU_ICELAKE_Y:
			return std::string("Icelake");
		case CPU_ICELAKE_DE:
			return std::string("Icelake-DE");
		case CPU_ICELAKE_SP:
			return std::string("Icelake-SP");
		case CPU_KABYLAKE:
		case CPU_KABYLAKE_MOBILE:
			return std::string("Kaby Lake");
		case CPU_KNIGHTS_LANDING:
			return std::string("Knight's Landing");
		case CPU_KNIGHTS_MILL:
			return std::string("Knight's Mill");
		case CPU_ATOM_GOLDMONT:
		case CPU_ATOM_GEMINI_LAKE:
		case CPU_ATOM_DENVERTON:
			return std::string("Atom");
		default:
            return std::string("Unsupported model");
	}
}

void IntelDevice::detectPowerCapsAvailability() {
	if (model_ < 0)
    {
		printf("\tUnsupported CPU model %d\n", model_);
	} //TODO - throw exception and handle it in Eco/Rapl

	switch(model_) {

		case CPU_SANDYBRIDGE_EP:
		case CPU_IVYBRIDGE_EP:
			devicePowerProfile_ = AvailableRaplPowerDomains(true, false, true, false, false);
			break;

		case CPU_HASWELL_EP:
		case CPU_BROADWELL_EP: // according to real system (PC Xeon E5) there is no PP0
		case CPU_ICELAKE_SP: // verified with real system
		case CPU_SKYLAKE_X: // TODO: To be checked on Skylake X if possible.
			devicePowerProfile_ = AvailableRaplPowerDomains(false, false, true, false, true);
			break;

		case CPU_KNIGHTS_LANDING:
		case CPU_KNIGHTS_MILL:
			devicePowerProfile_ = AvailableRaplPowerDomains(false, false, true, false, true);
			break;

		case CPU_SANDYBRIDGE:
		case CPU_IVYBRIDGE:
			devicePowerProfile_ = AvailableRaplPowerDomains(true, true, false, false, false);
			break;

		case CPU_HASWELL:
		case CPU_HASWELL_ULT:
		case CPU_HASWELL_GT3E:
		case CPU_BROADWELL:
		case CPU_BROADWELL_GT3E:
		case CPU_ATOM_GOLDMONT:
		case CPU_ATOM_GEMINI_LAKE:
		case CPU_ATOM_DENVERTON:
			devicePowerProfile_ = AvailableRaplPowerDomains(true, true, true, false, false);
			break;

		case CPU_SKYLAKE:
		case CPU_SKYLAKE_HS:
		case CPU_KABYLAKE: // TODO: confirm below settings with some doc. Actually des19 (KabyLake) has PKG/PP0/DRAM
						   //       and does not support PP1. No info about PSys either.
		case CPU_KABYLAKE_MOBILE:
			devicePowerProfile_ = AvailableRaplPowerDomains(true, false, true, true, false);
			break;
	}
}

void IntelDevice::prepareRaplDirsFromAvailableDomains()
{
    // TODO: rework below loop and verify correctness for KNL and KNM
    for (unsigned i = 0; i < totalPackages_; i++) {
        raplDirs_.packagesDirs_.emplace_back(raplDirs_.raplBaseDirectory_ + std::to_string(i) + "/");
        if (devicePowerProfile_.pp0_) {
            raplDirs_.pp0Dirs_.emplace_back(raplDirs_.raplBaseDirectory_ + std::to_string(i) + ":0/");
        } else {
            if (devicePowerProfile_.dram_) {
                raplDirs_.dramDirs_.emplace_back(raplDirs_.raplBaseDirectory_ + std::to_string(i) + ":0/");
            }
        }
        if (devicePowerProfile_.pp1_) {
            raplDirs_.pp1Dirs_.emplace_back(raplDirs_.raplBaseDirectory_ + std::to_string(i) + ":1/");
            if (devicePowerProfile_.dram_) {
                raplDirs_.dramDirs_.emplace_back(raplDirs_.raplBaseDirectory_ + std::to_string(i) + ":2/");
            }
        } else {
            if (devicePowerProfile_.dram_ && raplDirs_.dramDirs_.empty()) {
                raplDirs_.dramDirs_.emplace_back(raplDirs_.raplBaseDirectory_ + std::to_string(i) + ":1/");
            }
        }
    }
}

bool IntelDevice::isDomainAvailable(Domain dom)
{
	return devicePowerProfile_.availableDomainsSet_.find(dom) != devicePowerProfile_.availableDomainsSet_.end();
}

void IntelDevice::readAndStoreDefaultLimits()
{
    auto fileExists = boost::filesystem::exists(defaultLimitsFile_);
    std::ofstream fs;
    if(!fileExists) {
        fs.open(defaultLimitsFile_, std::ios::out | std::ios::trunc);
    }
    raplDefaultCaps_.defaultConstrPKG_ = std::make_shared<Constraints>(
		                                              readLimitFromFile(raplDirs_.packagesDirs_[0] + raplDirs_.pl0dir_),
                                                      readLimitFromFile(raplDirs_.packagesDirs_[0] + raplDirs_.pl1dir_),
                                                      readLimitFromFile(raplDirs_.packagesDirs_[0] + raplDirs_.window0dir_),
                                                      readLimitFromFile(raplDirs_.packagesDirs_[0] + raplDirs_.window1dir_));
    if (!fileExists) {
        fs << "PKG\n" << *raplDefaultCaps_.defaultConstrPKG_;
    }
    if (devicePowerProfile_.pp0_) {
        raplDefaultCaps_.defaultConstrPP0_ = std::make_shared<SubdomainInfo>(
			                                                  readLimitFromFile(raplDirs_.pp0Dirs_[0] + raplDirs_.pl0dir_),
                                                              readLimitFromFile(raplDirs_.pp0Dirs_[0] + raplDirs_.window0dir_),
                                                              readLimitFromFile(raplDirs_.pp0Dirs_[0] + raplDirs_.isEnabledDir_));
        if (!fileExists) {
            fs << "PP0\n" << *raplDefaultCaps_.defaultConstrPP0_;
        }
    }
    if (devicePowerProfile_.pp1_) {
        raplDefaultCaps_.defaultConstrPP1_ = std::make_shared<SubdomainInfo>(
			                                                  readLimitFromFile(raplDirs_.pp1Dirs_[0] + raplDirs_.pl0dir_),
                                                              readLimitFromFile(raplDirs_.pp1Dirs_[0] + raplDirs_.window0dir_),
                                                              readLimitFromFile(raplDirs_.pp1Dirs_[0] + raplDirs_.isEnabledDir_));
        if (!fileExists) {
            fs << "PP1\n" << *raplDefaultCaps_.defaultConstrPP1_;
        }
    }
    if (devicePowerProfile_.dram_) {
        raplDefaultCaps_.defaultConstrDRAM_ = std::make_shared<SubdomainInfo>(
			                                                   readLimitFromFile(raplDirs_.dramDirs_[0] + raplDirs_.pl0dir_),
                                                               readLimitFromFile(raplDirs_.dramDirs_[0] + raplDirs_.window0dir_),
                                                               readLimitFromFile(raplDirs_.dramDirs_[0] + raplDirs_.isEnabledDir_));
        if (!fileExists) {
            fs << "DRAM\n" << *raplDefaultCaps_.defaultConstrDRAM_;
        }
    }
    if (fs.is_open()) {
        fs.close();
    }
}

void IntelDevice::restoreDefaultLimits ()
{
    currentPowerLimitInWatts_ = totalPackages_ * raplDefaultCaps_.defaultConstrPKG_->longPower / 1e6;
    //assume that both PKGs has the same limits
    for (auto& currentPkgDir : raplDirs_.packagesDirs_) {
        writeLimitToFile (currentPkgDir + raplDirs_.pl0dir_, raplDefaultCaps_.defaultConstrPKG_->longPower);
        writeLimitToFile (currentPkgDir + raplDirs_.pl1dir_, raplDefaultCaps_.defaultConstrPKG_->shortPower);
        writeLimitToFile (currentPkgDir + raplDirs_.window0dir_, raplDefaultCaps_.defaultConstrPKG_->longWindow);
        writeLimitToFile (currentPkgDir + raplDirs_.window1dir_, raplDefaultCaps_.defaultConstrPKG_->shortWindow);
    }
    for (auto& currentPP0dir : raplDirs_.pp0Dirs_) {
        writeLimitToFile (currentPP0dir + raplDirs_.pl0dir_, raplDefaultCaps_.defaultConstrPP0_->powerLimit);
        writeLimitToFile (currentPP0dir + raplDirs_.window0dir_, raplDefaultCaps_.defaultConstrPP0_->timeWindow);
        writeLimitToFile (currentPP0dir + raplDirs_.isEnabledDir_, raplDefaultCaps_.defaultConstrPP0_->isEnabled);
    }
    for (auto& currentPP1dir : raplDirs_.pp1Dirs_) {
        writeLimitToFile (currentPP1dir + raplDirs_.pl0dir_, raplDefaultCaps_.defaultConstrPP1_->powerLimit);
        writeLimitToFile (currentPP1dir + raplDirs_.window0dir_, raplDefaultCaps_.defaultConstrPP1_->timeWindow);
        writeLimitToFile (currentPP1dir + raplDirs_.isEnabledDir_, raplDefaultCaps_.defaultConstrPP1_->isEnabled);
    }
    for (auto& currentDRAMdir : raplDirs_.dramDirs_) {
        writeLimitToFile (currentDRAMdir + raplDirs_.pl0dir_, raplDefaultCaps_.defaultConstrDRAM_->powerLimit);
        writeLimitToFile (currentDRAMdir + raplDirs_.window0dir_, raplDefaultCaps_.defaultConstrDRAM_->timeWindow);
        writeLimitToFile (currentDRAMdir + raplDirs_.isEnabledDir_, raplDefaultCaps_.defaultConstrDRAM_->isEnabled);
    }
}

double IntelDevice::getPowerLimitInWatts() const
{
	return currentPowerLimitInWatts_;
}

void IntelDevice::setPowerLimitInMicroWatts(unsigned long limitInMicroW)
{
    Domain dom = PowerCapDomain::PKG; // TODO: the domain is hardcoded so that it can fit generic API for CPU and GPU
                                      //       It should be considered to drop support for PP0, PP1 domains and to
                                      //       have separate API for DRAM domain.
    auto&& numPkgs = totalPackages_; // packagesDirs_.size();
    auto singlePKGcap = limitInMicroW / numPkgs;
    switch (dom) {
        case PowerCapDomain::PKG :
            setLongTimeWindow(int(2*1e5)); // set to 200ms
            for (auto& curentPkgDir : raplDirs_.packagesDirs_) {
                writeLimitToFile(curentPkgDir + raplDirs_.pl0dir_, singlePKGcap);
                //TODO: rework below temporary solution
                //      move current cap to power interface class
                //      along with this whole method setPowerCap
                currentPowerLimitInWatts_ = (double)limitInMicroW / 1000000;
            }
            break;
        case PowerCapDomain::PP0 :
            for (auto& curentPP0dir : raplDirs_.pp0Dirs_) {
                writeLimitToFile(curentPP0dir + raplDirs_.pl0dir_, limitInMicroW);
                writeLimitToFile(curentPP0dir + raplDirs_.isEnabledDir_, 1);
            }
            break;
        case PowerCapDomain::PP1 :
            for (auto& curentPP1dir : raplDirs_.pp1Dirs_) {
                writeLimitToFile(curentPP1dir + raplDirs_.pl0dir_, limitInMicroW);
                writeLimitToFile(curentPP1dir + raplDirs_.isEnabledDir_, 1);
            }
            break;
        case PowerCapDomain::DRAM :
            for (auto& curentDRAMdir : raplDirs_.dramDirs_) {
                writeLimitToFile(curentDRAMdir + raplDirs_.pl0dir_, limitInMicroW);
                writeLimitToFile(curentDRAMdir + raplDirs_.isEnabledDir_, 1);
            }
            break;
        default :
            break;
    }
}

void IntelDevice::setLongTimeWindow(int longTimeWindow) {
    for (auto& curentPkgDir : raplDirs_.packagesDirs_) {
        writeLimitToFile (curentPkgDir + raplDirs_.window0dir_, longTimeWindow);
    }
}

void IntelDevice::initPerformanceCounters()
{
    pcm_ = pcm::PCM::getInstance();
    std::cerr << "\n Resetting PMU configuration" << std::endl;
    pcm_->resetPMU();
    pcm::PCM::ErrorCode status = pcm_->program();
    if (status != pcm::PCM::Success) {
        std::cerr << "Unsuccesfull CPU events programming - application can not be run properly\n Exiting...\n";
        // TODO: exception should be thrown
    }
}

double IntelDevice::getCurrentPowerInWatts(std::optional<Domain> domain) const // this method shall have the input parameter "deviceID" back
{
    Domain d = Domain::PKG;
    if (domain.has_value())
    {
        d = domain.value();
    }
    double result = 0.0;
    for (auto&& rapl : raplVec_)
    {
        result += rapl.getCurrentPower()[d];
    }
    return result;
}


void IntelDevice::reset()
{
    for (auto&& rapl : raplVec_)
    {
        rapl.reset();
    }
    std::vector<pcm::SocketCounterState> dummySocketStates_;

    pcm_->getAllCounterStates(sysBeforeState_, dummySocketStates_, beforeState_);
}

double IntelDevice::getNumInstructionsSinceReset() const
{
    pcm::SystemCounterState sysAfterState_;
    std::vector<pcm::CoreCounterState> afterState_;
    std::vector<pcm::SocketCounterState> dummySocketStates_;
    pcm_->getAllCounterStates(sysAfterState_, dummySocketStates_, afterState_);
	return (double)getInstructionsRetired(sysBeforeState_, sysAfterState_)/1000000;
}

unsigned long long int IntelDevice::getPerfCounter() const
{
    return (unsigned long long) this->getNumInstructionsSinceReset();
}


void IntelDevice::triggerPowerApiSample()
{
    for (auto&& rapl : raplVec_)
    {
        rapl.sample();
    }
}

void IntelDevice::checkIdlePowerConsumption()
{
    // TODO: pass below two values through config file
    int idleCheckTimeSeconds = 10;
    int msPause = 100;
    std::cout << "\nChecking idle average power consumption for " << idleCheckTimeSeconds << "s.\n";
    double energy = 0.0;
    reset();
    const auto start = std::chrono::high_resolution_clock::now();
    for (auto i = 0; i < idleCheckTimeSeconds * 1000; i += msPause)
    {
        if (!(i%1000)) std::cout << "." << std::flush;
        auto tmp = std::chrono::high_resolution_clock::now();
        usleep(msPause * 1000);
        triggerPowerApiSample();
        auto timeDeltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - tmp).count();
        energy += timeDeltaMs * getCurrentPowerInWatts() / 1000;
    }
    double totalTimeInSeconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "\r";
    idlePowerConsumption_ = energy / totalTimeInSeconds;
    std::cout << std::fixed << std::setprecision(3)
              << "\n[INFO] IntelDevice idle average power consumption for CPU PKG domain is " << idlePowerConsumption_ << " W\n";
}