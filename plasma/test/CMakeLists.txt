cmake_minimum_required (VERSION 3.13)

add_executable( unit_tests TestSuiteA1.cpp TestSuiteB1.cpp)
add_dependencies(unit_tests SBE_GEN)
target_link_libraries(unit_tests gtest_main XYZ plasma_lib)
add_test( unit_tests unit_tests )
