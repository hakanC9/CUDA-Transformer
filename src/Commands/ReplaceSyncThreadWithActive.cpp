#include "Commands/ReplaceSyncThreadWithActive.h"

ReplaceSyncThreadWithActive::ReplaceSyncThreadWithActive(clang::Rewriter &rewriter, clang::CallExpr *syncCall,
                                                         clang::ASTContext &context)
    : rewriter(rewriter), syncCall(syncCall), context(context)
{
}

void ReplaceSyncThreadWithActive::execute()
{

    // Get the beginning of the code because cooperative_groups header must be included
    clang::SourceManager &sourceManager = context.getSourceManager();
    clang::FileID mainFileID = sourceManager.getMainFileID();
    clang::SourceLocation InsertLoc = sourceManager.getLocForStartOfFile(mainFileID);

    rewriter.InsertText(InsertLoc, "#include <cooperative_groups.h>\n");

    std::string replacementText =
        "cooperative_groups::thread_group active1 = cooperative_groups::coalesced_threads();\n\t active1.sync();\n";

    rewriter.ReplaceText(syncCall->getSourceRange(), replacementText);

    // Add a comment after the full statement, after the semicolon
    rewriter.InsertTextAfterToken(syncCall->getEndLoc().getLocWithOffset(1),
                                  "\t//////// CUDA-TRANSFORMER WAS HERE : __syncthreads() replaced with Synchronize "
                                  "only 4 threads in the group");
}
