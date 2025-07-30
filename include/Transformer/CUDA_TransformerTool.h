#pragma once

#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "Helper/ConfigParser.h"

#include <memory>
#include <string>
#include <vector>
#include "Helper/AccuracyEvaluator.h"


class CUDA_TransformerTool {
public:

    CUDA_TransformerTool();

    std::vector<std::string> analyze();

    std::vector<std::string> transform(std::string optimizationString);

    void setAccuracyEvaluator(std::unique_ptr<AccuracyEvaluator> evaluator);

private:

    ConfigParser parser;

    std::unique_ptr<AccuracyEvaluator> evaluator;

    std::unique_ptr<clang::tooling::CompilationDatabase> Compilations;

    std::map<std::string, std::string> Configurations;

    std::string optimization_indices = "";

    std::vector<std::string> run(std::string& optimizationString);

    std::vector<std::string> getDepoResults(std::string depoOutputString);
    
    std::string readProgramOutput();

    void copyTransformedToOriginal(std::string& optimizationString);

    void saveOriginal();

    void revertToOriginal();
};
