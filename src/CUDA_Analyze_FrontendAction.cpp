#include "Transformer/CUDA_Analyze_FrontendAction.h"
#include "Transformer/CUDA_Analyze_ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"



CUDA_Analyze_FrontendAction::CUDA_Analyze_FrontendAction(std::shared_ptr<std::vector<std::string>> optimizationPossibilities) 
                                                        :optimizationPossibilities(optimizationPossibilities){}


std::unique_ptr<clang::ASTConsumer> CUDA_Analyze_FrontendAction::CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    
    TheRewriter.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());

    return std::make_unique<CUDA_Analyze_ASTConsumer>(&Compiler.getASTContext(), TheRewriter, 
                                                    targetExpressions, transformer, optimizationPossibilities);
}
