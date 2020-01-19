cmake_minimum_required (VERSION 3.15)

add_executable( unit_tests tests.cpp)

target_link_libraries(unit_tests gtest_main)
add_test( unit_tests unit_tests )
