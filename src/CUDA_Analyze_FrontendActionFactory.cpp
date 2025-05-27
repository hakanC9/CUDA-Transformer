#include "Transformer/CUDA_Analyze_FrontendActionFactory.h"


CUDA_Analyze_FrontendActionFactory::CUDA_Analyze_FrontendActionFactory(){
    this->optimizationPossibilities = std::make_shared<std::vector<std::string>>();
}

std::unique_ptr<clang::FrontendAction> CUDA_Analyze_FrontendActionFactory::create()  {
    return std::make_unique<CUDA_Analyze_FrontendAction>(optimizationPossibilities);
}


std::vector<std::string> CUDA_Analyze_FrontendActionFactory::getOptimizationPossibilities(){
    return *optimizationPossibilities;
}