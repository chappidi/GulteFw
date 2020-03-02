cmake_minimum_required (VERSION 3.15)
project (plasma CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
IF(WIN32 AND MSVC)
    ADD_COMPILE_DEFINITIONS(_HAS_STD_BYTE=0)
ENDIF()

include(cmake/googletest.cmake)
fetch_googletest(
    ${PROJECT_SOURCE_DIR}/cmake
    ${PROJECT_BINARY_DIR}/googletest
    )

add_subdirectory (sbe)
add_subdirectory (aeron)
add_subdirectory (plasma_lib)
add_subdirectory (plasma)
enable_testing()
add_subdirectory (tests)
