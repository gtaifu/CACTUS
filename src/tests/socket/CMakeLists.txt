cmake_minimum_required(VERSION 3.10)
include(../../../util.cmake)

message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")

add_executable(socket_test main.cpp
../../6_qvm_server/socket.cpp )



target_link_libraries(socket_test   SystemC::systemc lib_core qvm_server)

include_directories(../../../lib/)
include_directories(../../0_core)
include_directories(../../6_qvm_server/)