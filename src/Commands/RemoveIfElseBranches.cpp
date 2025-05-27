#include "Commands/RemoveIfElseBranches.h"

RemoveIfElseBranches::RemoveIfElseBranches(clang::Rewriter &rewriter, std::vector<clang::Stmt *> &ifElseBody,
                                           clang::ASTContext &context,
                                           clang::SourceRange ifElseSourceRange)
    : rewriter(rewriter), ifElseBody(ifElseBody), context(context), ifElseSourceRange(ifElseSourceRange)
{
}

void RemoveIfElseBranches::printIfElseBody()
{
    clang::PrintingPolicy policy(context.getLangOpts());
    llvm::errs() << ifElseBody.size() << "\n";
    for (const auto &stmt : ifElseBody)
    {

        stmt->printPretty(llvm::errs(), nullptr, policy);
        llvm::errs() << "*********************************************************\n";
    }
}
void RemoveIfElseBranches::execute()
{

    // Get the start location of the first if statement
    rewriter.RemoveText(ifElseSourceRange);

    rewriter.InsertTextBefore(ifElseSourceRange.getBegin(),
                              "//////// CUDA-TRANSFORMER WAS HERE : If-Else Branches Removed\n");

}