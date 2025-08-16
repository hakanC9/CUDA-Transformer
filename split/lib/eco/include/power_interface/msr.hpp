/*
   Copyright 2022, Adam Krzywaniak.

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

#include "eco_constants.hpp"

static constexpr int UNDEFINED_FD {-1};

enum class Quantity {
    Energy,
    Power,
    Time
};
struct PowerInfo {
    double thermalDesignPower;
    double minPower;
    double maxPower;
    double maxTimeWindow;
};

class MSR {
public:
    MSR() = delete;
    MSR(int core);
    ~MSR();
    uint64_t getEnergyStatus(Domain domain = Domain::PKG);
    double getUnits(Quantity q);
    double getFixedDramUnitsValue(); // some server CPUs use different Power Unit for DRAM
    PowerInfo getPowerInfoForPKG();
    void enableClamping(Domain domain = Domain::PKG);
    void enablePowerCapping(Domain domain = Domain::PKG);
    void disableClamping(Domain domain = Domain::PKG);
    void disablePowerCapping(Domain domain = Domain::PKG);
    bool checkLockedByBIOS();

private:
    int fileDescriptor_ {UNDEFINED_FD};
	void openMSR(int core);
	void writeMSR(int offset, uint64_t value);
	uint64_t readMSR(int offset);
    int getOffsetForPowerLimit(Domain domain = Domain::PKG);
};
