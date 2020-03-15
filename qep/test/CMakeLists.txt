
add_executable( qep_unit_test
	test_qep.cpp
	speed_test.cpp
	)
target_link_libraries(qep_unit_test gtest_main base_freq)

add_test( qep_unit_test qep_unit_test )