#include "Transformer/CUDA_Transform_FrontendAction.h"
#include "Transformer/CUDA_Transform_ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"

CUDA_Transform_FrontendAction::CUDA_Transform_FrontendAction(
    std::string optChoices, 
    std::string resultDir)
    : 
    OptChoices(optChoices),
    resultDir(resultDir) {}


std::unique_ptr<clang::ASTConsumer> CUDA_Transform_FrontendAction::CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    
    TheRewriter.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());

    return std::make_unique<CUDA_Transform_ASTConsumer>(&Compiler.getASTContext(), TheRewriter, targetExpressions, 
                                                                        transformer, OptChoices);
}


void CUDA_Transform_FrontendAction::EndSourceFileAction(){

        clang::SourceManager &SM = TheRewriter.getSourceMgr();
        clang::FileID mainFileID = SM.getMainFileID();
    

        std::error_code ec;
    
        // Create the directory if it doesn't exist
        if (!std::filesystem::exists(resultDir)) {
            if (!std::filesystem::create_directory(resultDir, ec)) {
                llvm::errs() << "Error creating directory 'results': " << ec.message() << "\n";
                return;
            }
        }
    

        std::string newFileName = OptChoices + ".cu";   //=> 101.cu
        std::filesystem::path outputPath = resultDir +"/"+ newFileName; //=>example/101.cu 
    
        // Open a file stream for the new file
        llvm::raw_fd_ostream outFile(outputPath.string(), ec, llvm::sys::fs::OF_None);
    
        if (ec) {
            llvm::errs() << "Error opening file " << outputPath << ": " << ec.message() << "\n";
            return;
        }
    
        TheRewriter.getEditBuffer(mainFileID).write(outFile);
        outFile.close();
}