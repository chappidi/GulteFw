# CMakeList.txt : CMake project for plasma, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)
set(SBE_JAR ${CMAKE_CURRENT_SOURCE_DIR}/../../sbe-tool-all.jar)
set(SBE_SCHEMA ${CMAKE_CURRENT_SOURCE_DIR}/plasma-schema.xml)

add_library(XYZ INTERFACE)
target_include_directories(XYZ INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(SBE_GEN
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/plasma_client
	COMMAND java -Dsbe.target.language=CPP -Dsbe.output.dir=${CMAKE_CURRENT_SOURCE_DIR} -jar ${SBE_JAR} ${SBE_SCHEMA}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/plasma-schema.xml"
)
# TODO: Add tests and install targets if needed.
