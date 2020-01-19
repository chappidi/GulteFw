cmake_minimum_required (VERSION 3.15)

add_executable( unit_tests tests.cpp)
add_dependencies(unit_tests SBE_GEN)
target_link_libraries(unit_tests gtest_main XYZ plasma_lib)
add_test( unit_tests unit_tests )
