# CMakeList.txt : CMake project for qep, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

if (MSVC_VERSION GREATER_EQUAL "1900")
	add_compile_options("/std:c++latest")
endif()
# Add source to this project's executable.
add_library (base_freq INTERFACE)
target_include_directories(base_freq INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# TODO: Add tests and install targets if needed.
