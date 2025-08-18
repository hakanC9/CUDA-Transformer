#include "Transformer/CUDA_ASTVisitor.h"
#include "clang/AST/Attrs.inc"


CUDA_ASTVisitor::CUDA_ASTVisitor(
    clang::ASTContext *context,
    clang::Rewriter &writer,
    Expressions &targetExpressions)
    : 
    context(context), 
    writer(writer), 
    parentMapContext(*context), 
    isVisitorInsideKernel(false),
    isVisitorInsideDevice(false), 
    isNextIfNested(false),
    targetExpressions(targetExpressions)
    {}
    
bool CUDA_ASTVisitor::hasKernelLaunch(clang::ForStmt *forStmt){
    
    if (!forStmt){
        return false;
    }

    // Traverse the body of the loop to check if it contains a CUDA kernel launch
    for (const clang::Stmt *S : forStmt->getBody()->children()){
        
        if (!S){
            continue;
        }

        // Recursively check the statement and its sub-statements for kernel calls
        if (const clang::CUDAKernelCallExpr *kernelCall = llvm::dyn_cast<clang::CUDAKernelCallExpr>(S)){
            return true; // This loop contains a kernel launch
        }

        // If the body contains other compound statements or nested loops, we may need to dive deeper
        for (const clang::Stmt *subStmt : S->children()){
            
            if (!subStmt){
                continue;
            }

            if (llvm::isa<clang::CUDAKernelCallExpr>(subStmt)){
                return true; // This nested sub-statement contains a kernel launch
            }
        }
    }

    return false; // No kernel launch found in this loop
}

bool CUDA_ASTVisitor::VisitFunctionDecl(clang::FunctionDecl *funcDecl){

    clang::SourceLocation loc = funcDecl->getBeginLoc();
    if (!loc.isValid() || !context->getSourceManager().isInMainFile(loc))
        return true; 

    if (!funcDecl || !funcDecl->hasBody())
        return true;

    if (funcDecl->hasAttr<clang::CUDAGlobalAttr>() || funcDecl->hasAttr<clang::CUDADeviceAttr>()){
        isVisitorInsideKernel = true;

        // Catch the body of the kernel function
        clang::Stmt *funcBody = funcDecl->getBody();
        if (!funcBody) {
            return false;
        }

        clang::CompoundStmt *body = llvm::dyn_cast<clang::CompoundStmt>(funcBody);
        if (!body) {
            return false;
        }

        if (body && !body->body_empty()){
            std::unordered_set<clang::Stmt *> processedStatements;

            for (auto *item : body->body()){
                if (!item){
                    continue;
                }

                if (auto *ifStmt = llvm::dyn_cast<clang::IfStmt>(item)){
                    // Get the if-else source range
                    clang::SourceLocation start = ifStmt->getIfLoc();

                    if (processedStatements.find(ifStmt) != processedStatements.end()){
                        continue; // Skip already processed statements
                    }

                    std::vector<clang::Stmt *> currentBranch;
                    currentBranch.push_back(ifStmt->getThen());
                    processedStatements.insert(ifStmt->getThen());


                    // If the if statement has an else branch
                    if (clang::Stmt *elseStmt = ifStmt->getElse()){

                        clang::SourceLocation end;

                        // Check if the else branch is an if statement (else if)
                        if (auto *elseIfStmt = llvm::dyn_cast<clang::IfStmt>(elseStmt)){
                            currentBranch.push_back(elseIfStmt->getThen());
                            processedStatements.insert(elseIfStmt->getThen());

                            // Check if the else if statement has an else branch 
                            if (auto *elseStmt = elseIfStmt->getElse()){
                                currentBranch.push_back(elseStmt);
                                processedStatements.insert(elseStmt);
                                end = elseStmt->getEndLoc();
                            }
                            else{
                                end = elseIfStmt->getEndLoc();
                            }

                        
                        }
                        // If the else branch is not an if statement - pure else
                        else{
                            currentBranch.push_back(elseStmt);
                            processedStatements.insert(elseStmt);
                            end = elseStmt->getEndLoc();

                        }
                        

                        targetExpressions.ifElseSourceRange.push(clang::SourceRange(start, end));
                    }

                    // If the if statement does not have an else branch - only one if branch
                    else{
                        clang::SourceLocation end = ifStmt->getEndLoc();
                        targetExpressions.ifElseSourceRange.push(clang::SourceRange(start, end));
                    }
                   
                    // Store the if-else body
                    targetExpressions.ifElseBodies.push_back(currentBranch);
                }
            }
        }
    }
    else{
        isVisitorInsideKernel = false;
    }
    return true;
}


bool CUDA_ASTVisitor::checkNestedIf(clang::Stmt *stmt){
    if (!stmt)
    {
        return false;
    }

    for (auto child : stmt->children())
    {
        if (auto nestedIf = llvm::dyn_cast<clang::IfStmt>(child))
        {
            isNextIfNested = true;
            return true;
        }
    }
    return false;
}


bool CUDA_ASTVisitor::VisitCUDAKernelCallExpr(clang::CUDAKernelCallExpr *kernelCall){

    clang::SourceLocation loc = kernelCall->getBeginLoc();
    if (!loc.isValid() || !context->getSourceManager().isInMainFile(loc))
        return true;

    targetExpressions.kernelCalls.push_back(kernelCall);
    return true;
}


bool CUDA_ASTVisitor::VisitCallExpr(clang::CallExpr *callExpr){

    clang::SourceLocation loc = callExpr->getBeginLoc();
    if (!loc.isValid() || !context->getSourceManager().isInMainFile(loc))
        return true;

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


bool CUDA_ASTVisitor::VisitTypeLoc(clang::TypeLoc typeLoc){

    clang::SourceLocation loc = typeLoc.getBeginLoc();
    if (!loc.isValid() || !context->getSourceManager().isInMainFile(loc))
        return true;

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


bool CUDA_ASTVisitor::VisitForStmt(clang::ForStmt *forStmt){

    clang::SourceLocation loc = forStmt->getForLoc();
    if (loc.isValid() && context->getSourceManager().isInMainFile(loc)) {
        if (isVisitorInsideKernel){
            targetExpressions.forStmts.push_back(forStmt);
        }
        else{
        // Check if the for loop contains a kernel launch in the host
            if (hasKernelLaunch(forStmt)){
                targetExpressions.kernelLaunchforStmts.push_back(forStmt);
            }
        }
    }

    return true;
}