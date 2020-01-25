
add_executable( unit_tests TestSuiteA1.cpp 
	TestSuiteB1.cpp TestSuiteB2.cpp 
	TestSuiteC1.cpp 
	TestSuiteD1.cpp TestSuiteD2.cpp
	TestSuiteG1.cpp)
target_link_libraries(unit_tests gtest_main plasmalib)

add_test( unit_tests unit_tests )
