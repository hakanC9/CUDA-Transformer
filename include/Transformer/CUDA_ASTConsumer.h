#ifndef CUDA_AST_CONSUMER_H
#define CUDA_AST_CONSUMER_H

#include "clang/AST/ASTConsumer.h"

#include <string>

#include "Transformer/CUDA_ASTVisitor.h"
#include "Transformer/Transformer.h"


class CUDA_ASTConsumer : public clang::ASTConsumer
{

    CUDA_ASTVisitor analysisVisitor;
    clang::Rewriter &writer;
    Transformer &transformer;
    bool analyzerMode;
    const std::string &optimizationChoices;  
    std::shared_ptr<std::string> optimizationPossibilities; 
    
  public:
    explicit CUDA_ASTConsumer(clang::ASTContext *context, clang::Rewriter &writer, Expressions &targetExpressions,
                              Transformer &transformer, bool analyzerMode, const std::string &optimizationChoices, std::shared_ptr<std::string> optimizationPossibilities);

    virtual void HandleTranslationUnit(clang::ASTContext &Context) override;
    void applyOptimizationsChoices(clang::ASTContext &context); 
    Expressions &targetExpressions;
    std::string getOptimizationPossibilities();
};

#endif