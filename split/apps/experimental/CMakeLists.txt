add_executable(DEPO_GSS dynamic_eco_gss.cpp)
target_link_libraries(DEPO_GSS PRIVATE eco ${COMMON_LIBS})
target_include_directories(DEPO_GSS PRIVATE ${CMAKE_SOURCE_DIR}/lib/eco/include)