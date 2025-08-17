#pragma once

#include <stdexcept>
#include <string>
#include <atomic>

class TimeoutException : public std::runtime_error {
public:
    explicit TimeoutException(const std::string& msg) : std::runtime_error(msg) {}
    
    
    inline static void checkStop(const std::atomic<bool>& stopFlag, std::string msg) {
        if (stopFlag) throw TimeoutException(msg);
    }
};
