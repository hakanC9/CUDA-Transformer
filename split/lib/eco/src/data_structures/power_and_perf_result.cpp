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

#include "data_structures/power_and_perf_result.hpp"

std::ostream& operator<<(std::ostream& os, const PowAndPerfResult& res) {
    if (res.appliedPowerCapInWatts_ < 0.0) {
        os << "refer.\t";
    } else {
        os << res.appliedPowerCapInWatts_ << "\t";
    }
    os << res.energyInJoules_ << "\t"
       << res.averageCorePowerInWatts_ << "\t"
       << res.filteredPowerOfLimitedDomainInWatts_ << "\t"
       << res.instructionsCount_ << "\t"
       << res.getInstrPerSecond() << "\t"
       << res.getInstrPerJoule() << " \t\t"
       << res.getEnergyTimeProd();
    return os;
}

bool PowAndPerfResult::isRightBetter(PowAndPerfResult& right, TargetMetric mode) {
    if (mode == TargetMetric::MIN_E) {
        return this->getEnergyPerInstr() > right.getEnergyPerInstr();
    } else if (mode == TargetMetric::MIN_E_X_T) {
        return this->getEnergyTimeProd() < right.getEnergyTimeProd();
    } else if (mode == TargetMetric::MIN_M_PLUS) {
        // this is dirty hack as actually Metric Plus is looking for minimum
        // not maximum of this metric.
        return this->myPlusMetric_ > right.myPlusMetric_;
    } else {
        // exception should be thrown as it should never happen to compare with unexpected mode
        return false;
    }
}

double PowAndPerfResult::checkPlusMetric(PowAndPerfResult ref, double k) {
    myPlusMetric_ = (1.0/k) * (ref.getInstrPerSecond()/getInstrPerSecond()) *
                    ((k-1.0) * (averageCorePowerInWatts_ / ref.averageCorePowerInWatts_) + 1.0);
    return myPlusMetric_;
}
