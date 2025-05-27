#include "Transformer/CUDA_ASTConsumer.h"
#include "Transformer/CUDA_ASTConsumer.h"
#include <string>

CUDA_ASTConsumer::CUDA_ASTConsumer(
    clang::ASTContext *context, 
    clang::Rewriter &writer, 
    Expressions &targetExpressions,
    Transformer &transformer, 
    bool analyzerMode, 
    const std::string &optimizationChoices, 
    std::shared_ptr<std::string> optimizationPossibilities)
    : 
    analysisVisitor(context, writer, targetExpressions), 
    writer(writer), transformer(transformer),
    targetExpressions(targetExpressions), 
    analyzerMode(analyzerMode), 
    optimizationChoices(optimizationChoices), 
    optimizationPossibilities(optimizationPossibilities) {}

void CUDA_ASTConsumer::HandleTranslationUnit(clang::ASTContext &context)
{

    analysisVisitor.TraverseDecl(context.getTranslationUnitDecl());

    
    if (analyzerMode)
    {
        *optimizationPossibilities = getOptimizationPossibilities();
        return;        
    }
    else{

        transformer.getOptimizationChoices(optimizationChoices);
        applyOptimizationsChoices(context);
        transformer.executeCommands();
    }
    
}

void CUDA_ASTConsumer::applyOptimizationsChoices(clang::ASTContext &context)
{
    /// IMPORTANT : ORDER OF APPLYING IS IMPORTANT  (MUST BE SAME AS PRINTING)

   
    for (clang::CallExpr *syncCall : targetExpressions.syncthreadCalls)
    {
        transformer.analyzeSyncthread(syncCall, writer, context);
    }

    for (clang::CallExpr *atomicCall : targetExpressions.atomicCalls)
    {
        transformer.analyzeAtomicCalls(atomicCall, writer, context);
    }

    for (clang::TypeLoc doubleTypeLoc : targetExpressions.doubles)
    {
        transformer.analyzeDoubles(doubleTypeLoc, writer);
    }

    for (clang::CUDAKernelCallExpr *kernelCall : targetExpressions.kernelCalls)
    {

        transformer.analyzeKernelCall(kernelCall, writer, context);
    }

    for (clang::ForStmt *forStmt : targetExpressions.forStmts)
    {
        transformer.analyzeForStmt(forStmt, writer, context);
    }

    for (clang::ForStmt *kernelLaunchforStmt : targetExpressions.kernelLaunchforStmts)
    {
        transformer.analyzeForStmt(kernelLaunchforStmt, writer, context);
    }

     for (std::vector<clang::Stmt *> ifElseBody : targetExpressions.ifElseBodies)
    {

        transformer.analyzeIfElse(ifElseBody, writer, context, targetExpressions.ifElseSourceRange);
    }


}

std::string CUDA_ASTConsumer::getOptimizationPossibilities()
{

    std::string optPossibilitiesOutput = "";

    /// IMPORTANT : ORDER OF PRINTING IS IMPORTANT
    for (size_t i = 0; i < targetExpressions.syncthreadCalls.size(); i++)
    {
        optPossibilitiesOutput.append("0");
        //std::cout << 0;
    }

    for (size_t i = 0; i < targetExpressions.atomicCalls.size(); i++)
    {
        optPossibilitiesOutput.append("1");
        //std::cout << 1;
    }

    for (size_t i = 0; i < targetExpressions.doubles.size(); i++)
    {
        optPossibilitiesOutput.append("2");
        //std::cout << 2;
    }

    for (size_t i = 0; i < targetExpressions.kernelCalls.size(); i++)
    {
        optPossibilitiesOutput.append("34");
        //std::cout << 3 << 4;
    }

    for (size_t i = 0; i < targetExpressions.kernelLaunchforStmts.size(); i++)
    {
        optPossibilitiesOutput.append("5");
        //std::cout << 5;
    }

    for (size_t i = 0; i < targetExpressions.forStmts.size(); i++)
    {
        optPossibilitiesOutput.append("6");
        //std::cout << 6;
    }


    for (std::vector<clang::Stmt *> ifElseBody : targetExpressions.ifElseBodies)
    {
        if (ifElseBody.size() == 1)
        {
            optPossibilitiesOutput.append("7");
            //std::cout << 7;
        }
        else if (ifElseBody.size() == 2)
        {
            optPossibilitiesOutput.append("8");
            //std::cout << 8;
        }
        else if (ifElseBody.size() == 3)
        {
            optPossibilitiesOutput.append("9");
            //std::cout << 9;
        }
        else
        {
            llvm::errs() << "Error: If-Else body size is not supported\n";
        }
    }
    return optPossibilitiesOutput;
}
