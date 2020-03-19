cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

include(ExternalProject)
ExternalProject_Add(
  RxCpp
  URL https://github.com/ReactiveX/RxCpp/archive/master.zip
#  GIT_REPOSITORY https://github.com/ReactiveX/RxCpp.git
#  GIT_TAG master
  PREFIX RxCpp
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  TEST_COMMAND ""
)
ExternalProject_Get_Property(RxCpp source_dir)
set(RXCPP_INCLUDE ${source_dir}/Rx/v2/src)
include_directories(${RXCPP_INCLUDE})
