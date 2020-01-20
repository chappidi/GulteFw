# CMakeList.txt : CMake project for plasma, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.13)

add_library(plasma_lib INTERFACE)
target_include_directories(plasma_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
# Add source to this project's executable.
add_executable (plasma "main.cpp" "plasma.h")
add_dependencies(plasma SBE_GEN)
target_link_libraries(plasma PUBLIC XYZ)

# TODO: Add tests and install targets if needed.
