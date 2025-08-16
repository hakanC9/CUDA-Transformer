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

#include <iostream>

struct TimeResult {
    TimeResult() {}
    TimeResult(double t) : totalTime_(t) {}
    TimeResult(double t, double wt, double tt) :
        totalTime_(t), waitTime_(wt), testTime_(tt) {}
    ~TimeResult() {}
    double getExecTime() const { return totalTime_ - (waitTime_ + testTime_); }
    double getRelativeWaitTime() const { return waitTime_ / totalTime_; }
    double getRelativeTestTime() const { return testTime_ / totalTime_; }
    friend TimeResult& operator+=(TimeResult& , const TimeResult&);
    friend TimeResult operator-(const TimeResult& , const TimeResult&);
    friend TimeResult pow(const TimeResult&, const double&);
    friend TimeResult sqrt(const TimeResult&);
    template <class T>
    friend TimeResult& operator/(TimeResult& left, const T& right) {
        left.totalTime_ /= right;
        left.waitTime_ /= right;
        left.testTime_ /= right;
        return left;
    }
    template <class T>
    friend TimeResult& operator/=(TimeResult& left, const T& right) {
        left.totalTime_ /= right;
        left.waitTime_ /= right;
        left.testTime_ /= right;
        return left;
    }
    TimeResult& operator/=(const TimeResult& right) {
        totalTime_ /= right.totalTime_;
        waitTime_ /= right.waitTime_;
        testTime_ /= right.testTime_;
        return *this;
    }
    template <class T>
    friend TimeResult& operator*=(TimeResult& left, const T& right) {
        left.totalTime_ *= right;
        left.waitTime_ *= right;
        left.testTime_ *= right;
        return left;
    }
    double totalTime_ {0.0};
    double waitTime_ {0.0};
    double testTime_ {0.0};
};

struct EnergyTimeResult {
    EnergyTimeResult() = default;
    EnergyTimeResult(double& e, double& t, double& p) : energy_(e), time_(t) , power_(p) {}
    EnergyTimeResult(double& e, TimeResult& t, double& p) : energy_(e), time_(t) , power_(p) {}
    EnergyTimeResult(const double& e, const double& t, const double& p) :
        energy_(e), time_(t), power_(p) {}
    EnergyTimeResult(const double& e, const TimeResult& t, const double& p) :
        energy_(e), time_(t), power_(p) {}
    ~EnergyTimeResult() = default;

    static constexpr double ALMOST_ZERO {0.00000000000000001};
    double energy_ {ALMOST_ZERO};
    TimeResult time_;
    double power_ {ALMOST_ZERO};

    friend EnergyTimeResult& operator+=(EnergyTimeResult& , const EnergyTimeResult&);
    friend EnergyTimeResult operator-(const EnergyTimeResult& , const EnergyTimeResult&);
    friend EnergyTimeResult pow(const EnergyTimeResult&, const double&);
    friend EnergyTimeResult sqrt(const EnergyTimeResult&);
    template <class T>
    friend EnergyTimeResult& operator/(EnergyTimeResult& left, const T& right) {
        left.energy_ /= right;
        left.time_ /= right;
        left.power_ /= right;
        return left;
    }
    template <class T>
    friend EnergyTimeResult& operator*=(EnergyTimeResult& left, const T& right) {
        left.energy_ *= right;
        left.time_ *= right;
        left.power_ *= right;
        return left;
    }
    double checkPlusMetric(const EnergyTimeResult& ref, double k) const;
};

struct FinalPowerAndPerfResult {
    FinalPowerAndPerfResult();
    FinalPowerAndPerfResult(double, double, double,
                            double, double, double,
                            TimeResult, double, double,
                            double = 0.0, double = 0.0, double = 0.0,
                            double = 0.0, double = 1.0);
    ~FinalPowerAndPerfResult() {}

    friend std::ostream& operator<<(std::ostream&, const FinalPowerAndPerfResult&);
    friend FinalPowerAndPerfResult& operator+=(FinalPowerAndPerfResult& , const FinalPowerAndPerfResult&);
    friend FinalPowerAndPerfResult& operator/=(FinalPowerAndPerfResult& , const unsigned&);

    double powercap {0.0}, energy {0.0}, pkgPower {0.0}, pp0power {0.0}, pp1power {0.0},
           dramPower {0.0}, inst {0.0}, cycl {0.0}, deltaE {0.0}, deltaT {0.0},
           relativeDeltaE {0.0}, relativeDeltaT {0.0}, enerTimeProd {0.0}, mPlus{1.0};
    TimeResult time_;
    EnergyTimeResult getEnergyAndTime() const { return EnergyTimeResult(energy, time_, pkgPower); }
    double getInstrPerSec() const { return inst / time_.totalTime_; }
    double getEnergyPerInstr() const { return energy / inst; }
};

class CompareFinalResultsForMinE {
public:
    bool operator() (FinalPowerAndPerfResult& left, FinalPowerAndPerfResult& right) const {
        return left.energy < right.energy;
    }
};

class CompareFinalResultsForMinEt {
public:
    bool operator() (FinalPowerAndPerfResult& left, FinalPowerAndPerfResult& right) const {
        return left.enerTimeProd < right.enerTimeProd;
    }
};

class CompareFinalResultsForMplus {
public:
    bool operator() (FinalPowerAndPerfResult& left, FinalPowerAndPerfResult& right) const {
        return left.mPlus < right.mPlus;
    }
};