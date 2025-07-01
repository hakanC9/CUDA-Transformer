#include "Transformer/CUDA_Analyze_ASTConsumer.h"


CUDA_Analyze_ASTConsumer::CUDA_Analyze_ASTConsumer(
    clang::ASTContext *context, 
    clang::Rewriter &writer, 
    Expressions &targetExpressions,
    Transformer &transformer, 
    std::shared_ptr<std::vector<std::string>> optimizationPossibilities)
    : 
    analysisVisitor(context, writer, targetExpressions), 
    writer(writer), 
    transformer(transformer),
    targetExpressions(targetExpressions),
    optimizationPossibilities(optimizationPossibilities) {}



void CUDA_Analyze_ASTConsumer::HandleTranslationUnit(clang::ASTContext &context){

    analysisVisitor.TraverseDecl(context.getTranslationUnitDecl());
    optimizationPossibilities->push_back(getOptimizationPossibilities());        
}


std::string CUDA_Analyze_ASTConsumer::getOptimizationPossibilities(){

    std::string optPossibilitiesOutput = "";

    for (size_t i = 0; i < targetExpressions.syncthreadCalls.size(); i++){
        optPossibilitiesOutput.append("0");
    }

    for (size_t i = 0; i < targetExpressions.atomicCalls.size(); i++){
        optPossibilitiesOutput.append("1");
    }

    for (size_t i = 0; i < targetExpressions.doubles.size(); i++){
        optPossibilitiesOutput.append("2");
    }

    for (size_t i = 0; i < targetExpressions.kernelCalls.size(); i++){
        optPossibilitiesOutput.append("34");
    }

    for (size_t i = 0; i < targetExpressions.kernelLaunchforStmts.size(); i++){
        optPossibilitiesOutput.append("5");
    }

    for (size_t i = 0; i < targetExpressions.forStmts.size(); i++){
        optPossibilitiesOutput.append("6");
    }


    for (std::vector<clang::Stmt *> ifElseBody : targetExpressions.ifElseBodies){
        if (ifElseBody.size() == 1){
            optPossibilitiesOutput.append("7");
        }
        else if (ifElseBody.size() == 2){
            optPossibilitiesOutput.append("8");
        }
        else if (ifElseBody.size() == 3){
            optPossibilitiesOutput.append("9");
        }
        else{
            llvm::errs() << "Error: If-Else body size is not supported\n";
        }
    }
    return optPossibilitiesOutput;
}