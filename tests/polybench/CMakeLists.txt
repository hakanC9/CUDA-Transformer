add_executable(CUDATransformer_polybench polybench_main.cpp)



target_link_libraries(CUDATransformer_polybench
    PUBLIC
    CUDATransformerLib
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

add_custom_command(TARGET CUDATransformer_polybench POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config.txt
        $<TARGET_FILE_DIR:CUDATransformer_polybench>/config.txt
)

add_custom_command(TARGET CUDATransformer_polybench POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/energy.sh
        $<TARGET_FILE_DIR:CUDATransformer_polybench>/energy.sh
)