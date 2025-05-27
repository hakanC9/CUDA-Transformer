#include "Commands/KernelCallReduction.h"

KernelCallReduction::KernelCallReduction(clang::Rewriter &rewriter, clang::CUDAKernelCallExpr *kernelCall,
                                         clang::ASTContext &context, int blockReductionRate, int threadReductionRate)
    : rewriter(rewriter), kernelCall(kernelCall), context(context), blockReductionRate(blockReductionRate),
      threadReductionRate(threadReductionRate)
{
}

void KernelCallReduction::changeLaunchParameter(const clang::Expr *callArg, bool isDim3, int reductionRate)
{
    std::string newText = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(callArg->getSourceRange()),
                                                      context.getSourceManager(), context.getLangOpts())
                              .str();
    if (isDim3)
    {
        newText += ".x -= " + std::to_string(reductionRate) + ";\t";

        // Add comment that transformer changes
        newText += "\t//////// CUDA-TRANSFORMER WAS HERE : dim3 parameter changed \n\t";

        rewriter.InsertTextBefore(kernelCall->getBeginLoc(), newText);
    }
    else
    {
        newText += " -" + std::to_string(reductionRate);

        rewriter.ReplaceText(callArg->getSourceRange(), newText);

        // Add a comment after the full statement, after the semicolon
        rewriter.InsertTextAfterToken(kernelCall->getEndLoc().getLocWithOffset(1),
                                      "\t\t//////// CUDA-TRANSFORMER WAS HERE : integer parameter changed");
    }
}

void KernelCallReduction::execute()
{
    const auto *config = kernelCall->getConfig();

    if (config && config->getNumArgs() >= 2)
    {
        const clang::Expr *blockArg = config->getArg(0);
        const clang::Expr *threadArg = config->getArg(1);

        // For the first parameter
        if (const clang::CXXConstructExpr *constructExpr = llvm::dyn_cast<clang::CXXConstructExpr>(blockArg))
        {
            // True state that its dim3 parameter so this change is different from other
            changeLaunchParameter(constructExpr, true, blockReductionRate);
        }
        else
        {
            changeLaunchParameter(blockArg, false, blockReductionRate);
        }

        // For the second parameter
        if (const clang::CXXConstructExpr *constructExpr = llvm::dyn_cast<clang::CXXConstructExpr>(threadArg))
        {
            // True state that its dim3 parameter so this change is different from other
            changeLaunchParameter(constructExpr, true, threadReductionRate);
        }
        else
        {
            changeLaunchParameter(threadArg, false, threadReductionRate);
        }
    }
}
