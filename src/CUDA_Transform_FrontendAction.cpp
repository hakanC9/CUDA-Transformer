#include "Transformer/CUDA_Transform_FrontendAction.h"
#include "Transformer/CUDA_Transform_ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"

CUDA_Transform_FrontendAction::CUDA_Transform_FrontendAction(
    std::string optChoices)
    : 
    OptChoices(optChoices)
    {}


std::unique_ptr<clang::ASTConsumer> CUDA_Transform_FrontendAction::CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    
    TheRewriter.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());

    return std::make_unique<CUDA_Transform_ASTConsumer>(&Compiler.getASTContext(), TheRewriter, targetExpressions, 
                                                                        transformer, OptChoices);
}


void CUDA_Transform_FrontendAction::EndSourceFileAction(){

    clang::SourceManager &SM = TheRewriter.getSourceMgr();
    clang::FileID mainFileID = SM.getMainFileID();

    auto EntryRefOrErr = SM.getFileEntryRefForID(mainFileID);
    if (!EntryRefOrErr) {
        llvm::errs() << "Error: Could not get FileEntryRef for main file ID.\n";
        return;
    }

    const clang::FileEntryRef &EntryRef = *EntryRefOrErr;
    std::string originalFileName = std::filesystem::path(EntryRef.getName().str()).filename().string();

    std::string resultDir = "temp_results";
    std::error_code ec;
    if (!std::filesystem::exists(resultDir)) {
        if (!std::filesystem::create_directory(resultDir, ec)) {
            llvm::errs() << "Error creating directory 'temp_results': " << ec.message() << "\n";
            return;
        }
    }

    std::filesystem::path outputPath = std::filesystem::path(resultDir) / originalFileName;

    
    llvm::raw_fd_ostream outFile(outputPath.string(), ec, llvm::sys::fs::OF_None);
    if (ec) {
        llvm::errs() << "Error opening file " << outputPath << ": " << ec.message() << "\n";
        return;
    }

    TheRewriter.getEditBuffer(mainFileID).write(outFile);
    outFile.close();

}