#include "Commands/ReplaceAtomicWithDirect.h"

ReplaceAtomicWithDirect::ReplaceAtomicWithDirect(clang::Rewriter &rewriter, clang::CallExpr *atomicCall,
                                                 clang::ASTContext &context)
    : rewriter(rewriter), atomicCall(atomicCall), context(context)
{
}

void ReplaceAtomicWithDirect::execute()
{


    clang::Expr *ptrExpr = atomicCall->getArg(0);
    clang::Expr *valueExpr = atomicCall->getArg(1);

    // Get the source text for the pointer expression and value expression
    std::string ptrText = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(ptrExpr->getSourceRange()),
                                               context.getSourceManager(), context.getLangOpts())
                              .str();
    std::string valueText = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(valueExpr->getSourceRange()),
                                                 context.getSourceManager(), context.getLangOpts())
                                .str();

    
    std::string newText{};
    std::string functionName = atomicCall->getDirectCallee()->getNameAsString();

    if (functionName == "atomicAdd")
    {

        newText += ptrText + " += " + valueText + ";\n";
    }
    else if (functionName == "atomicSub")
    {

        newText += ptrText + " -= " + valueText + ";\n";
    }

    else if (functionName == "atomicExch")
    {

        newText += ptrText + " = " + valueText + ";\n";
    }

    else if (functionName == "atomicMin")
    {

        newText += ptrText + " = min(" + ptrText + ", " + valueText + ");\n";
    }

    else if (functionName == "atomicMax")
    {

        newText += ptrText + " = max(" + ptrText + ", " + valueText + ");\n";
    }

    else if (functionName == "atomicInc")
    {

        newText += ptrText + " = " + ptrText + " + 1;\n";
    }

    else if (functionName == "atomicDec")
    {

        newText += ptrText + " = " + ptrText + " - 1;\n";
    }

    else if (functionName == "atomicCAS")
    {

        clang::Expr *valueExpr2 = atomicCall->getArg(2);
        std::string valueText2 = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(valueExpr2->getSourceRange()),
                                                      context.getSourceManager(), context.getLangOpts())
                                     .str();

        newText += "if (" + ptrText + " == " + valueText + ") {\n";
        newText += "\t" + ptrText + " = " + valueText2 + ";\n";
        newText += "\t}\n";
    }

    else if (functionName == "atomicAND")
    {

        newText += ptrText + " = " + ptrText + " & " + valueText + ";\n";
    }

    else if (functionName == "atomicOR")
    {

        newText += ptrText + " = " + ptrText + " | " + valueText + ";\n";
    }

    else if (functionName == "atomicXOR")
    {

        newText += ptrText + " = " + ptrText + " ^ " + valueText + ";\n";
    }

    rewriter.ReplaceText(atomicCall->getSourceRange(), newText);

    // Add a comment after the full statement, after the semicolon
    rewriter.InsertTextAfterToken(atomicCall->getEndLoc().getLocWithOffset(1),
                                  "\t//////// CUDA-TRANSFORMER WAS HERE : atomic call replaced with own direct operation");
}