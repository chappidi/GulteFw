cmake_minimum_required (VERSION 3.15)
project (plasma CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/googletest.cmake)
fetch_googletest(
    ${PROJECT_SOURCE_DIR}/cmake
    ${PROJECT_BINARY_DIR}/googletest
    )

add_subdirectory (sbe)
add_subdirectory (plasma_lib)
add_subdirectory (plasma)
enable_testing()
add_subdirectory (test)
add_subdirectory (V1/tests)
add_subdirectory (V2/tests)
