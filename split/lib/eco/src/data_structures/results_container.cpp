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

#include "data_structures/results_container.hpp"

#include <cmath>

void ResultsContainer::storeOneResult(unsigned index, const FinalPowerAndPerfResult& oneRes) {
    if (index < vec_.size()) {
        vec_[index] = std::move(oneRes);
    } else {
        std::cerr << "[WARNING] Attempt to store unexpected result. Index out of range.\n";
    }
}

EnergyTimeResult ResultsContainer::getAverageResult() const {
    EnergyTimeResult res(0.0, 0.0, 0.0);
    for (auto&& r : vec_) {
        res += r.getEnergyAndTime();
    }
    return res / vec_.size();
}

EnergyTimeResult ResultsContainer::getStdDev() const {
    EnergyTimeResult res(0.0, 0.0, 0.0);
    auto&& av = getAverageResult();
    for (auto&& sample : vec_) {
        res += pow(av - sample.getEnergyAndTime(), 2);
    }
    return sqrt(res / vec_.size());
}

EnergyTimeResult ResultsContainer::getStdDevRel() const {
    auto&& res = getStdDev();
    auto&& av = getAverageResult();
    res.energy_ /= av.energy_;
    res.time_ /= av.time_;
    res.power_ /= av.power_;
    return res;
}
