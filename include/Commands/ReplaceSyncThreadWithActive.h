#ifndef REPLACE_SYNC_WITH_ACTIVE_H
#define REPLACE_SYNC_WITH_ACTIVE_H

#include "Commands/TransformCommand.h"
#include "clang/Basic/SourceManager.h"

class ReplaceSyncThreadWithActive : public TransformCommand
{
    clang::Rewriter &rewriter;
    clang::CallExpr *syncCall;
    clang::ASTContext &context;

  public:
    ReplaceSyncThreadWithActive(clang::Rewriter &rewriter, clang::CallExpr *syncCall, clang::ASTContext &context);

    void execute() override;
};

#endif