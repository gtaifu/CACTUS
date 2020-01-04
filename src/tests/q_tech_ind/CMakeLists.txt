cmake_minimum_required(VERSION 3.10)
include(../../../util.cmake)

message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")

add_executable(q_tech_ind main.cpp instruction_generator.cpp 
../../1_digital/quantum/tech_ind/q_decoder_bin.cpp ../../1_digital/quantum/tech_ind/q_decoder_asm.cpp
../../1_digital/quantum/tech_ind/q_decoder.cpp ../../1_digital/quantum/tech_ind/q_tech_ind.cpp 
../../1_digital/quantum/tech_ind/operation_combiner.cpp ../../1_digital/quantum/tech_ind/addr_mask_decoder.cpp 
../../1_digital/quantum/tech_ind/mask_reg_file.cpp ../../1_digital/quantum/tech_ind/op_decoder.cpp
../../1_digital/quantum/tech_ind/vliw_pipelane.cpp ../../1_digital/quantum/tech_ind/meas_issue_gen.cpp)



target_link_libraries(q_tech_ind    SystemC::systemc lib_core)


include_directories(../../../lib/)
include_directories(../../0_core)
include_directories(../../1_digital/quantum/tech_ind)