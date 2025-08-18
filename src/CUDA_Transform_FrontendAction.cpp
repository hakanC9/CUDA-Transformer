#include "Transformer/CUDA_Transform_FrontendAction.h"
#include "Transformer/CUDA_Transform_ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"

#include "Helper/TimeoutException.h"

CUDA_Transform_FrontendAction::CUDA_Transform_FrontendAction(
    std::string optChoices,
    std::string& optimizationString,
    const std::atomic<bool>& stopFlag)
    :
    transformer(stopFlag), 
    OptChoices(optChoices),
    optimizationString(optimizationString),
    stopFlag(stopFlag)
    {}


std::unique_ptr<clang::ASTConsumer> CUDA_Transform_FrontendAction::CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    
    TheRewriter.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());

    return std::make_unique<CUDA_Transform_ASTConsumer>(&Compiler.getASTContext(), TheRewriter, targetExpressions, 
                                                                        transformer, OptChoices, stopFlag);
}


void CUDA_Transform_FrontendAction::EndSourceFileAction(){

    if(stopFlag){
        throw TimeoutException("Timeout has occured while writing transformed files");
    }
    else{
        clang::SourceManager &SM = TheRewriter.getSourceMgr();
        clang::FileID mainFileID = SM.getMainFileID();

        auto EntryRefOrErr = SM.getFileEntryRefForID(mainFileID);
        if (!EntryRefOrErr) {
            llvm::errs() << "Error: Could not get FileEntryRef for main file ID.\n";
            return;
        }

        const clang::FileEntryRef &EntryRef = *EntryRefOrErr;
        std::string originalFileName = std::filesystem::path(EntryRef.getName().str()).filename().string();

        std::filesystem::path baseDir = "temp_results";
        std::filesystem::path optDir = baseDir / optimizationString;

        std::error_code ec;
        std::filesystem::create_directories(optDir, ec);
        if (ec) {
            llvm::errs() << "Error creating directory '" << optDir << "': " << ec.message() << "\n";
            return;
        }

        std::filesystem::path outputPath = optDir / originalFileName;

        llvm::raw_fd_ostream outFile(outputPath.string(), ec, llvm::sys::fs::OF_None);
        if (ec) {
            llvm::errs() << "Error opening file " << outputPath << ": " << ec.message() << "\n";
            return;
        }

        TheRewriter.getEditBuffer(mainFileID).write(outFile);
        outFile.close();
    }

}