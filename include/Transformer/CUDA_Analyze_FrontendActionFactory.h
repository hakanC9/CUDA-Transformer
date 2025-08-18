#pragma once


#include "clang/Tooling/Tooling.h"
#include <vector>
#include <Transformer/CUDA_Analyze_FrontendAction.h>

class CUDA_Analyze_FrontendActionFactory : public clang::tooling::FrontendActionFactory {

public:

    explicit CUDA_Analyze_FrontendActionFactory();
    std::unique_ptr<clang::FrontendAction> create() override;
    std::vector<std::string> getOptimizationPossibilities();

private:
    
    std::shared_ptr<std::vector<std::string>>  optimizationPossibilities;
};


