#ifndef CUDA_ANALYZE_FRONTENDACTION_H
#define CUDA_ANALYZE_FRONTENDACTION_H


#include "clang/Frontend/FrontendAction.h"
#include "clang/Rewrite/Core/Rewriter.h"


#include "Transformer/Expressions.h"
#include "Transformer/Transformer.h"



class CUDA_Analyze_FrontendAction : public clang::ASTFrontendAction {

    public:
        clang::Rewriter TheRewriter;
        Expressions targetExpressions;
        Transformer transformer;

        CUDA_Analyze_FrontendAction(std::shared_ptr<std::vector<std::string>> optimizationPossibilities);

        std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
            clang::CompilerInstance &Compiler,
            llvm::StringRef InFile) override;

    private:
        std::shared_ptr<std::vector<std::string>> optimizationPossibilities;

};



#endif