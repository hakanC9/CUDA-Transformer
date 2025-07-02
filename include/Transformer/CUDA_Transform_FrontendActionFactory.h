#ifndef CUDA_TRANSFORM_FRONTENDACTION_FACTORY_H
#define CUDA_TRANSFORM_FRONTENDACTION_FACTORY_H


#include "clang/Tooling/Tooling.h"


class CUDA_Transform_FrontendActionFactory : public clang::tooling::FrontendActionFactory {
    public:

        explicit CUDA_Transform_FrontendActionFactory(std::vector<std::string> optimizationsToApply);
    
        std::unique_ptr<clang::FrontendAction> create() override;
        
    private:

        int optimizationIndex = 0;
        std::vector<std::string> optimizationsToApply;
    };


#endif