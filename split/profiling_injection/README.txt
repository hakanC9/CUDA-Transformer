libinjection_2.so
    * Expands on the injection_1 sample to add CUPTI Callback and Profiler API calls
    * Registers callbacks for cuLaunchKernel and context creation.  This will be
      sufficient for many target applications, but others may require other launches
      to be matched, eg cuLaunchCoooperativeKernel or cuLaunchGrid.  See the Callback
      API for all possible kernel launch callbacks.
    * Creates a Profiler API configuration for each context in the target (using the
      context creation callback).  The Profiler API is configured using Kernel Replay
      and Auto Range modes with a configurable number of kernel launches within a pass.
    * The kernel launch callback is used to track how many kernels have launched in
      a given context's current pass, and if the pass reached its maximum count, it
      prints the metrics and starts a new pass.
    * At exit, any context with an unprocessed metrics (any which had partially
      completed a pass) print their data.
    * This library links in the profilerHostUtils library which may be built from the
      cuda/extras/CUPTI/samples/extensions/src/profilerhost_util/ directory

To use the injection library, set LD_LIBRARY_PATH and LD_PRELOAD to include that library
when you launch the target application:

env LD_PRELOAD=./libinjection_2.so LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:`pwd` ./simple_target

sudo CUDA_INJECTION64_PATH=~/workspace/eco/profiling_injection/libinjection_2.so ~/workspace/eco/build/eco_gpu ./cublasgemm-benchmark/gemm 4 16