#include "Commands/LoopPerforation.h"
#include "clang/Lex/Lexer.h"

LoopPerforation::LoopPerforation(clang::Rewriter &rewriter, clang::ForStmt *forStmt, clang::ASTContext &context,
                                 float perforationRate)
    : rewriter(rewriter), forStmt(forStmt), context(context), perforationRate(perforationRate)
{
}

void LoopPerforation::printForStmt()
{
    clang::PrintingPolicy policy(context.getLangOpts());
    llvm::errs() << "For loop before transformation:\n";
    forStmt->printPretty(llvm::errs(), nullptr, policy);
    llvm::errs() << "*********************************************************\n";
}

void LoopPerforation::execute()
{

    std::string appendedString = " * (" + std::to_string(perforationRate) + " )";

    if (!forStmt) {
        llvm::errs() << "Error: forStmt is null\n";
        return;
    }

    if (!forStmt->getInc()) {
        llvm::errs() << "Error: forStmt->getInc() is null\n";
        return;
    }

    if (!forStmt->getBeginLoc().isValid()) {
        llvm::errs() << "Error: forStmt->getBeginLoc() is invalid\n";
        return;
    }

    clang::Expr *incExpr = forStmt->getInc();
    clang::SourceRange incRange = incExpr->getSourceRange();

    std::string incStr = clang::Lexer::getSourceText(
        clang::CharSourceRange::getTokenRange(incRange),
        context.getSourceManager(), context.getLangOpts()).str();

    std::string newIncStr = "(" + incStr + ") * " + std::to_string(perforationRate);

    rewriter.ReplaceText(incRange, newIncStr);

 

    rewriter.InsertTextBefore(forStmt->getBeginLoc(), "//////// CUDA-TRANSFORMER WAS HERE : Loop Perforation\n");
}