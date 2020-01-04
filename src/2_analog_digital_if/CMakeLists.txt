cmake_minimum_required(VERSION 3.0)
include(../../util.cmake)

set (CUR_LIB_NAME lib_adi)

message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")

set(SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE SOURCES "${SRC_PATH}/*.cpp")

add_library(${CUR_LIB_NAME} ${SOURCES})
target_link_libraries(${CUR_LIB_NAME} SystemC::systemc lib_core)

target_include_directories(${CUR_LIB_NAME} PUBLIC ../../lib/)
target_include_directories(${CUR_LIB_NAME} PUBLIC ../0_core/)
