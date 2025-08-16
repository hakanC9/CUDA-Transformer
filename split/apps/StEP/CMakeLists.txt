add_executable(StEP static_energy_profiler.cpp)
target_link_libraries(StEP PRIVATE eco ${COMMON_LIBS})
target_include_directories(StEP PRIVATE ${CMAKE_SOURCE_DIR}/lib/eco/include)