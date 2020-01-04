cmake_minimum_required(VERSION 3.10)
include(../../../util.cmake)

message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")

add_executable(q_tech_dep main.cpp instruction_generator.cpp 
../../1_digital/quantum/tech_dep/event_queue_manager.cpp ../../1_digital/quantum/tech_dep/q_tech_dep.cpp
../../1_digital/quantum/tech_dep/timing_control_unit.cpp ../../1_digital/quantum/tech_dep/fast_conditional_execution.cpp)



target_link_libraries(q_tech_dep    SystemC::systemc lib_core lib_quantum_tech_ind)


include_directories(../../../lib/)
include_directories(../../0_core)
include_directories(../../1_digital/quantum/tech_ind)
include_directories(../../1_digital/quantum/tech_dep)