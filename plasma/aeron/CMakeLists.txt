cmake_minimum_required (VERSION 3.15)

#add_library(aeron_driver SHARED IMPORTED)
#set_target_properties(aeron_driver PROPERTIES
#	IMPORTED_IMPLIB "D:\\GIT_DIR_3RD\\aeron\\out\\build\\x64-Debug\\lib\\aeron_driver.lib"
#	INTERFACE_INCLUDE_DIRECTORIES "D:\\GIT_DIR_3RD\\aeron\\aeron-driver\\src\\main\\c"
#	)

add_library(aeron_client STATIC IMPORTED)
set_target_properties(aeron_client PROPERTIES
	IMPORTED_LOCATION "D:\\GIT_DIR_3RD\\aeron\\out\\build\\x64-Debug\\lib\\aeron_client.lib"
	INTERFACE_INCLUDE_DIRECTORIES "D:\\GIT_DIR_3RD\\aeron\\aeron-client\\src\\main\\cpp"
	)

add_library(AERON STATIC AeronClient.cpp)
target_include_directories(AERON PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AERON PUBLIC aeron_client)
#target_link_libraries(AERON PUBLIC aeron_driver aeron_client)
