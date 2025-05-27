#ifndef CUDA_ANALYZE_AST_CONSUMER_H
#define CUDA_ANALYZE_AST_CONSUMER_H

#include "clang/AST/ASTConsumer.h"

#include <string>

#include "Transformer/CUDA_ASTVisitor.h"
#include "Transformer/Transformer.h"


class CUDA_Analyze_ASTConsumer : public clang::ASTConsumer
{

    public:
        explicit CUDA_Analyze_ASTConsumer(clang::ASTContext *context, clang::Rewriter &writer, Expressions &targetExpressions,
                                Transformer &transformer, std::shared_ptr<std::vector<std::string>> optimizationPossibilities);

        virtual void HandleTranslationUnit(clang::ASTContext &Context) override;
        Expressions &targetExpressions;
        std::string getOptimizationPossibilities();

    private:
        CUDA_ASTVisitor analysisVisitor;
        clang::Rewriter &writer;
        Transformer &transformer;
        std::shared_ptr<std::vector<std::string>> optimizationPossibilities;
        
};

#endif