#pragma once

#include "clang/Frontend/FrontendAction.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <filesystem>
#include <memory>
#include <string>

#include "Transformer/Expressions.h"
#include "Transformer/Transformer.h"

class CUDA_Transform_FrontendAction : public clang::ASTFrontendAction {

    public:
        CUDA_Transform_FrontendAction(std::string optChoices, std::string& optimizationString);

        std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
            clang::CompilerInstance &Compiler,
            llvm::StringRef InFile) override;

        void EndSourceFileAction() override;

    
    private:
        clang::Rewriter TheRewriter;
        Expressions targetExpressions;
        Transformer transformer;
        
        std::string OptChoices;
        std::string& optimizationString;
};
