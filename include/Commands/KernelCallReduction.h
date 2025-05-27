#ifndef KERNEL_CALL_REDUCTION_H
#define KERNEL_CALL_REDUCTION_H

#include "Commands/TransformCommand.h"
#include "clang/AST/ExprCXX.h"
#include "clang/Lex/Lexer.h"

class KernelCallReduction : public TransformCommand
{
    clang::Rewriter &rewriter;
    clang::CUDAKernelCallExpr *kernelCall;
    clang::ASTContext &context;
    int blockReductionRate, threadReductionRate;
    void changeLaunchParameter(const clang::Expr *callArg, bool isDim3, int reductionRate);

  public:
    KernelCallReduction(clang::Rewriter &rewriter, clang::CUDAKernelCallExpr *kernelCall, clang::ASTContext &context,
                        int blockReductionRate, int threadReductionRate);

    void execute() override;
};

#endif