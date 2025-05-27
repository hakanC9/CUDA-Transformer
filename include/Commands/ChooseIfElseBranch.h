#include "Commands/TransformCommand.h"
#include <queue>
#include "clang/Lex/Lexer.h"

class ChooseIfElseBranch : public TransformCommand
{
    clang::Rewriter &rewriter;
    std::vector<clang::Stmt *> ifElseBody;
    clang::ASTContext &context;
    clang::SourceRange ifElseSourceRange;
    int branchNumber;

  public:
    ChooseIfElseBranch(clang::Rewriter &rewriter, std::vector<clang::Stmt *> &ifElseBody, clang::ASTContext &context,
                       clang::SourceRange ifElseSourceRange, int branchNumber);

    void execute() override;
};