# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.15)
project ("plasma")

#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sbe)
find_package(Java 1.8 COMPONENTS Runtime)
SET(SBE_TOOL_JAVA_CMD ${Java_JAVA_EXECUTABLE})
message(STATUS "JAVA_CMD=${SBE_TOOL_JAVA_CMD}")

# Include sub-projects.
add_subdirectory ("sbe")
add_subdirectory ("plasma")
