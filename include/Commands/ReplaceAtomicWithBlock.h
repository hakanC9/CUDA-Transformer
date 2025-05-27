// ReplaceSyncWithWarpCommand.h
#ifndef REPLACE_ATOMIC_WITH_BLOCK_H
#define REPLACE_ATOMIC_WITH_BLOCK_H

#include "Commands/TransformCommand.h"
#include "clang/Lex/Lexer.h"

class ReplaceAtomicWithBlock : public TransformCommand
{
    clang::Rewriter &rewriter;
    clang::CallExpr *atomicCall;
    clang::ASTContext &context;

  public:
    ReplaceAtomicWithBlock(clang::Rewriter &rewriter, clang::CallExpr *atomicCall, clang::ASTContext &context);

    void execute() override;
};

#endif