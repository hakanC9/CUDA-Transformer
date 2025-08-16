set(SOURCES
    src/eco.cpp
    src/params_config.cpp
    src/plot_builder.cpp
    src/device_state.cpp
    src/data_structures/data_filter.cpp
    src/data_structures/final_power_and_perf_result.cpp
    src/data_structures/power_and_perf_result.cpp
    src/data_structures/results_container.cpp
    src/devices/intel_device.cpp
    src/power_interfaces/msr.cpp
    src/power_interfaces/Rapl.cpp
)


if(DEFINED WITH_XPU)
  set(SOURCES
         ${SOURCES}
        src/devices/xpu_device.cpp
               )
else()
  set(SOURCES
         ${SOURCES}
        src/devices/cuda_device.cpp
               )
endif()

add_library(eco
  ${SOURCES}
)

target_include_directories(eco PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
