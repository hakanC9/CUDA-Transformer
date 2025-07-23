#pragma once

#include "Helper/AccuracyEvaluator.h"


class DefaultEvaluator : public AccuracyEvaluator{
public:

    DefaultEvaluator(const std::string& targetOutput);

    double getAccuracy() override;
    double getAccuracy(std::string transformedOutput) override;


private:

    const std::string& targetOutput;
};