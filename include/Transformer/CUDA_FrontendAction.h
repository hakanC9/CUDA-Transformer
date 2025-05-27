#ifndef CUDA_FRONTENDACTION_H
#define CUDA_FRONTENDACTION_H

#include "clang/Frontend/FrontendAction.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <filesystem>
#include <memory>
#include <string>

#include "Transformer/Expressions.h"
#include "Transformer/Transformer.h"

class CUDA_FrontendAction : public clang::ASTFrontendAction {
    clang::Rewriter TheRewriter;
    Expressions targetExpressions;
    Transformer transformer;
    bool AnalyzerMode;
    const std::string &OptChoices;
    std::shared_ptr<std::string> optimizationPossibilities;
    std::shared_ptr<std::string>  outputFilePath;
    std::string resultDir;

public:
    CUDA_FrontendAction(
        bool analyzerMode,
        const std::string &optChoices,
        std::shared_ptr<std::string> optimizationPossibilities,
        std::shared_ptr<std::string> outputFilePath, std::string resultDir);

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &Compiler,
        llvm::StringRef InFile) override;

    void EndSourceFileAction() override {

        if (!AnalyzerMode){

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
            
            *outputFilePath = outputPath.generic_string();
        }
    }

    std::string getOutputFilePath(){
        return *outputFilePath;
    }

};

#endif