#include "Commands/TransformCommand.h"

class LoopPerforation : public TransformCommand
{
    clang::Rewriter &rewriter;
    clang::ForStmt *forStmt;
    clang::ASTContext &context;
    float perforationRate;
    void printForStmt();

  public:
    LoopPerforation(clang::Rewriter &rewriter, clang::ForStmt *forStmt, clang::ASTContext &context, float perforationRate);

    void execute() override;
};