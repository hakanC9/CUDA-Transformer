#include "Helper/Timer.h"

#include <functional>
#include <iostream>
#include <thread>


void Timer::runTimer(std::chrono::seconds timeout, std::atomic<bool>& stopFlag){
    std::thread timerThread(launch, timeout, std::ref(stopFlag));
    timerThread.detach();
}



void Timer::launch(std::chrono::seconds timeout, std::atomic<bool>& stopFlag){
    std::cout << "\nTimeout started\n";
    std::this_thread::sleep_for(timeout);
    stopFlag = true;
}