#pragma once

#include <string>

class AccuracyEvaluator{

public:
    virtual double getAccuracy() = 0;
    virtual double getAccuracy(const std::string output) = 0;  
    ~AccuracyEvaluator(){};
};