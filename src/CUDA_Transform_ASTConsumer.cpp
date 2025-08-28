#include "Transformer/CUDA_Transform_ASTConsumer.h"
#include <string>


CUDA_Transform_ASTConsumer::CUDA_Transform_ASTConsumer(
    clang::ASTContext *context, 
    clang::Rewriter &writer, 
    Expressions &targetExpressions,
    Transformer &transformer, 
    std::string optimizationChoices)
    : 
    analysisVisitor(context, writer, targetExpressions), 
    writer(writer), transformer(transformer),
    targetExpressions(targetExpressions), 
    optimizationChoices(optimizationChoices)
    {}

void CUDA_Transform_ASTConsumer::HandleTranslationUnit(clang::ASTContext &context){

    analysisVisitor.TraverseDecl(context.getTranslationUnitDecl());

    transformer.getOptimizationChoices(optimizationChoices);
    applyOptimizationsChoices(context);
    transformer.executeCommands();
}

void CUDA_Transform_ASTConsumer::applyOptimizationsChoices(clang::ASTContext &context){
    /// IMPORTANT : ORDER OF APPLYING IS IMPORTANT

    for (clang::CallExpr *syncCall : targetExpressions.syncthreadCalls){
        transformer.analyzeSyncthread(syncCall, writer, context);
    }

    for (clang::CallExpr *atomicCall : targetExpressions.atomicCalls){
        transformer.analyzeAtomicCalls(atomicCall, writer, context);
    }

    for (clang::TypeLoc doubleTypeLoc : targetExpressions.doubles){
        transformer.analyzeDoubles(doubleTypeLoc, writer);
    }

    for (clang::CUDAKernelCallExpr *kernelCall : targetExpressions.kernelCalls){
        transformer.analyzeKernelCall(kernelCall, writer, context);
    }

    for (clang::ForStmt *forStmt : targetExpressions.forStmts){
        transformer.analyzeForStmt(forStmt, writer, context);
    }

    for (clang::ForStmt *kernelLaunchforStmt : targetExpressions.kernelLaunchforStmts){
        transformer.analyzeForStmt(kernelLaunchforStmt, writer, context);
    }

     for (std::vector<clang::Stmt *> ifElseBody : targetExpressions.ifElseBodies){
        transformer.analyzeIfElse(ifElseBody, writer, context, targetExpressions.ifElseSourceRange);
    }

}
