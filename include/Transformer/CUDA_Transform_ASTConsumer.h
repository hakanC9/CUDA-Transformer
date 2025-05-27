#ifndef CUDA_TRANSFORM_AST_CONSUMER_H
#define CUDA_TRANSFORM_AST_CONSUMER_H

#include "clang/AST/ASTConsumer.h"

#include <string>

#include "Transformer/Transformer.h"
#include "Transformer/Expressions.h"
#include "Transformer/CUDA_ASTVisitor.h"

class CUDA_Transform_ASTConsumer : public clang::ASTConsumer{


      public:

          explicit CUDA_Transform_ASTConsumer(clang::ASTContext *context, clang::Rewriter &writer, Expressions &targetExpressions,
                                              Transformer &transformer, std::string optimizationChoices);

          virtual void HandleTranslationUnit(clang::ASTContext &Context) override;
          void applyOptimizationsChoices(clang::ASTContext &context); 
          Expressions &targetExpressions;

      private:
      
          CUDA_ASTVisitor analysisVisitor;
          clang::Rewriter &writer;
          Transformer &transformer;
          std::string optimizationChoices;  
      

};

#endif