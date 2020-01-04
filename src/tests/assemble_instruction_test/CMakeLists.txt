cmake_minimum_required(VERSION 3.10)
include(../../../util.cmake)

message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")

add_executable(assemle_test main.cpp instruction_generator.cpp config_setting.cpp clock_counter.cpp
../../2_analog_digital_if/adi.cpp ../../2_analog_digital_if/analog_digital_convert.cpp
../../2_analog_digital_if/msmt_result_gen.cpp )



target_link_libraries(assemle_test    SystemC::systemc lib_core lib_quantum_tech_ind lib_quantum_tech_dep)


include_directories(../../../lib/)
include_directories(../../0_core)
include_directories(../../1_digital/quantum/tech_ind)
include_directories(../../1_digital/quantum/tech_dep)
include_directories(../../2_analog_digital_if)