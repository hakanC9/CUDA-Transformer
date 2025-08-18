#include "Transformer/CUDA_Transform_ASTConsumer.h"
#include <string>
#include "Helper/TimeoutException.h"


CUDA_Transform_ASTConsumer::CUDA_Transform_ASTConsumer(
    clang::ASTContext *context, 
    clang::Rewriter &writer, 
    Expressions &targetExpressions,
    Transformer &transformer, 
    std::string optimizationChoices,
    const std::atomic<bool>& stopFlag)
    : 
    analysisVisitor(context, writer, targetExpressions), 
    writer(writer), transformer(transformer),
    targetExpressions(targetExpressions), 
    optimizationChoices(optimizationChoices),
    stopFlag(stopFlag)
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
        if (stopFlag) throw TimeoutException("Timeout occured while applying optimization choices for syncCall!");
        transformer.analyzeSyncthread(syncCall, writer, context);
    }

    for (clang::CallExpr *atomicCall : targetExpressions.atomicCalls){
        if (stopFlag) throw TimeoutException("Timeout occured while applying optimization choices for atomicCall!");
        transformer.analyzeAtomicCalls(atomicCall, writer, context);
    }

    for (clang::TypeLoc doubleTypeLoc : targetExpressions.doubles){
        if (stopFlag) throw TimeoutException("Timeout occured while applying optimization choices for doubleTypeLoc!");
        transformer.analyzeDoubles(doubleTypeLoc, writer);
    }

    for (clang::CUDAKernelCallExpr *kernelCall : targetExpressions.kernelCalls){
        if (stopFlag) throw TimeoutException("Timeout occured while applying optimization choices for kernelCall!");
        transformer.analyzeKernelCall(kernelCall, writer, context);
    }

    for (clang::ForStmt *forStmt : targetExpressions.forStmts){
        if (stopFlag) throw TimeoutException("Timeout occured while applying optimization choices for forStmt!");
        transformer.analyzeForStmt(forStmt, writer, context);
    }

    for (clang::ForStmt *kernelLaunchforStmt : targetExpressions.kernelLaunchforStmts){
        if (stopFlag) throw TimeoutException("Timeout occured while applying optimization choices for kernelLaunchforStmt!");
        transformer.analyzeForStmt(kernelLaunchforStmt, writer, context);
    }

     for (std::vector<clang::Stmt *> ifElseBody : targetExpressions.ifElseBodies){
        if (stopFlag) throw TimeoutException("Timeout occured while applying optimization choices for ifElseBody!");
        transformer.analyzeIfElse(ifElseBody, writer, context, targetExpressions.ifElseSourceRange);
    }

}
