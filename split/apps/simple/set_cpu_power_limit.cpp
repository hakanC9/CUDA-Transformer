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
#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#define RAPL_BASE_DIRECTORY_0 "/sys/class/powercap/intel-rapl/intel-rapl:0/"
#define RAPL_BASE_DIRECTORY_1 "/sys/class/powercap/intel-rapl/intel-rapl:1/"
#define LONG_LIMIT "constraint_0_power_limit_uw"
#define SHORT_LIMIT "constraint_1_power_limit_uw"
#define LONG_WINDOW "constraint_0_time_window_us"
#define SHORT_WINDOW "constraint_1_time_window_us"


void writeLimitToFile (std::string fileName, int limit){

    std::ofstream outfile (fileName.c_str(), std::ios::out | std::ios::trunc);
    if (outfile.is_open()){
        outfile << limit;
    } else {
        std::cerr << "cannot write the limit to file\n";
        std::cerr << "file not open\n";
    }
    outfile.close();
}

int readLimitFromFile (std::string fileName){

    std::ifstream limitFile (fileName.c_str());
    std::string line;
    int limit = -1;
    if (limitFile.is_open()){
        while ( getline (limitFile, line) ){
            limit = atoi(line.c_str());
        }
        limitFile.close();
    } else {
        std::cerr << "cannot read the limit file\n";
        std::cerr << "file not open\n";
    }
    return limit;
}

int main (int argc, char *argv[]) {

    if(argc < 3){
        printf("Usage: ./SetPowerLimit longPowerLimit[W] shortPowerLimit[W] longTimeWindow[s]\n");
        exit(EXIT_FAILURE);
    }
    for (std::string raplDir : {RAPL_BASE_DIRECTORY_0, RAPL_BASE_DIRECTORY_1}) {
        std::string pl0dir = raplDir + LONG_LIMIT;
        std::string pl1dir = raplDir + SHORT_LIMIT;
        std::string tw0dir = raplDir + LONG_WINDOW;

        int powerLimit0 = atoi(argv[1])*1000000;
        int powerLimit1 = atoi(argv[2])*1000000;
        int inputTime = atoi(argv[3]);
        int timeWindow0 = inputTime*1000000;

        writeLimitToFile (pl0dir, powerLimit0);
        if (readLimitFromFile(pl0dir) != powerLimit0) {
            std::cerr << "Limit was not overwritten succesfully.\n"
                        << "HINT: Check dmesg if it is not locked by BIOS.\n";
        }
        writeLimitToFile (pl1dir, powerLimit1);
        if (readLimitFromFile(pl1dir) != powerLimit1) {
            std::cerr << "Limit was not overwritten succesfully.\n"
                        << "HINT: Check dmesg if it is not locked by BIOS.\n";
        }

    // The time window length has certain values that can be set.
    // In the simplest case for 1s (1000000 us) the value that is writen to the time window
    // is smaller by 576 us. The same for multiplications of 1s but only up to 8s.
    // Above 8s the granularity of possible values is smaller but still keeps the missing 576 us.
    // For numbers higher than 8s the value of time window is set to the last possible value smaller
    // than the value that we try to set.
    // The possible values are discrete and they were found in empirical way.
    // The possible values are:
    // 1, 2, 3, 4, 5, 6, 7, 8,    [+1s]
    // 10, 12, 14, 16,            [+2s]
    // 20, 24, 28, 32,            [+4s]
    // 40, 48, 56, 64,            [+8s]
    // 80, 96, 112, 128          [+16s]
    //...

        writeLimitToFile (tw0dir, timeWindow0);
    //TODO: Handle in below validation the possible values described above.
        if (readLimitFromFile(tw0dir) != (timeWindow0 - inputTime * 576)) {
            std::cerr << "Limit was not overwritten succesfully.\n"
                        << "HINT: Check dmesg if it is not locked by BIOS.\n";
        }
    }
    return 0;
}
