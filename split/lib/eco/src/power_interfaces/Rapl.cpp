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

#include "power_interface/Rapl.hpp"


AvailableRaplPowerDomains::AvailableRaplPowerDomains (bool p0, bool p1, bool d, bool ps, bool du) :
    pp0_(p0), pp1_(p1), dram_(d), psys_(ps), fixedDramUnits_(du)
{
	availableDomainsSet_.insert(PowerCapDomain::PKG);
	if (pp0_) availableDomainsSet_.insert(PowerCapDomain::PP0);
	if (pp1_) availableDomainsSet_.insert(PowerCapDomain::PP1);
	if (dram_) availableDomainsSet_.insert(PowerCapDomain::DRAM);
}


void RaplStateSequence::rotateStates() {
	RaplState tmp = previous_;
    previous_ = current_;
    current_ = next_;
    next_ = tmp;
}

void RaplStateSequence::storeNextState(RaplState& rs) {
	next_ = rs;
}

void RaplStateSequence::reset() {
	RaplState emptyState;
	previous_ = current_ = next_ = emptyState;
}

uint64_t RaplStateSequence::energyDelta(uint64_t before, uint64_t after) const
{
    if (before > after) {
		// Check for overflow
        constexpr uint64_t MAX_INT = ~((uint32_t) 0);
        return uint64_t(after + (MAX_INT - before));
    }
	else {
        return uint64_t (after - before);
	}
}

double RaplStateSequence::timeDelta(const TimePoint& begin, const TimePoint& end) const
{
    return std::chrono::duration<double>(end-begin).count();
}

double RaplStateSequence::getTotalTime(TimePoint startTime) const
{
	return timeDelta(startTime, current_.timeSec_);
}

RaplState RaplStateSequence::getCurrentEnergyIncrement() const
{
    return RaplState(energyDelta(current_.pkg_, next_.pkg_),
	                 energyDelta(current_.pp0_, next_.pp0_),
					 energyDelta(current_.pp1_, next_.pp1_),
					 energyDelta(current_.dram_, next_.dram_),
					 std::chrono::high_resolution_clock::now()); // probably does not matter
}

RaplState RaplStateSequence::getPreviousEnergyIncrement() const
{
    return RaplState(energyDelta(previous_.pkg_, current_.pkg_),
	                 energyDelta(previous_.pp0_, current_.pp0_),
					 energyDelta(previous_.pp1_, current_.pp1_),
					 energyDelta(previous_.dram_, current_.dram_),
					 std::chrono::high_resolution_clock::now()); // probably does not matter
}

double RaplStateSequence::getCurrentTimeIncrement() const
{
	return timeDelta(current_.timeSec_, next_.timeSec_);
}

double RaplStateSequence::getPreviousTimeIncrement() const
{
	return timeDelta(previous_.timeSec_, current_.timeSec_);
}

RaplState& operator+=(RaplState& left, const RaplState& right) {
    left.pkg_ += right.pkg_;
    left.pp0_ += right.pp0_;
    left.pp1_ += right.pp1_;
    left.dram_ += right.dram_;
	return left;
}

void Rapl::initializeRaplForPowerReadingAndCapping()
{
    MSR msr(cpuCore_);

    power_units  = msr.getUnits(Quantity::Power);
    energy_units = msr.getUnits(Quantity::Energy);
    time_units   = msr.getUnits(Quantity::Time);
    if (availableDomains_.fixedDramUnits_) {
        dram_energy_units = msr.getFixedDramUnitsValue();
        std::cout << "DRAM: Using " << dram_energy_units << " J instead of " << energy_units <<" J.\n";
    }
    else {
        dram_energy_units = energy_units;
    }

    auto powerInfo = msr.getPowerInfoForPKG();
    printf("\t\tPackage thermal spec: %.3fW\n", powerInfo.thermalDesignPower);
    printf("\t\tPackage minimum power: %.3fW\n", powerInfo.minPower);
    printf("\t\tPackage maximum power: %.3fW\n", powerInfo.maxPower);
    printf("\t\tPackage maximum time window: %.6fs\n", powerInfo.maxTimeWindow);

    if (msr.checkLockedByBIOS())
    {
        std::cout << "[INFO] When locked by BIOS it is possible to read power from RAPL but you cannot limit the power." << std::endl;
		return;
	}
    msr.enableClamping(Domain::PKG);
    msr.enablePowerCapping(Domain::PKG);
    msr.disableClamping(Domain::PP0);
    msr.disableClamping(Domain::PP0);

}

