# CUDATransformer

CUDATransformer library* is a tool that is built on LLVM's [Libtooling](https://clang.llvm.org/docs/LibTooling.html) with the goal of transforming target CUDA files by applying various optimization passes.

### Dependencies

1. [LLVM & Clang](https://llvm.org/) for the tool itself.
2. [spdlog](https://github.com/gabime/spdlog) and [CUDA Toolkit](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/) for DEPO tool.
3. [DEPO Tool](https://projects.task.gda.pl/akrz/split) for obtaining various metrics like power consumption.
4. [Ninja build system](https://ninja-build.org) for building llvm faster. (Not required, "Unix Makefiles" can be used as generator type)

## Building LLVM

1. Clone the llvm-project. For faster installation, shallow copy is suggested.

    ```bash
    git clone --depth 1 https://github.com/llvm/llvm-project.git
    ```

2. Create and navigate to build directory
    ```bash
    mkdir llvm-project/build && cd llvm-project/build
    ```

3. Configure cmake. Suggested commands:
    ```bash
    cmake -S ../llvm -B. -G "Ninja" \
    -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
    -DCMAKE_BUILD_TYPE=Release \ 
    -DLLVM_ABI_BREAKING_CHECKS=FORCE_OFF \
    -DLLVM_ENABLE_RTTI=ON \
    -DLLVM_LINK_LLVM_DYLIB = ON
    ```
    Last 3 options are mostly related to statically linking the tool. Can be omitted for faster builds at the moment.

4. Build llvm.
    ```bash
    ninja
    ```

## Building DEPO Tool For Metrics

1. Build spdlog. Do not forget to add to path.

2. Install dependencies
    ```bash
    sudo apt-get install gnuplot libboost-all-dev graphviz 
    ```

3. Clone split.
    ```bash
    git clone https://projects.task.gda.pl/akrz/split.git
    ```

4. Change the line
    ```cmake
    include_directories(/usr/local/cuda/include)
    ```
    then build the tool
    ```bash
    mkdir split/build && cd split/build
    cmake ..
    make
    ```

## Building CUDATransformer

The tool is built [out-of-tree](https://www.youtube.com/watch?v=7wOU7csj1ME). So no need to configure cmake files in clang-tools-extra directory. Simply;
1.  ```bash
    git clone https://github.com/hakanC9/CUDA-Transformer.git
    mkdir CUDA-Transformer/build && cd CUDA-Transformer/build
    cmake ..
    make -j $(nproc)
    ```

# Using the tool

A simple main.cpp file is provided to represent functionality of the tool. Do not forget to modify CMakeLists.txt and energy1.sh files as they use hard-coded paths (subject to change). For each terminal session build.sh script under split/profiling_injection/ should be run with super user privileges for depo tool to collect metrics.

## Keys in config.txt

1. **project_path:** Absolute path to the target project

2. **resource_dir:** Absolute path to the clang installation (***/path/to/llvm-project/build/lib/clang/21***)

3. **compilation_database_options:** Semi-column seperated options to apply generating compilation database (***-std=c++17;--cuda-gpu-arch=sm_75***)

4. **compile_options:** Semi-column seperated options to apply for compilation process (***-w;-Wno-deprecated-gpu-targets***)

5. **includes:** Semi-column seperated absolute paths to includes directories (***/usr/local/cuda/include***)

6. **executable:** Name of the executable file without file extension (***main.cpp -> main***)

7. **run_options:** Semi-column seperated options to apply for executing
