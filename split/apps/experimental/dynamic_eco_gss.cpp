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

#include "eco.hpp"
#include "data_structures/results_container.hpp"
#include "plot_builder.hpp"
#include "devices/intel_device.hpp"

#include <sstream>
#include <iostream>

static inline double delta(double def, double curr) {
    return 100 * (def - curr)/def;
}

template <class Stream>
Stream& operator<<(Stream& os,
                   const std::pair<ResultsContainer, ResultsContainer>& resPair) {
    auto&& ref = resPair.first;
    auto&& curr = resPair.second;
    auto&& refEandT = ref.getAverageResult();
    auto&& currEandT = curr.getAverageResult();
    auto&& currStddev = curr.getStdDev();
    auto&& currStddevRel = curr.getStdDevRel();
    os << std::fixed << std::setprecision(3) << currEandT.power_
        << std::setprecision(2)
        << " ± " << currStddev.power_
        << "\t" << currStddevRel.power_
        << std::setprecision(3) << "\t" << currEandT.energy_
        << std::setprecision(2)
        << " ± " << currStddev.energy_
        << "\t" << currStddevRel.energy_
        << std::setprecision(3) << "\t" << delta(refEandT.energy_, currEandT.energy_)
        << std::setprecision(1)
        << "\t" << currEandT.time_.totalTime_
        << std::setprecision(2)
        << " ± " << currStddev.time_.totalTime_
        << "\t" << currStddevRel.time_.totalTime_
        << std::setprecision(2) << "\t" << delta(refEandT.time_.totalTime_, currEandT.time_.totalTime_)
        << "\t" << currEandT.time_.waitTime_
        << "\t" << currEandT.time_.testTime_
        << std::setprecision(1) << "\t" << (currEandT.energy_ / 1000) * currEandT.time_.totalTime_
        << std::setprecision(2) << " ± "
        << std::sqrt(std::pow(currStddevRel.time_.totalTime_, 2) + std::pow(currStddevRel.energy_, 2));
    return os;
}

std::stringstream printResult(const ResultsContainer& ref, const ResultsContainer& curr, const double& k) {
    std::stringstream tmp;
    tmp << "\t\t" << std::make_pair(ref, curr) << "\t"
        << std::setprecision(3)
        << curr.getAverageResult().checkPlusMetric(ref.getAverageResult(), k)
        << "\n";
    return tmp;
}

int main (int argc, char *argv[]) {

    // temporary fix
    std::ofstream watchdog ("/proc/sys/kernel/nmi_watchdog", std::ios::out | std::ios::trunc);
	watchdog << "0";
	watchdog.close();


    Eco eco(std::make_shared<IntelDevice>());
    std::ofstream outResultFile (eco.getResultFileName(), std::ios::out | std::ios::trunc);

    BothStream bout(outResultFile);
    bout << "# ";
    for (int i=1; i<argc; i++) {
        bout << argv[i] << " ";
    }
    bout << "\n";

    std::initializer_list<double> kList = {eco.getK()};
    std::initializer_list<double> kListShort = {eco.getK()};
    const auto&& numIterations = eco.getNumIterations();
    std::stringstream tmp;
    tmp << "# Result is an average of " << numIterations << " runs.\n";
    tmp << "#_________\t\tAv.Power[W]"
        << "\t\t\t\tEnergy[J]"
        << "\t\t\t\tdE[%]"
        << "\ttime[s]"
        << "\t\t\t\tdT[%]"
        << "\twaitT[s]\ttestT[s]"
        << "\tExt[kJs]"
        << "\tEDS(k=" << eco.getK() << ")[-]\n";

    auto startTime = std::chrono::high_resolution_clock::now();
    ResultsContainer resultsDef(numIterations);
    for (int i = 0; i < numIterations; i++) {
        resultsDef.storeOneResult(i, eco.runAppWithSampling(argv));
    }
    tmp << "Default___" << printResult(resultsDef, resultsDef, *kList.begin()).str() << "\n\n";
    auto&& metricList = {TargetMetric::MIN_E,
                         TargetMetric::MIN_E_X_T,
                         TargetMetric::MIN_M_PLUS};
    const auto&& searchType = SearchType::GOLDEN_SECTION_SEARCH;
    for (auto&& metric : metricList) {
        auto&& kListLocal =
            ((metric == TargetMetric::MIN_M_PLUS) ? kList : kListShort);
        for (auto&& k : kListLocal) {
            eco.setCustomK(k);
            ResultsContainer results(numIterations);
            for (int i = 0; i < numIterations; i++) {
                // eco.idleSample(10); // separate test runs with 10 seconds break
                results.storeOneResult(i, eco.runAppWithSearch(argv, metric, searchType));
            }
            tmp << metric << printResult(resultsDef, results, k).str() << "\n\n";
        }
    }
    bout << tmp.str();

    auto totalTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - startTime);
    bout << "TotalTime: " << int(totalTime.count()) / 60 << "min " << int(totalTime.count()) % 60 << "sec\n";
    bout.flush();
    outResultFile.close();

    eco.plotPowerLog(std::nullopt);

    // Plot result file automatically
    std::string imgFileName = eco.getResultFileName();
    PlotBuilder p(imgFileName.replace(imgFileName.end() - 3,
                                      imgFileName.end(),
                                      "png"));
	p.setPlotTitle(eco.getDeviceName());
    std::cout << "Processing " << eco.getResultFileName() << " file...\n";
	p.plotTmpGSS(eco.getResultFileName());
    p.submitPlot();

	return 0;
}