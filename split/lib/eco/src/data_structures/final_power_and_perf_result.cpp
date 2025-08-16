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

#include "data_structures/final_power_and_perf_result.hpp"

#include <cmath>

FinalPowerAndPerfResult::FinalPowerAndPerfResult() :
    powercap(0.0), energy(0.0), pkgPower(0.0), pp0power(0.0), pp1power(0.0), dramPower(0.0),
    time_(0.0), deltaE(0.0), deltaT(0.0), relativeDeltaE(0.0), relativeDeltaT(0.0), mPlus(0.0) {}

FinalPowerAndPerfResult::FinalPowerAndPerfResult(double cap, double e, double pkgp,
                                                 double p0p, double p1p, double drp,
                                                 TimeResult t, double ins, double cyc,
                                                 double de, double dt, double rde,
                                                 double rdt, double mpl) :
    powercap(cap), energy(e), pkgPower(pkgp), pp0power(p0p), pp1power(p1p), dramPower(drp),
    time_(t), inst(ins), cycl(cyc), deltaE(de), deltaT(dt), relativeDeltaE(rde),
    relativeDeltaT(rdt), mPlus(mpl)
    {
        enerTimeProd = energy * time_.totalTime_;
    }

std::ostream& operator<<(std::ostream& os, const FinalPowerAndPerfResult& res) {
    if (res.powercap < 0.0) {
        os << "refer.\t";
    } else {
        os << res.powercap << "\t";
    }
    os  << res.energy << "\t"
        << res.pkgPower << "\t"
        << res.time_.totalTime_ << "\t"
        << res.enerTimeProd << "\t"
        << res.deltaE << "\t"
        << res.deltaT << "\t"
        << res.relativeDeltaE << "\t"
        << res.relativeDeltaT << "\t"
        << res.inst << " "
        << res.inst/res.time_.totalTime_ << " "
        << (res.cycl/res.time_.totalTime_)/res.pkgPower << "\t"
        << ((res.cycl/res.time_.totalTime_)*(res.cycl/res.time_.totalTime_))/res.pkgPower << "\t"
        << res.mPlus;
    return os;
}

FinalPowerAndPerfResult& operator+=(FinalPowerAndPerfResult& left, const FinalPowerAndPerfResult& right) {
    left.powercap += right.powercap;
    left.energy += right.energy;
    left.pkgPower += right.pkgPower;
    left.pp0power += right.pp0power;
    left.pp1power += right.pp1power;
    left.dramPower += right.dramPower;
    left.time_ += right.time_;
    left.inst += right.inst;
    left.cycl += right.cycl;
    left.deltaE += right.deltaE;
    left.deltaT += right.deltaT;
    left.relativeDeltaE += right.relativeDeltaE;
    left.relativeDeltaT += right.relativeDeltaT;
    left.mPlus += right.mPlus;

    left.enerTimeProd = left.energy * left.time_.getExecTime();
    return left;
}

FinalPowerAndPerfResult& operator/=(FinalPowerAndPerfResult& left, const unsigned& right) {
    if (right == 0) {
        std::cerr << "[WARNING] Division by 0 using operator /=.\n";
        return left;
    }
    left.powercap /= right;
    left.energy /= right;
    left.pkgPower /= right;
    left.pp0power /= right;
    left.pp1power /= right;
    left.dramPower /= right;
    left.time_ /= right;
    left.inst /= right;
    left.cycl /= right;
    left.deltaE /= right;
    left.deltaT /= right;
    left.relativeDeltaE /= right;
    left.relativeDeltaT /= right;
    left.mPlus /= right;

    left.enerTimeProd = left.energy * left.time_.getExecTime();

    return left;
}

TimeResult& operator+=(TimeResult& left, const TimeResult& right) {
    left.totalTime_ += right.totalTime_;
    left.waitTime_ += right.waitTime_;
    left.testTime_ += right.testTime_;
    return left;
}

TimeResult operator-(const TimeResult& left, const TimeResult& right) {
    TimeResult res;
    res.totalTime_ = left.totalTime_ - right.totalTime_;
    res.waitTime_ = left.waitTime_ - right.waitTime_;
    res.testTime_ = left.testTime_ - right.testTime_;
    return res;
}

TimeResult pow(const TimeResult& base, const double& exponent) {
    TimeResult res;
    res.totalTime_ = std::pow(base.totalTime_, exponent);
    res.waitTime_ = std::pow(base.waitTime_, exponent);
    res.testTime_ = std::pow(base.testTime_, exponent);
    return res;
}

TimeResult sqrt(const TimeResult& base) {
    TimeResult res;
    res.totalTime_ = std::sqrt(base.totalTime_);
    res.waitTime_ = std::sqrt(base.waitTime_);
    res.testTime_ = std::sqrt(base.testTime_);
    return res;
}

EnergyTimeResult& operator+=(EnergyTimeResult& left, const EnergyTimeResult& right) {
    left.energy_ += right.energy_;
    left.time_ += right.time_;
    left.power_ += right.power_;
    return left;
}

EnergyTimeResult operator-(const EnergyTimeResult& left, const EnergyTimeResult& right) {
    EnergyTimeResult res;
    res.energy_ = left.energy_ - right.energy_;
    res.time_ = left.time_ - right.time_;
    res.power_ = left.power_ - right.power_;
    return res;
}

EnergyTimeResult pow(const EnergyTimeResult& base, const double& exponent) {
    EnergyTimeResult res;
    res.energy_ = std::pow(base.energy_, exponent);
    res.time_ = pow(base.time_, exponent);
    res.power_ = std::pow(base.power_, exponent);
    return res;
}

EnergyTimeResult sqrt(const EnergyTimeResult& base) {
    EnergyTimeResult res;
    res.energy_ = std::sqrt(base.energy_);
    res.time_ = sqrt(base.time_);
    res.power_ = std::sqrt(base.power_);
    return res;
}

double EnergyTimeResult::checkPlusMetric(const EnergyTimeResult& ref, double k) const {
    double alfa = (k - 1) / (k * ref.energy_);
    double beta = 1 / (k * ref.time_.totalTime_);
    return (alfa * energy_) + (beta * time_.totalTime_);
}