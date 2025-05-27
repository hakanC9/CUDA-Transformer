#include "Commands/ReplaceDoubleWithFloat.h"

ReplaceDoubleWithFloat::ReplaceDoubleWithFloat(clang::Rewriter &rewriter, clang::TypeLoc typeLoc)
    : rewriter(rewriter), typeLoc(typeLoc)
{
}

void ReplaceDoubleWithFloat::execute()
{
    rewriter.ReplaceText(typeLoc.getSourceRange(), "float");
    rewriter.InsertTextBefore(typeLoc.getSourceRange().getBegin(),
                        "//////// CUDA-TRANSFORMER WAS HERE : double type replaced with float\n");
}