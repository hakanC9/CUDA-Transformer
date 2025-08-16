add_executable(CpuPowerMonitor cpu_power_monitor.cpp)
target_link_libraries(CpuPowerMonitor PRIVATE eco ${COMMON_LIBS})
target_include_directories(CpuPowerMonitor PRIVATE ${CMAKE_SOURCE_DIR}/lib/eco/include)

add_executable(SetCpuPowerLimit set_cpu_power_limit.cpp)
target_link_libraries(SetCpuPowerLimit PRIVATE eco ${COMMON_LIBS})
target_include_directories(SetCpuPowerLimit PRIVATE ${CMAKE_SOURCE_DIR}/lib/eco/include)
