
add_executable( thor_unit_test
	TestChildCancel.cpp
	TestChildReplace.cpp
	TestParentChild.cpp
	)
target_link_libraries(thor_unit_test gtest_main plasmalib test_common)

add_test( thor_unit_test thor_unit_test )
