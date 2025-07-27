#pragma once

#include "Commands/TransformCommand.h"
#include "clang/Basic/SourceManager.h"

class ReplaceSyncThreadWithTile : public TransformCommand
{
    clang::Rewriter &rewriter;
    clang::CallExpr *syncCall;
    clang::ASTContext &context;

  public:
    ReplaceSyncThreadWithTile(clang::Rewriter &rewriter, clang::CallExpr *syncCall, clang::ASTContext &context);

    void execute() override;
};

