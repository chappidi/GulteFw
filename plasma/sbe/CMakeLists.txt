set(SBE_JAR ${CMAKE_CURRENT_SOURCE_DIR}/../../sbe-all-1.17.0.jar)
set(SBE_SCHEMA ${CMAKE_CURRENT_SOURCE_DIR}/plasma-schema.xml)

add_custom_target(GEN_HDR
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/plasma_client
	COMMAND java -Dsbe.target.language=CPP -Dsbe.output.dir=${CMAKE_CURRENT_SOURCE_DIR} -jar ${SBE_JAR} ${SBE_SCHEMA}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/plasma-schema.xml"
)

add_library(SBE INTERFACE)
target_include_directories(SBE INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(SBE GEN_HDR)

# TODO: Add tests and install targets if needed.
