
add_executable( plasma_unit_test
	TestNewOrder.cpp
	)
target_link_libraries(plasma_unit_test gtest_main plasmalib test_common)

add_test( plasma_unit_test plasma_unit_test )
