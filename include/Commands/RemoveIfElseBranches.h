#include "Commands/TransformCommand.h"
#include "clang/Lex/Lexer.h"
#include <queue>


class RemoveIfElseBranches : public TransformCommand
{
    clang::Rewriter &rewriter;
    std::vector<clang::Stmt *> ifElseBody;
    clang::ASTContext &context;
    clang::SourceRange ifElseSourceRange;
    void printIfElseBody();

  public:
    RemoveIfElseBranches(clang::Rewriter &rewriter, std::vector<clang::Stmt *> &ifElseBody, clang::ASTContext &context,
                         clang::SourceRange ifElseSourceRange);

    void execute() override;
};