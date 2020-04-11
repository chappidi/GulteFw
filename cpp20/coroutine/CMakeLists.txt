cmake_minimum_required (VERSION 3.15 FATAL_ERROR)

if (MSVC_VERSION GREATER_EQUAL "1900")
	add_compile_options("/await")
endif()

add_executable(coroutine_test "basic_test.cpp")
target_link_libraries(coroutine_test gtest_main)
add_test( coroutine_test coroutine_test )