# Add source to this project's executable.
add_executable (plasma main.cpp)
target_link_libraries(plasma PUBLIC AERON plasmalib)

#add_custom_command(TARGET plasma POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "D:\\GIT_DIR_3RD\\aeron\\out\\build\\x64-Debug\\binaries\\aeron_driver.dll" $<TARGET_FILE_DIR:plasma>	)

# TODO: Add tests and install targets if needed.