Rapl::Rapl(int core, AvailableRaplPowerDomains avDom) :
    cpuCore_(core), availableDomains_(avDom)
{
    initializeRaplForPowerReadingAndCapping();
    reset();
}

void Rapl::reset() {
    rss_.reset();
    // sample twice to fill current and previous
    sample();
    sample();
	RaplState emptyState;
	totalResultSinceLastReset_ = emptyState;
}

void Rapl::sample() {
    MSR msr(cpuCore_);
	RaplState nextState(
		msr.getEnergyStatus(Domain::PKG),
		msr.getEnergyStatus(Domain::PP0),
		availableDomains_.pp1_ ? msr.getEnergyStatus(Domain::PP1) : 0,
		availableDomains_.dram_ ? msr.getEnergyStatus(Domain::DRAM) : 0,
		std::chrono::high_resolution_clock::now());

    rss_.storeNextState(nextState);

    totalResultSinceLastReset_ += rss_.getCurrentEnergyIncrement();
	rss_.rotateStates();
}

double Rapl::calculate_power(uint64_t energyIncrement, double time_delta, double units) const
{
    if (time_delta == 0.0f || time_delta == -0.0f) { return 0.0; }
    double energy = units * ((double) energyIncrement);
    return energy / time_delta;
}

double Rapl::pkg_current_power() const
{
    double t = rss_.getPreviousTimeIncrement();
    return calculate_power(rss_.getPreviousEnergyIncrement().pkg_, t, energy_units);
}

double Rapl::pp0_current_power() const
{
    double t = rss_.getPreviousTimeIncrement();
    return calculate_power(rss_.getPreviousEnergyIncrement().pp0_, t, energy_units);
}

double Rapl::pp1_current_power() const
{
    double t = rss_.getPreviousTimeIncrement();
    return calculate_power(rss_.getPreviousEnergyIncrement().pp1_, t, energy_units);
}

double Rapl::dram_current_power() const
{
    double t = rss_.getPreviousTimeIncrement();
    return calculate_power(rss_.getPreviousEnergyIncrement().dram_, t, dram_energy_units);
}

double Rapl::pkg_average_power() const
{
    return pkg_total_energy() / get_total_time();
}

double Rapl::pp0_average_power() const
{
    return pp0_total_energy() / get_total_time();
}

double Rapl::pp1_average_power() const
{
    return pp1_total_energy() / get_total_time();
}

double Rapl::dram_average_power() const
{
    return dram_total_energy() / get_total_time();
}

double Rapl::pkg_total_energy() const
{
    return energy_units * ((double) totalResultSinceLastReset_.pkg_);
}

double Rapl::pp0_total_energy() const
{
    return energy_units * ((double) totalResultSinceLastReset_.pp0_);
}

double Rapl::pp1_total_energy() const
{
    return energy_units * ((double) totalResultSinceLastReset_.pp1_);
}

double Rapl::dram_total_energy() const
{
    return dram_energy_units * ((double) totalResultSinceLastReset_.dram_);
}

double Rapl::get_total_time() const
{
    return rss_.getTotalTime(totalResultSinceLastReset_.timeSec_);
}

double Rapl::current_time() {
    return rss_.getTotalTime(totalResultSinceLastReset_.timeSec_);
}

double Rapl::pkg_max_power() const
{
    auto&& pkgPowerInfo = MSR(cpuCore_).getPowerInfoForPKG();
    auto&& maxPower = pkgPowerInfo.maxPower;
    return maxPower ? maxPower : pkgPowerInfo.thermalDesignPower;
}

EnergyCrossDomains Rapl::getTotalEnergy() const
{
    EnergyCrossDomains result;
    result[Domain::PKG] = pkg_total_energy();
    result[Domain::PP0] = pp0_total_energy();
    result[Domain::PP1] = pp1_total_energy();
    result[Domain::DRAM] = dram_total_energy();
    return result;
}

PowerCrossDomains Rapl::getAveragePower() const
{
    EnergyCrossDomains result;
    result[Domain::PKG] = pkg_average_power();
    result[Domain::PP0] = pp0_average_power();
    result[Domain::PP1] = pp1_average_power();
    result[Domain::DRAM] = dram_average_power();
    return result;
}

PowerCrossDomains Rapl::getCurrentPower() const
{
    EnergyCrossDomains result;
    result[Domain::PKG] = pkg_current_power();
    result[Domain::PP0] = pp0_current_power();
    result[Domain::PP1] = pp1_current_power();
    result[Domain::DRAM] = dram_current_power();
    return result;
}