add_executable(DEPO dynamic_energy_performance_optimizer.cpp)
target_link_libraries(DEPO PRIVATE eco ${COMMON_LIBS})
target_include_directories(DEPO PRIVATE ${CMAKE_SOURCE_DIR}/lib/eco/include)