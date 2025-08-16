cmake_minimum_required(VERSION 3.10)

project(phd CXX)
set(CMAKE_CXX_STANDARD 17)


# We assume that platform can have either XPU or GPU , but not both
# Hence users can build split with CUDA or with Level Zero but not both at the same time
if(WITH_XPU)
       add_definitions(-DWITH_XPU)
else()
      include_directories(/usr/local/cuda-12.8/include)

endif()


include(CMakeLists.txt.in)
find_package(yaml-cpp REQUIRED)

if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()

add_subdirectory(lib/eco)

add_dependencies(
    eco
    pcm
    gnuplot-iostream
    )

set(COMMON_LIBS
    ${CMAKE_CURRENT_BINARY_DIR}/pcm-src/libPCM.a
    m
    rt
    dl
    pthread
    boost_iostreams
    boost_system
    boost_filesystem
    boost_program_options
    yaml-cpp
    spdlog::spdlog
    )
if(DEFINED WITH_XPU)
       find_library(LIBZE_LOADER_LIBRARY NAMES ze_loader PATHS ${LIBZE_LOADER_PATH})
       set(COMMON_LIBS
               ${COMMON_LIBS}
               ${LIBZE_LOADER_LIBRARY}
               )

else()
       set(COMMON_LIBS
               ${COMMON_LIBS}
               /usr/local/cuda-12.8/targets/x86_64-linux/lib/stubs/libnvidia-ml.a
               #nvidia-ml
               cuda
               )
endif()

#TODO: Add suport in DEPO for XPU
if(NOT DEFINED WITH_XPU OR WITH_XPU STREQUAL "OFF" OR WITH_XPU STREQUAL "0")
    add_subdirectory(apps/DEPO)
endif()
add_subdirectory(apps/StEP)
add_subdirectory(apps/simple)
add_subdirectory(apps/experimental)

# workloads
add_custom_target(
    minibenchmarks
    ALL COMMAND make
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/minibenchmarks/openmp/
    )

if(WITH_XPU)
# unit tests
enable_testing()

add_executable(
test_xpu
tests/test_xpu.cpp
)
target_include_directories(test_xpu PRIVATE ${CMAKE_SOURCE_DIR}/lib/eco/include)
target_link_libraries(test_xpu eco ${COMMON_LIBS})
if(UNIX)
    target_link_libraries(test_xpu dl)
endif()

add_dependencies(
    test_xpu
    eco
    pcm
    )
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config.yaml
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
add_custom_command(TARGET test_xpu PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${CMAKE_SOURCE_DIR}/tests/gemm.spv
                        ${CMAKE_BINARY_DIR}/gemm.spv)
add_test(
    NAME test_xpu
    COMMAND test_xpu
    )
endif()
