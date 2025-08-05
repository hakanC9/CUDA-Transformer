# grappolo

set( GPU_HEADERS
  ${CMAKE_SOURCE_DIR}/gpu_graph
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set( GPUFOLDER_SRC
     ${CMAKE_SOURCE_DIR}/${GPUFOLDER}/gpu_main.cpp
     ${CMAKE_SOURCE_DIR}/${GPUFOLDER}/graphHOST.cpp

     ${CMAKE_SOURCE_DIR}/${GPUFOLDER}/aggregateCommunity.cu
     ${CMAKE_SOURCE_DIR}/${GPUFOLDER}/assignGraph.cu
     ${CMAKE_SOURCE_DIR}/${GPUFOLDER}/binWiseGaussSeidel.cu
     ${CMAKE_SOURCE_DIR}/${GPUFOLDER}/communityGPU.cu
     ${CMAKE_SOURCE_DIR}/${GPUFOLDER}/computeModularity.cu
     ${CMAKE_SOURCE_DIR}/${GPUFOLDER}/computeTime.cu
     ${CMAKE_SOURCE_DIR}/${GPUFOLDER}/coreutility.cu
     ${CMAKE_SOURCE_DIR}/${GPUFOLDER}/gatherInformation.cu
     ${CMAKE_SOURCE_DIR}/${GPUFOLDER}/graphGPU.cu
     ${CMAKE_SOURCE_DIR}/${GPUFOLDER}/independentKernels.cu
     #     ${CMAKE_SOURCE_DIR}/${GPUFOLDER}/preprocessing.cu
   )

CUDA_ADD_EXECUTABLE( gpu_graph 
  ${GPUFOLDER_SRC}
  OPTIONS 
  -gencode=arch=compute_60,code=sm_60
  -gencode=arch=compute_70,code=sm_70
  -gencode=arch=compute_75,code=sm_75
  -gencode=arch=compute_80,code=sm_80
)

target_include_directories( gpu_graph 
     PUBLIC 
     ${GPU_HEADERS}
     ${IO_HEADERS}
)

target_link_libraries( gpu_graph 
     inout
     util
     ${CUDA_LIBRARIES}
)

install( TARGETS
  gpu_graph
  #DESTINATION lib
  DESTINATION bin
)

install( FILES
  ${GPU_HEADERS}/commonconstants.h
  ${GPU_HEADERS}/communityGPU.h
  ${GPU_HEADERS}/devconstants.h
  ${GPU_HEADERS}/graphGPU.h
  ${GPU_HEADERS}/graphHOST.h
  ${GPU_HEADERS}/hashitem.h
  ${GPU_HEADERS}/hostconstants.h
  ${GPU_HEADERS}/myutility.h
  ${GPU_HEADERS}/openaddressing.h
  DESTINATION include
)

