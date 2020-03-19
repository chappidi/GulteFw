
add_executable( boost_fiber_test
	fiber_test.cpp
	)
target_link_libraries(boost_fiber_test gtest_main ${Boost_LIBRARIES})

add_test( boost_fiber_test boost_fiber_test )