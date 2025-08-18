cmake_minimum_required(VERSION 3.13)
project(CUDATransformer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Can needed if the client code can not be compiled with clang++ 
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -fexceptions")

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(Boost REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Found CLANG ${CLANG_PACKAGE_VERSION}")

set(LLVM_DIR "/home/pars/Desktop/llvm-project")     # Needs to be configured for different systems


option ( TEST_GRAPPOLO "Test code for grappolo project" ON)
option ( TEST_POLYBENCH "Test code for polybench's convolution2d project" ON)
option ( TEST_GARDENIA "Test code for gardenia's bc_linear_base.cu file" ON)

set ( 
    TOOL_INCLUDES 
    ${CMAKE_SOURCE_DIR}/include
    )

set ( 
    LLVM_INCLUDES         
    ${LLVM_DIR}/build/include
    ${LLVM_DIR}/build/tools/clang/include
    ${LLVM_DIR}/clang/include
    ${LLVM_DIR}/llvm/include
    )


set (
    TOOL_SOURCES
    ${CMAKE_SOURCE_DIR}/src/CUDA_TransformerTool.cpp
    ${CMAKE_SOURCE_DIR}/src/Transformer.cpp
    ${CMAKE_SOURCE_DIR}/src/CUDA_ASTVisitor.cpp
    
    ${CMAKE_SOURCE_DIR}/src/CUDA_Transform_FrontendActionFactory.cpp
    ${CMAKE_SOURCE_DIR}/src/CUDA_Transform_FrontendAction.cpp
    ${CMAKE_SOURCE_DIR}/src/CUDA_Transform_ASTConsumer.cpp
    
    ${CMAKE_SOURCE_DIR}/src/CUDA_Analyze_FrontendActionFactory.cpp
    ${CMAKE_SOURCE_DIR}/src/CUDA_Analyze_FrontendAction.cpp
    ${CMAKE_SOURCE_DIR}/src/CUDA_Analyze_ASTConsumer.cpp

    ${CMAKE_SOURCE_DIR}/src/Commands/RemoveSyncThread.cpp
    ${CMAKE_SOURCE_DIR}/src/Commands/ReplaceSyncWithWarp.cpp
    ${CMAKE_SOURCE_DIR}/src/Commands/ReplaceAtomicWithBlock.cpp
    ${CMAKE_SOURCE_DIR}/src/Commands/ReplaceAtomicWithDirect.cpp
    ${CMAKE_SOURCE_DIR}/src/Commands/ReplaceSyncThreadWithTile.cpp
    ${CMAKE_SOURCE_DIR}/src/Commands/ReplaceSyncThreadWithActive.cpp
    ${CMAKE_SOURCE_DIR}/src/Commands/KernelCallReduction.cpp
    ${CMAKE_SOURCE_DIR}/src/Commands/RemoveIfElseBranches.cpp
    ${CMAKE_SOURCE_DIR}/src/Commands/ChooseIfElseBranch.cpp
    ${CMAKE_SOURCE_DIR}/src/Commands/ReplaceDoubleWithFloat.cpp
    ${CMAKE_SOURCE_DIR}/src/Commands/LoopPerforation.cpp

    ${CMAKE_SOURCE_DIR}/src/Helper/ConfigParser.cpp
    ${CMAKE_SOURCE_DIR}/src/Helper/DefaultEvaluator.cpp
    ${CMAKE_SOURCE_DIR}/src/Helper/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/Helper/FileHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/Helper/QuickResponse.cpp
    ${CMAKE_SOURCE_DIR}/src/Helper/Timer.cpp
    )


add_library(CUDATransformerLib 
        STATIC
        ${TOOL_SOURCES}
)


target_include_directories(CUDATransformerLib
    PUBLIC
        ${TOOL_INCLUDES}
        ${LLVM_INCLUDES}
)

target_link_libraries(CUDATransformerLib
    PUBLIC
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangFrontendTool
    clangSerialization
    clangTooling
    clangEdit
    Boost::boost
    #${LLVM_LIBS}    # Can be redundant but still safe to include since a lot of clang components initially needs llvm
)

add_executable(CUDATransformerMain 
    main.cpp
    ) 

target_link_libraries(CUDATransformerMain
    PRIVATE 
    CUDATransformerLib
    )

add_custom_command(TARGET CUDATransformerMain POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config.txt
        $<TARGET_FILE_DIR:CUDATransformerMain>/config.txt
)

add_custom_command(TARGET CUDATransformerMain POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/energy.sh
        $<TARGET_FILE_DIR:CUDATransformerMain>/energy.sh
)

IF ( TEST_GRAPPOLO )
    MESSAGE ( "Test code for grappolo is set to be built" )
    add_subdirectory ( tests/grappolo )
ENDIF ( TEST_GRAPPOLO )


IF ( TEST_POLYBENCH )
    MESSAGE ( "Test code for polybench is set to be built" )
    add_subdirectory ( tests/polybench )
ENDIF ( TEST_POLYBENCH )

IF ( TEST_GARDENIA )
    MESSAGE ( "Test code for gardenia is set to be built" )
    add_subdirectory ( tests/gardenia )
ENDIF ( TEST_GARDENIA )