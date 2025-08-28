#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "Commands/KernelCallReduction.h"
#include "Commands/RemoveIfElseBranches.h"
#include "Commands/RemoveSyncThread.h"
#include "Commands/ReplaceAtomicWithBlock.h"
#include "Commands/ReplaceAtomicWithDirect.h"
#include "Commands/ReplaceSyncThreadWithActive.h"
#include "Commands/ReplaceSyncThreadWithTile.h"
#include "Commands/ReplaceSyncWithWarp.h"
#include "Commands/ChooseIfElseBranch.h"
#include "Commands/ReplaceDoubleWithFloat.h"
#include "Commands/LoopPerforation.h"
#include "Commands/TransformCommand.h"

class Transformer{

public:

    Transformer() = default;
    ~Transformer() = default; // Destructor to clean up commands
    void executeCommands();   // Execute all commands
    void addCommand(std::unique_ptr<TransformCommand> command);

    /* Analyze and ask user to choice for optimization
      */
    void analyzeSyncthread(clang::CallExpr *callExpr, clang::Rewriter &writer, clang::ASTContext &context);
    void analyzeAtomicCalls(clang::CallExpr *callExpr, clang::Rewriter &writer, clang::ASTContext &context);
    void analyzeKernelCall(clang::CUDAKernelCallExpr *callExpr, clang::Rewriter &writer, clang::ASTContext &context);
    void analyzeIfElse(std::vector<clang::Stmt *> &ifElseBody, clang::Rewriter &writer, clang::ASTContext &context,
                        std::queue<clang::SourceRange> &ifElseSourceRange);
    void analyzeDoubles(clang::TypeLoc typeLoc, clang::Rewriter &writer);
    void analyzeForStmt(clang::ForStmt *forStmt, clang::Rewriter &writer, clang::ASTContext &context);

    void getOptimizationChoices(const std::string &choices);

private:

    std::vector<std::unique_ptr<TransformCommand>> commands; // List of commands to execute

    std::string optimizationChoices;

    void displayWrongChoiceError();
    std::queue<int> choiceQueue;
};
