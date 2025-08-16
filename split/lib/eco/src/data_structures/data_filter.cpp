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

#include "data_structures/data_filter.hpp"

#include <algorithm>

double DataFilter::getSum() const
{
    double acc = 0.0;
    for (auto&& dataPoint : data_) {
        acc += dataPoint;
    }
    return acc;
}

double DataFilter::getSMA() const
{
    return getSum() / data_.size();
}

void DataFilter::storeDataPoint(double dataPoint) {
    if (data_.size() == filterSize_) {
        data_[activeIndex_] = dataPoint;
        shiftActiveIndex();
    } else {
        data_.push_back(dataPoint);
    }
}

void DataFilter::shiftActiveIndex() {
    auto maxIndex = filterSize_ - 1;
    if (activeIndex_ + 1 > maxIndex) {
        activeIndex_ = 0; // reset active index
    } else {
        activeIndex_++; // move active index
    }
}

double DataFilter::getCleanedRelativeError() const
{
    if (data_.size() >1)
    {
        const auto minmax = std::minmax_element(data_.begin(), data_.end());
        auto min = *minmax.first;
        auto max = *minmax.second;
        auto cleanedSMA = (getSum() - (min + max)) / (data_.size() - 2);
        return (max - min) / cleanedSMA;
    }
    else
    {
        return 1.00;
    }
}

double DataFilter::getRelativeError() {
    const auto minmax = std::minmax_element(data_.begin(), data_.end());
    return (*minmax.second - *minmax.first) / getSMA();
}