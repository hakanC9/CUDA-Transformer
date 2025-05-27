#include "Transformer/CUDA_FrontendAction.h"
#include "Transformer/CUDA_ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"

CUDA_FrontendAction::CUDA_FrontendAction(
    bool analyzerMode, 
    const std::string &optChoices, 
    std::shared_ptr<std::string> optimizationPossibilities,
    std::shared_ptr<std::string>outputFilePath,
    std::string resultDir)
    : 
    AnalyzerMode(analyzerMode), 
    OptChoices(optChoices), 
    optimizationPossibilities(optimizationPossibilities),
    outputFilePath(outputFilePath),
    resultDir(resultDir) {}


std::unique_ptr<clang::ASTConsumer> CUDA_FrontendAction::CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    

    TheRewriter.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());

    return std::make_unique<CUDA_ASTConsumer>(&Compiler.getASTContext(), TheRewriter, targetExpressions, 
                                                                        transformer, AnalyzerMode, OptChoices, optimizationPossibilities);
}
