#include "Commands/LoopPerforation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include <regex>


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
    if (!forStmt) {
        llvm::errs() << "Error: forStmt is null\n";
        return;
    }

    clang::Expr *incExpr = forStmt->getInc();
    if (!incExpr) {
        llvm::errs() << "Error: forStmt->getInc() is null\n";
        return;
    }

    clang::SourceManager &SM = context.getSourceManager();
    clang::LangOptions LangOpts = context.getLangOpts();

    clang::SourceLocation start = SM.getExpansionLoc(incExpr->getBeginLoc());
    clang::SourceLocation end = SM.getExpansionLoc(incExpr->getEndLoc());
    end = clang::Lexer::getLocForEndOfToken(end, 0, SM, LangOpts);
    clang::SourceRange incRange(start, end);

    std::string incStr = clang::Lexer::getSourceText(
        clang::CharSourceRange::getCharRange(incRange), SM, LangOpts).str();


    std::string newIncStr;
    std::smatch match;
    bool matched = false;

    // Regexes
    std::regex assignAddPattern(R"(\s*(\w+)\s*=\s*\1\s*\+\s*(\w+)\s*)");
    std::regex plusEqualsPattern(R"(\s*(\w+)\s*\+=\s*(\w+)\s*)");
    std::regex postIncPattern(R"(\s*(\w+)\s*\+\+\s*)");

    std::regex assignSubPattern(R"(\s*(\w+)\s*=\s*\1\s*-\s*(\w+)\s*)");
    std::regex minusEqualsPattern(R"(\s*(\w+)\s*-=\s*(\w+)\s*)");
    std::regex postDecPattern(R"(\s*(\w+)\s*--\s*)");

    std::regex assignDivPattern(R"(\s*(\w+)\s*=\s*\1\s*/\s*(\w+)\s*)");
    std::regex assignMulPattern(R"(\s*(\w+)\s*=\s*\1\s*\*\s*(\w+)\s*)");

    // Match and generate transformed increment
    if (std::regex_match(incStr, match, assignAddPattern)) {
        matched = true;
        newIncStr = match[1].str() + " = " + match[1].str() + " + (" + match[2].str() + " * " + std::to_string(perforationRate) + "))";
    }
    else if (std::regex_match(incStr, match, plusEqualsPattern)) {
        matched = true;
        newIncStr = match[1].str() + " += (" + match[2].str() + " * " + std::to_string(perforationRate) + "))";
    }
    else if (std::regex_match(incStr, match, postIncPattern)) {
        matched = true;
        newIncStr = match[1].str() + " += " + std::to_string(perforationRate) +")";
    }
    else if (std::regex_match(incStr, match, assignSubPattern)) {
        matched = true;
        newIncStr = match[1].str() + " = " + match[1].str() + " - (" + match[2].str() + " * " + std::to_string(perforationRate) + "))";
    }
    else if (std::regex_match(incStr, match, minusEqualsPattern)) {
        matched = true;
        newIncStr = match[1].str() + " -= (" + match[2].str() + " * " + std::to_string(perforationRate) + "))";
    }
    else if (std::regex_match(incStr, match, postDecPattern)) {
        matched = true;
        newIncStr = match[1].str() + " -= " + std::to_string(perforationRate) + ")";
    }
    else if (std::regex_match(incStr, match, assignDivPattern)) {
        matched = true;
        newIncStr = match[1].str() + " = " + match[1].str() + " / (" + match[2].str() + " * " + std::to_string(perforationRate) + "))";
    }
    else if (std::regex_match(incStr, match, assignMulPattern)) {
        matched = true;
        newIncStr = match[1].str() + " = " + match[1].str() + " * (" + match[2].str() + " * " + std::to_string(perforationRate) + "))";
    }

    if (!matched) {
        llvm::errs() << "SKIPPED: unsupported increment format: [" << incStr << "]\n";
        return;
    }

    // Replace the original increment expression
    rewriter.ReplaceText(incRange, newIncStr);

    // Insert a comment before the loop
    rewriter.InsertTextBefore(forStmt->getBeginLoc(),
        "//////// CUDA-TRANSFORMER WAS HERE : Loop Perforation\n");
}


