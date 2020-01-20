
add_executable( unit_tests TestSuiteA1.cpp TestSuiteB1.cpp)
target_link_libraries(unit_tests gtest_main plasmalib)

add_test( unit_tests unit_tests )
