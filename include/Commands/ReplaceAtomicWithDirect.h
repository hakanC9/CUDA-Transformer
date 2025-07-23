#pragma once

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

