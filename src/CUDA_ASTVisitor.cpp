#include "Transformer/CUDA_ASTVisitor.h"

CUDA_ASTVisitor::CUDA_ASTVisitor(clang::ASTContext *context, clang::Rewriter &writer, Expressions &targetExpressions)
    : context(context), writer(writer), parentMapContext(*context), isVisitorInsideKernel(false), isNextIfNested(false),
      targetExpressions(targetExpressions)
{
}
bool CUDA_ASTVisitor::hasKernelLaunch(clang::ForStmt *forStmt)
{
    if (!forStmt) // Check if forStmt is null
    {
        return false;
    }

    // Traverse the body of the loop to check if it contains a CUDA kernel launch
    for (const clang::Stmt *S : forStmt->getBody()->children())
    {
        if (!S) // Skip if statement is null
        {
            continue;
        }

        // Recursively check the statement and its sub-statements for kernel calls
        if (const clang::CUDAKernelCallExpr *kernelCall = llvm::dyn_cast<clang::CUDAKernelCallExpr>(S))
        {

            return true; // This loop contains a kernel launch
        }

        // If the body contains other compound statements or nested loops, we may need to dive deeper
        for (const clang::Stmt *subStmt : S->children())
        {
            if (!subStmt) // Skip null sub-statements
            {
                continue;
            }

            if (llvm::isa<clang::CUDAKernelCallExpr>(subStmt))
            {
                return true; // This nested sub-statement contains a kernel launch
            }
        }
    }

    return false; // No kernel launch found in this loop
}

bool CUDA_ASTVisitor::VisitFunctionDecl(clang::FunctionDecl *funcDecl)
{
    if (!funcDecl)
    {
        return false;
    }

    if (funcDecl->hasAttr<clang::CUDAGlobalAttr>())
    {
        isVisitorInsideKernel = true;

        // Catch the body of the kernel function
        clang::Stmt* stmt = funcDecl->getBody();
        if (!stmt) return false;  // or handle safely

        auto *body = llvm::dyn_cast<clang::CompoundStmt>(stmt);
        if (!body) return false;

        if (body && !body->body_empty())
        {
            std::unordered_set<clang::Stmt *> processedStatements;

            for (auto *item : body->body())
            {
                if (!item)
                {
                    llvm::errs() << "item is null" << "\n";
                    continue;
                }

                if (auto *ifStmt = llvm::dyn_cast<clang::IfStmt>(item))
                {
                    // Get the if-else source range
                    clang::SourceLocation start = ifStmt->getIfLoc();

                    if (processedStatements.find(ifStmt) != processedStatements.end())
                    {
                        continue; // Skip already processed statements
                    }

                    std::vector<clang::Stmt *> currentBranch;
                    currentBranch.push_back(ifStmt->getThen());
                    processedStatements.insert(ifStmt->getThen());


                    // If the if statement has an else branch
                    if (clang::Stmt *elseStmt = ifStmt->getElse())
                    {

                        clang::SourceLocation end;

                        // Check if the else branch is an if statement (else if)
                        if (auto *elseIfStmt = llvm::dyn_cast<clang::IfStmt>(elseStmt))
                        {
                            currentBranch.push_back(elseIfStmt->getThen());
                            processedStatements.insert(elseIfStmt->getThen());

                            // Check if the else if statement has an else branch 
                            if (auto *elseStmt = elseIfStmt->getElse())
                            {
                                currentBranch.push_back(elseStmt);
                                processedStatements.insert(elseStmt);
                                end = elseStmt->getEndLoc();
                            }
                            else
                            {
                                end = elseIfStmt->getEndLoc();
                            }

                        
                        }
                        // If the else branch is not an if statement - pure else
                        else
                        {
                            currentBranch.push_back(elseStmt);
                            processedStatements.insert(elseStmt);
                            end = elseStmt->getEndLoc();

                        }
                        

                        targetExpressions.ifElseSourceRange.push(clang::SourceRange(start, end));
                    }

                    // If the if statement does not have an else branch - only one if branch
                    else
                    {
                        clang::SourceLocation end = ifStmt->getEndLoc();

                        targetExpressions.ifElseSourceRange.push(clang::SourceRange(start, end));
                    }
                   
                    // Store the if-else body
                    targetExpressions.ifElseBodies.push_back(currentBranch);
                }
            }
        }
    }
    else
    {
        isVisitorInsideKernel = false;
    }
    return true;
}

bool CUDA_ASTVisitor::checkNestedIf(clang::Stmt *stmt)
{
    if (!stmt)
    {
        return false;
    }

    for (auto child : stmt->children())
    {
        if (auto nestedIf = llvm::dyn_cast<clang::IfStmt>(child))
        {
            llvm::errs() << "Nested If Statement Found:\n";
            isNextIfNested = true;
            return true;
        }
    }
    return false;
}

bool CUDA_ASTVisitor::VisitCUDAKernelCallExpr(clang::CUDAKernelCallExpr *kernelCall)
{
    targetExpressions.kernelCalls.push_back(kernelCall);
    return true;
}

bool CUDA_ASTVisitor::VisitCallExpr(clang::CallExpr *callExpr)
{
    // Change expressions that only inside the kernel function
    if (isVisitorInsideKernel)
    {
        // Check for null pointer
        if (const clang::FunctionDecl *callee = callExpr->getDirectCallee())
        {
            if (callee->getNameAsString() == "__syncthreads")
            {
                targetExpressions.syncthreadCalls.push_back(callExpr);
            }
            else if (callee->getNameAsString().substr(0, 6) == "atomic")
            {
                targetExpressions.atomicCalls.push_back(callExpr);
            }
        }
    }

    return true;
}

bool CUDA_ASTVisitor::VisitTypeLoc(clang::TypeLoc typeLoc)
{
    if (isVisitorInsideKernel)
    {
        clang::QualType type = typeLoc.getType();

        if (type->isSpecificBuiltinType(clang::BuiltinType::Double))
        {
            targetExpressions.doubles.push_back(typeLoc);
        }
    }

    return true;
}

bool CUDA_ASTVisitor::VisitForStmt(clang::ForStmt *forStmt)
{

    if (isVisitorInsideKernel)
    {
        targetExpressions.forStmts.push_back(forStmt);
    }
    else
    {
        // Check if the for loop contains a kernel launch in the host
        if (hasKernelLaunch(forStmt))
        {
            targetExpressions.kernelLaunchforStmts.push_back(forStmt);
        }
    }

    return true;
}