cmake_minimum_required(VERSION 3.10)
include(../../../util.cmake)

message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")

# add_executable(tb_core main.cpp)
# add_executable(counter_tb test_cycle_counter.cpp)
# add_executable(counter_sink_tb test_cycle_counter_sink.cpp)
add_executable(tb_util test_num_util.cpp)
# add_executable(tb_spdlog test_spdlog.cpp)
# add_executable(tb_q_data_type test_q_data_type.cpp)
add_executable(tb_config_reader test_config_reader.cpp)

# target_link_libraries(tb_core           SystemC::systemc lib_core)
# target_link_libraries(counter_tb        SystemC::systemc lib_core)
# target_link_libraries(counter_sink_tb   SystemC::systemc lib_core)
target_link_libraries(tb_util           lib_core)
# target_link_libraries(tb_q_data_type    SystemC::systemc lib_core)
target_link_libraries(tb_config_reader    SystemC::systemc lib_core)


include_directories(../../../lib/)
include_directories(../../0_core)
