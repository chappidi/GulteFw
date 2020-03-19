cmake_minimum_required (VERSION 3.15 FATAL_ERROR)
project ("qep" CXX)
if (MSVC_VERSION GREATER_EQUAL "1900")
	add_compile_options("/std:c++latest")
endif()

#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include(cmake/boost.cmake)
include(cmake/googletest.cmake)
include(cmake/rxcpp.cmake)
fetch_googletest(
    ${PROJECT_SOURCE_DIR}/cmake
    ${PROJECT_BINARY_DIR}/googletest
    )

# Include sub-projects.
add_subdirectory ("base_freq")
enable_testing()
add_subdirectory ("test")
add_subdirectory ("fiber")
add_subdirectory ("RxMain")
