#pragma once

#include "Commands/TransformCommand.h"
#include "clang/AST/TypeLoc.h"

class ReplaceDoubleWithFloat : public TransformCommand
{
    clang::Rewriter &rewriter;
    clang::TypeLoc typeLoc;

  public:
    ReplaceDoubleWithFloat(clang::Rewriter &rewriter, clang::TypeLoc typeLoc);

    void execute() override;
};