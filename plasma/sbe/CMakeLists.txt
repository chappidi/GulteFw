# CMakeList.txt : CMake project for plasma, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)
set(SBE_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/plasma_client/NewOrderSingle.h")
set(SBE_JAR ${CMAKE_CURRENT_SOURCE_DIR}/../../sbe-tool-all.jar)
set(SBE_SCHEMA ${CMAKE_CURRENT_SOURCE_DIR}/plasma-schema.xml)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES plasma_client)

add_custom_command(OUTPUT ${SBE_HEADERS} 
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/plasma_client
	COMMAND java -Dsbe.target.language=CPP -Dsbe.output.dir=${CMAKE_CURRENT_SOURCE_DIR} -jar ${SBE_JAR} ${SBE_SCHEMA}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/plasma-schema.xml"
)

#add_custom_target(libfoo_untar DEPENDS ${SBE_HEADERS})
add_library(XYZ INTERFACE)

add_custom_target(SBE_GEN
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/plasma_client
	COMMAND java -Dsbe.target.language=CPP -Dsbe.output.dir=${CMAKE_CURRENT_SOURCE_DIR} -jar ${SBE_JAR} ${SBE_SCHEMA}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/plasma-schema.xml"
)
target_include_directories(XYZ INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
# TODO: Add tests and install targets if needed.
