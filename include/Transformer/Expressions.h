#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"

#include <vector>
#include <queue>

class Expressions
{

  public:
    std::vector<clang::CallExpr *> syncthreadCalls;
    std::vector<clang::CallExpr *> atomicCalls;
    std::vector<clang::CUDAKernelCallExpr *> kernelCalls;
    std::vector<std::vector<clang::Stmt *>> ifElseBodies;
    std::queue<clang::SourceRange> ifElseSourceRange;
    std::vector<clang::TypeLoc> doubles;
    std::vector<clang::ForStmt *> forStmts;
    std::vector<clang::ForStmt *> kernelLaunchforStmts;

  
};

#endif