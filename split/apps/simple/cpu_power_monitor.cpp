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

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <memory>

#include "plot_builder.hpp"
// Workaround: below two has to be included in such order to ensure no warnings
//             about macro redefinitions. Some of the macros in Rapl.hpp are already
//             defined in types.h included by cpucounters.h but not all of them.
#include <cpucounters.h>
#include "device_state.hpp"
#include "devices/intel_device.hpp"
//----------------------------------------------------------------------------------

int main(int argc, char *argv[]) {

	std::ofstream watchdog ("/proc/sys/kernel/nmi_watchdog", std::ios::out | std::ios::trunc);
	watchdog << "0";
	watchdog.close();

    pcm::PCM * m = pcm::PCM::getInstance();
	m->resetPMU();
	pcm::PCM::ErrorCode status = m->program(pcm::PCM::DEFAULT_EVENTS, nullptr);
	if(status != pcm::PCM::Success) {
		std::cout << "Unsuccesfull CPU events programming - application can not be run properly\n Exiting...\n";
		return -1;
	}

	pcm::SystemCounterState SysBeforeState, SysAfterState;
	std::vector<pcm::CoreCounterState> BeforeState, AfterState;
	std::vector<pcm::SocketCounterState> DummySocketStates;

    DeviceStateAccumulator ds(std::make_shared<IntelDevice>());
	int ms_pause = 100;       // sample every 100ms
	std::string outFileName = "./rapl.csv";
	std::ofstream outfile (outFileName, std::ios::out | std::ios::trunc);
	outfile << std::fixed << std::setprecision(3) << "curr.P\tPP0\t\tPP1\t\tDRAM\ttime\n";

	m->getAllCounterStates(SysBeforeState, DummySocketStates, BeforeState);

    int fd = open("redirected_GPC.txt", O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if (fd < 0) { perror("open"); abort(); }

    pid_t childProcId = fork();
    if (childProcId >= 0) { //fork successful
        if (childProcId == 0) { // child process

            if (dup2(fd, 1) < 0) { perror("dup2"); abort(); }
            close(fd);
			int exec_status = execvp(argv[1], argv+1);
			if (exec_status) {
                std::cerr << "execv failed with error " << errno
                          << " " << strerror(errno) << std::endl;
			}
		} else {              // parent process
			
			int status = 1;
			waitpid(childProcId, &status, WNOHANG);	
			while (status) {
				
				usleep(ms_pause * 1000);

				ds.sample();
				outfile << ds.getCurrentPower(Domain::PKG) << "\t"
				<< ds.getCurrentPower(Domain::PP0) << "\t"
				<< ds.getCurrentPower(Domain::PP1) << "\t"
				<< ds.getCurrentPower(Domain::DRAM) << "\t"
				<< ds.getTimeSinceReset() << std::endl;
				waitpid(childProcId, &status, WNOHANG);	
			}
			wait(&status);

			double totalTimeInSeconds = ds.getTimeSinceReset();

			std::cout << std::endl
				<< "\t PKG Total Energy:\t" << ds.getEnergySinceReset() << " J" << std::endl
				// << "\t PP0 Total Energy:\t" << ds.getEnergySinceReset(Domain::PP0) << " J" << std::endl
				// << "\t PP1 Total Energy:\t" << ds.getEnergySinceReset(Domain::PP1) << " J" << std::endl
				// << "\tDRAM Total Energy:\t" << ds.getEnergySinceReset(Domain::DRAM) << " J" << std::endl
				<< "\t PKG Average Power:\t" << ds.getEnergySinceReset() / totalTimeInSeconds << " W" << std::endl
				// << "\t PP0 Average Power:\t" << ds.getEnergySinceReset(Domain::PP0) / totalTimeInSeconds << " W" << std::endl
				// << "\t PP1 Average Power:\t" << ds.getEnergySinceReset(Domain::PP1) / totalTimeInSeconds << " W" << std::endl
				// << "\tDRAM Average Power:\t" << ds.getEnergySinceReset(Domain::DRAM) / totalTimeInSeconds << " W" << std::endl
				<< "\tTotal time:\t\t" << totalTimeInSeconds << " sec" << std::endl;
		}
	} else {
		std::cerr << "fork failed" << std::endl;
		return 1;
	}
	m->getAllCounterStates(SysAfterState, DummySocketStates, AfterState);
	pcm::uint64 cycles = getCycles(SysBeforeState, SysAfterState);
    pcm::uint64 instr = getInstructionsRetired(SysBeforeState, SysAfterState);
    std::cout << "\ttotal cycles:\t\t" << (double)cycles/1000000 << " M\n"
			  << "\ttotal instr.:\t\t" << (double)instr/1000000 <<" M\n";
	outfile.close();

	PlotBuilder p("hehe1.png");
	Series currentPower (outFileName, 5, 1, "power [W]");
	p.plot({currentPower});
	return 0;
}
