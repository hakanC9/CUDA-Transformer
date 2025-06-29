#include "Transformer/CUDA_Transform_FrontendActionFactory.h"
#include "Transformer/CUDA_Transform_FrontendAction.h"

CUDA_Transform_FrontendActionFactory::CUDA_Transform_FrontendActionFactory(
    std::vector<std::string> optimizationsToApply)
    :
    optimizationsToApply(optimizationsToApply)
    {}

std::unique_ptr<clang::FrontendAction> CUDA_Transform_FrontendActionFactory::create()  {

    std::string optChoices = optimizationsToApply.at(optimizationIndex++);
    return std::make_unique<CUDA_Transform_FrontendAction>(optChoices);
}

