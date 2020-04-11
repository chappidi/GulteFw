cmake_minimum_required (VERSION 3.15 FATAL_ERROR)

if (MSVC_VERSION GREATER_EQUAL "1900")
    add_compile_options("/experimental:module")
endif()
add_custom_target(HW 
    COMMAND cl /EHsc /std:c++latest /MD /c /experimental:module ${CMAKE_CURRENT_SOURCE_DIR}/helloworld.ixx
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(hwApp  ALL
   COMMAND cl /EHsc /std:c++latest /MD /experimental:module /module:reference helloworld.ifc helloworld.obj ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_dependencies(hwApp HW)
