cmake_minimum_required(VERSION 3.10)
include(../../../util.cmake)


message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")


link_directories(../../src/config/build)
link_directories(../../src/core/build)
link_directories(../../src/classical/build)

# add_executable(tb_icache test_icache.cpp)
# target_link_libraries(tb_icache SystemC::systemc lib_config lib_classical)

# add_executable(tb_ic_slice test_ic_slice.cpp)
# target_link_libraries(tb_ic_slice SystemC::systemc lib_config lib_classical)

# add_executable(tb_meas_reg_file test_meas_reg_file.cpp)
# target_link_libraries(tb_meas_reg_file SystemC::systemc lib_config lib_classical)

add_executable(classical_top test_top.cpp ../../1_digital/classical/classical_top.cpp
../../1_digital/classical/classical_pipeline.cpp ../../1_digital/classical/classical_fetch.cpp
../../1_digital/classical/classical_decode.cpp ../../1_digital/classical/classical_execute.cpp
../../1_digital/classical/icache_top.cpp ../../1_digital/classical/icache_rtl.cpp 
../../1_digital/classical/icache_slice.cpp ../../1_digital/classical/meas_reg_file_top.cpp 
../../1_digital/classical/meas_reg_file_slice.cpp ../../1_digital/classical/meas_reg_file_rtl.cpp)

target_link_libraries(classical_top SystemC::systemc lib_core)
#target_link_libraries(classical_top SystemC::systemc lib_core lib_classical)


include_directories(../../../lib/)
#include_directories(../../config)
include_directories(../../0_core)
include_directories(../../1_digital/classical)
# message("${Blue}Finsihed processing ${CMAKE_CURRENT_LIST_FILE}.${ColorRest}")
