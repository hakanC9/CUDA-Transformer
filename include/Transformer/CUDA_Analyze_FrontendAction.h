#pragma once


#include "clang/Frontend/FrontendAction.h"
#include "clang/Rewrite/Core/Rewriter.h"


#include "Transformer/Expressions.h"
#include "Transformer/Transformer.h"



class CUDA_Analyze_FrontendAction : public clang::ASTFrontendAction {

public:
    
    CUDA_Analyze_FrontendAction(std::shared_ptr<std::vector<std::string>> optimizationPossibilities);

    clang::Rewriter TheRewriter;
    Expressions targetExpressions;

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) override;

private:

    std::shared_ptr<std::vector<std::string>> optimizationPossibilities;

};

