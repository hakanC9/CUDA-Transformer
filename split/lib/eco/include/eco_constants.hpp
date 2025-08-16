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
#include <iomanip>
#include <map>
#include <memory>
#include <iostream>

enum class WatchdogStatus {
    ENABLED,
    DISABLED
};

enum class TargetMetric {
    MIN_E,
    MIN_E_X_T,
    MIN_M_PLUS
};

enum class SearchType {
    LINEAR_SEARCH,
    GOLDEN_SECTION_SEARCH
};

template <class Stream>
Stream& operator<<(Stream& os, const TargetMetric& m) {
    switch(m) {
        case TargetMetric::MIN_E :
            os << "Min_E_____";
            break;
        case TargetMetric::MIN_E_X_T :
            os << "Min_Ext___";
            break;
        case TargetMetric::MIN_M_PLUS :
            os << "Min_M_plus";
            break;
        default :
            os << "Undefined metric";
            break;
    }
    return os;
}

template <class Stream>
Stream& operator<<(Stream& os, const SearchType& st) {
    switch(st) {
        case SearchType::LINEAR_SEARCH :
            os << "Linear Search";
            break;
        case SearchType::GOLDEN_SECTION_SEARCH :
            os << "Golden Section Search";
            break;
        default :
            os << "Undefined search";
            break;
    }
    return os;
}

typedef enum PowerCapDomain {
    PKG  = 0,
    PP0  = 1,
    PP1  = 2,
    DRAM = 3
} Domain;

template <class Stream>
Stream& operator<<(Stream& os, const Domain& dom) {
    switch(dom) {
        case PowerCapDomain::PKG :
            os << "Package";
            break;
        case PowerCapDomain::PP0 :
            os << "PP0";
            break;
        case PowerCapDomain::PP1 :
            os << "PP1";
            break;
        case PowerCapDomain::DRAM :
            os << "DRAM";
            break;
        default :
            os << "unknown domain";
            break;
    }
    return os;
}

struct SubdomainInfo {
    SubdomainInfo(double pl, double tw, bool en) :
        powerLimit(pl), timeWindow(tw), isEnabled(en) {}
    ~SubdomainInfo() {}

    double powerLimit, timeWindow;
    bool isEnabled;

    template <class Stream>
    friend Stream& operator<<(Stream& os, const SubdomainInfo& sd)
    {
        os << std::fixed << std::setprecision(0)
        << "limit:\t\t" << sd.powerLimit << "\n"
        << "window:\t\t" << sd.timeWindow  << "\n"
        << "enabled:\t" << (sd.isEnabled ? 1 : 0) << "\n"
        << std::flush;
        return os;
    }
};

struct Constraints {
    Constraints(double lp, double sp, double lw, double sw) :
        longPower(lp), shortPower(sp), longWindow(lw), shortWindow(sw) {}
    ~Constraints() {}

    double longPower, shortPower, longWindow, shortWindow;

    template <class Stream>
    friend Stream& operator<<(Stream& os, const Constraints& c)
    {
        os << std::fixed << std::setprecision(0)
        << "long limit:\t\t" << c.longPower << "\n"
        << "long window:\t" << c.longWindow  << "\n"
        << "short limit:\t" << c.shortPower << "\n"
        << "short window:\t" << c.shortWindow << "\n"
        << std::flush;
        return os;
    }
};

using SubdomainInfoSP = std::shared_ptr<SubdomainInfo> ;
using ConstraintsSP = std::shared_ptr<Constraints> ;
using CrossDomainQuantity = std::map<Domain, double> ;
using EnergyCrossDomains = CrossDomainQuantity;
using PowerCrossDomains = CrossDomainQuantity;
