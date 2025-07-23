#pragma once

#include "Commands/TransformCommand.h"

class RemoveSyncThread : public TransformCommand
{
    clang::Rewriter &rewriter;
    clang::CallExpr *syncCall;

  public:
    RemoveSyncThread(clang::Rewriter &rewriter, clang::CallExpr *syncCall);

    void execute() override;
};


