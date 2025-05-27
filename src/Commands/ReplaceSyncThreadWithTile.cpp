#include "Commands/ReplaceSyncThreadWithTile.h"

ReplaceSyncThreadWithTile::ReplaceSyncThreadWithTile(clang::Rewriter &rewriter, clang::CallExpr *syncCall,
                                                     clang::ASTContext &context)
    : rewriter(rewriter), syncCall(syncCall), context(context)
{
}

void ReplaceSyncThreadWithTile::execute()
{

    // Get the beginning of the code because cooperative_groups header must be included
    clang::SourceManager &sourceManager = context.getSourceManager();
    clang::FileID mainFileID = sourceManager.getMainFileID();
    clang::SourceLocation InsertLoc = sourceManager.getLocForStartOfFile(mainFileID);

    rewriter.InsertText(InsertLoc, "#include <cooperative_groups.h>\n");

    std::string replacementText =
        "cooperative_groups::thread_group tile32_1 = "
        "cooperative_groups::tiled_partition(cooperative_groups::this_thread_block(), 32);\n\t";
    replacementText +=
        "cooperative_groups::thread_group tile4_1 = cooperative_groups::tiled_partition(tile32_1, 4);\n\t";
    replacementText += "tile4_1.sync()";

    rewriter.ReplaceText(syncCall->getSourceRange(), replacementText);

    // Add a comment after the full statement, after the semicolon
    rewriter.InsertTextAfterToken(syncCall->getEndLoc().getLocWithOffset(1),
                                  "\t//////// CUDA-TRANSFORMER WAS HERE : __syncthreads() replaced with Synchronize "
                                  "only 4 threads in the group");
}
