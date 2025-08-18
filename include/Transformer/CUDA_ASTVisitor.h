#pragma once

#include "clang/AST/ASTTypeTraits.h"
#include "clang/AST/ParentMap.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <unordered_set>

#include "Transformer/Expressions.h"

class CUDA_ASTVisitor : public clang::RecursiveASTVisitor<CUDA_ASTVisitor>{

public:
  explicit CUDA_ASTVisitor(clang::ASTContext *context, clang::Rewriter &writer, Expressions &targetExpressions);

  bool VisitCUDAKernelCallExpr(clang::CUDAKernelCallExpr *kernelCallEexpr);
  bool VisitCallExpr(clang::CallExpr *callExpr);
  bool VisitFunctionDecl(clang::FunctionDecl *funcDecl);
  bool VisitTypeLoc(clang::TypeLoc typeLoc);
  bool VisitForStmt(clang::ForStmt *forStmt);
  Expressions &targetExpressions;

private:
  clang::ASTContext *context;
  clang::Rewriter &writer;
  clang::ParentMapContext parentMapContext;

  // We aim that changed the inside the kernel function(mostly)
  // So we need the ensure the expressions is inside the kernel function not the host functions
  bool isVisitorInsideKernel;
  bool isVisitorInsideDevice;
  bool isNextIfNested; // Flag to indicate whether the next if statement is nested

  bool checkNestedIf(clang::Stmt *stmt);
  std::vector<clang::Stmt *> bodies; // Store the bodies of if-else statements

  bool hasKernelLaunch(clang::ForStmt *forStmt);

};

