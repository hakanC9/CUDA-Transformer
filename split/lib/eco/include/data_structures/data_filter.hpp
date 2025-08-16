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

#include <vector>

class DataFilter {
public:
    DataFilter() = delete;
    DataFilter(int size) :
        filterSize_(size) {}
    double getSMA() const;
    double getRelativeError();
    double getCleanedRelativeError() const;

    void storeDataPoint(double dataPoint);
private:
    void shiftActiveIndex();
    double getSum() const;

    std::vector<double> data_;
    unsigned filterSize_;
    unsigned activeIndex_ {0};
};