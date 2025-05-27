// ReplaceSyncWithWarpCommand.h
#ifndef REPLACE_ATOMIC_WITH_DIRECT_H
#define REPLACE_ATOMIC_WITH_DIRECT_H

#include "Commands/TransformCommand.h"
#include "clang/Lex/Lexer.h"

class ReplaceAtomicWithDirect : public TransformCommand
{
    clang::Rewriter &rewriter;
    clang::CallExpr *atomicCall;
    clang::ASTContext &context;

  public:
    ReplaceAtomicWithDirect(clang::Rewriter &rewriter, clang::CallExpr *atomicCall, clang::ASTContext &context);

    void execute() override;
};

#endif