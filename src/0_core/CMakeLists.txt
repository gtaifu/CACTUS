cmake_minimum_required(VERSION 3.0)

include(../../util.cmake)

set (CUR_LIB_NAME lib_core)

message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")

set(SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE SOURCES "${SRC_PATH}/*.cpp")

add_library(${CUR_LIB_NAME} ${SOURCES})
target_link_libraries(${CUR_LIB_NAME} SystemC::systemc)

target_include_directories(${CUR_LIB_NAME} PUBLIC ../../lib/)


#Copy the interface python file
CONFIGURE_FILE(${SRC_PATH}/../../test_files/log_levels.json ${PROJECT_BINARY_DIR}/bin/log_levels.json COPYONLY)
