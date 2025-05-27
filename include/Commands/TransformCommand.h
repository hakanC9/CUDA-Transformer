#ifndef OPTIMIZATION_COMMAND_H
#define OPTIMIZATION_COMMAND_H

#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ASTContext.h"


class TransformCommand
{
    public:
        virtual ~TransformCommand() = default;
        virtual void execute() = 0;
};

#endif