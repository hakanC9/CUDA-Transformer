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

#include <fstream>
#include <iostream>
#include "params_config.hpp"
#include <yaml-cpp/yaml.h>

ParamsConfig::ParamsConfig()
{
    loadConfig();
    // TODO: might be printed on request and silent by default
    printConfigExplained();
}

void ParamsConfig::printConfigExplained()
{
    std::cout << "\tCPU RAPL sampling time is " << msPause_ << "ms.\n";
    std::cout << "\tPowercaps step for Linear Search is "
            << percentStep_ << "%\n";
    std::cout << "\tCPU idle power consumption check time set to "
            << idleCheckTime_ << "s\n";
    std::cout << "\tEach experiment stored in result.csv is an average of "
            << numIterations_ << " test runs.\n";
    std::cout << "\tEnergy profiling for PKG domain will break after "
            << perfDropStopCondition_
            << "% drop of performance for tested power limit.\n";
    // below one is probably deprecated
    std::cout << "\tEnergy/Power sampling "
            << (powerSampleOn_ ? "on." : "off") << ".\n";
    std::cout << "\tTuning phase will be executed with "
            << (double)msTestPhasePeriod_/1000 << "s period for each power cap.\n";
    std::cout << "\tPower caps range is "
            << (reducedPowerCapRange_ ? "" : "not") << "reduced.\n";
    std::cout << "\tLogging current power to power_log.csv "
            << (isPowerLogOn_ ? "ENABLED" : "DISABLED") << ".\n";
    std::cout << "\tTuning phase will be delayed by "
            << optimizationDelay_ << " seconds.\n";
    std::cout << "\tTuning phase will be repeated after "
            << repeatTuningPeriodInSec_ << " seconds.\n";
    std::cout << "\tDEPO will DO "
            << (doWaitPhase_ ? "" : "NOT") << " wait for steady power consumption profile basing on SMA filtered power reading.\n";
    }


void ParamsConfig::loadConfig()
{
    YAML::Node config = YAML::LoadFile("config.yaml");

    msPause_ = config["msPause"].as<int>();
    percentStep_ = config["percentStep"].as<int>();
    idleCheckTime_ = config["idleCheckTime"].as<int>();
    numIterations_ = config["numIterations"].as<int>();
    perfDropStopCondition_ = config["perfDropStopCondition"].as<int>();
    powerSampleOn_ = config["powerSampleOn"].as<int>();
    targetMetric_ = config["targetMetric"].as<int>();
    msTestPhasePeriod_ = config["msTestPhasePeriod"].as<int>();
    reducedPowerCapRange_ = config["reducedPowerCapRange"].as<int>();
    isPowerLogOn_ = config["powerLog"].as<int>();
    optimizationDelay_ = config["optimizationDelay"].as<int>();
    k_ = config["k"].as<double>();
    repeatTuningPeriodInSec_ = config["repeatTuningPeriodInSec"].as<int>();
    doWaitPhase_ = config["doWaitPhase"].as<int>();
    referenceRunMultiplier_ = config["referenceRunMultiplier"].as<int>();
    usTestPhasePeriod_ = msTestPhasePeriod_ * 1000;
    // return cfg;
}