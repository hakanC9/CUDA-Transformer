#pragma once

#include "Commands/TransformCommand.h"

class ReplaceSyncWithWarp : public TransformCommand
{
    clang::Rewriter &rewriter;
    clang::CallExpr *syncCall;

  public:
    ReplaceSyncWithWarp(clang::Rewriter &rewriter, clang::CallExpr *syncCall);

    void execute() override;
};

