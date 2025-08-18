#include <atomic>
#include <chrono>


class Timer{

public:
    static void runTimer(std::chrono::seconds timeout, std::atomic<bool>& stopFlag);
private:    
    static void launch(std::chrono::seconds timeout, std::atomic<bool>& stopFlag);
};