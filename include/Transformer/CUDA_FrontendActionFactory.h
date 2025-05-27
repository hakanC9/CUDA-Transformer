#ifndef CUDA_FRONTENDACTION_FACTORY_H
#define CUDA_FRONTENDACTION_FACTORY_H

#include "clang/Tooling/Tooling.h"

#include <memory>
#include <string>

#include "Transformer/CUDA_FrontendAction.h"


class CUDA_FrontendActionFactory : public clang::tooling::FrontendActionFactory {
    public:
        CUDA_FrontendActionFactory(
            bool analyzerMode,
            const std::string &optChoices,
            std::shared_ptr<std::string> optimizationPossibilities,
            std::shared_ptr<std::string>  outputFilePath,
            std::string resultDir)
            : 
            AnalyzerMode(analyzerMode), 
            OptChoices(optChoices), 
            optimizationPossibilities(optimizationPossibilities), 
            outputFilePath(outputFilePath),
            resultDir(resultDir)
            {}
    
        std::unique_ptr<clang::FrontendAction> create() override {
            return std::make_unique<CUDA_FrontendAction>(AnalyzerMode, OptChoices, optimizationPossibilities, outputFilePath, resultDir);
        }
            
    
    private:
        bool AnalyzerMode;
        std::string OptChoices;
        std::shared_ptr<std::string> optimizationPossibilities;
        std::shared_ptr<std::string>  outputFilePath;
        std::string resultDir;
    };

#endif
