# CMakeList.txt : CMake project for plasma, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

# Add source to this project's executable.
add_executable (plasma "main.cpp" "plasma.h")
add_dependencies(plasma SBE_GEN)
target_link_libraries(plasma PUBLIC XYZ)

# TODO: Add tests and install targets if needed.
