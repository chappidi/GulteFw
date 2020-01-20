set(LIB_NAME plasmalib)

add_library(${LIB_NAME} STATIC plasma.cpp)
target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${LIB_NAME} PUBLIC SBE)
