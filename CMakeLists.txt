cmake_minimum_required(VERSION 3.13)
project(CUDATransformer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Can needed if the client code can not be compiled with clang++ 
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -fexceptions")

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Found CLANG ${CLANG_PACKAGE_VERSION}")

set(LLVM_DIR "/home/pars/Desktop/llvm-project")     # Needs to be configured for different systems

add_executable(CUDATransformer                      # Needs to be changed to add library (static/shared)
    main.cpp                                        # Needs to be removed for standalone (library) use
    src/CUDA_TransformerTool.cpp
    src/Transformer.cpp
    src/CUDA_ASTVisitor.cpp
    
    src/CUDA_Transform_FrontendActionFactory.cpp
    src/CUDA_Transform_FrontendAction.cpp
    src/CUDA_Transform_ASTConsumer.cpp
    
    src/CUDA_Analyze_FrontendActionFactory.cpp
    src/CUDA_Analyze_FrontendAction.cpp
    src/CUDA_Analyze_ASTConsumer.cpp

    src/Commands/RemoveSyncThread.cpp
    src/Commands/ReplaceSyncWithWarp.cpp
    src/Commands/ReplaceAtomicWithBlock.cpp
    src/Commands/ReplaceAtomicWithDirect.cpp
    src/Commands/ReplaceSyncThreadWithTile.cpp
    src/Commands/ReplaceSyncThreadWithActive.cpp
    src/Commands/KernelCallReduction.cpp
    src/Commands/RemoveIfElseBranches.cpp
    src/Commands/ChooseIfElseBranch.cpp
    src/Commands/ReplaceDoubleWithFloat.cpp
    src/Commands/LoopPerforation.cpp
)

target_include_directories(CUDATransformer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${LLVM_DIR}/build/include
        ${LLVM_DIR}/build/tools/clang/include
        ${LLVM_DIR}/clang/include
        ${LLVM_DIR}/llvm/include
)

target_link_libraries(CUDATransformer
    PUBLIC
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangFrontendTool
    clangSerialization
    clangTooling
    clangEdit
    ${LLVM_LIBS}    # Can be redundant but still safe to include since a lot of clang components initially needs llvm
)

