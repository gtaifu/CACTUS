cmake_minimum_required(VERSION 3.10)
include(../../../util.cmake)

message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")

add_executable(q_decoder main.cpp ../../1_digital/quantum/tech_ind/q_decoder.cpp instruction_generator.cpp
../../1_digital/quantum/tech_ind/q_decoder_bin.cpp ../../1_digital/quantum/tech_ind/q_decoder_asm.cpp)

target_link_libraries(q_decoder    SystemC::systemc lib_core)


include_directories(../../../lib/)
include_directories(../../0_core)
include_directories(../../1_digital/quantum/tech_ind)