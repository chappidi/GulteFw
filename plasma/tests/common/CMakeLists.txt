cmake_minimum_required (VERSION 3.15)

add_library(test_common INTERFACE)
target_include_directories(test_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
