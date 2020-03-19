cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(rxcpp-download NONE)

include(ExternalProject)

#execute_process(COMMAND wget https://github.com/ReactiveX/RxCpp/archive/master.zip -O RxCpp.zip)
#message(${CMAKE_CURRENT_SOURCE_DIR})
#message(${CMAKE_BINARY_DIR})

ExternalProject_Add(
  RxCpp
  GIT_REPOSITORY https://github.com/Reactive/RxCpp.git
  GIT_TAG master
#	URL https://github.com/ReactiveX/RxCpp/archive/RxCpp.zip
#	URL ${CMAKE_BINARY_DIR}/RxCpp.zip
#  PREFIX RxCpp
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  TEST_COMMAND ""
)

ExternalProject_Get_Property(RxCpp source_dir)
set(RXCPP_INCLUDE ${source_dir}/Rx/v2/src)
include_directories(${RXCPP_INCLUDE})