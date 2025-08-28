#pragma once

#include "clang/Tooling/Tooling.h"


class CUDA_Transform_FrontendActionFactory : public clang::tooling::FrontendActionFactory {
    public:

        explicit CUDA_Transform_FrontendActionFactory(
            std::vector<std::string> optimizationsToApply,
            std::string& optimizationString);
    
        std::unique_ptr<clang::FrontendAction> create() override;
        
    private:

        int optimizationIndex = 0;
        std::vector<std::string> optimizationsToApply;
        std::string& optimizationString;
    };


