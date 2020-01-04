project(test_core CXX)
cmake_minimum_required(VERSION 3.10)
message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")

find_package(SystemCLanguage CONFIG REQUIRED)
set (CMAKE_CXX_STANDARD ${SystemC_CXX_STANDARD} CACHE STRING "C++ standard to build all targets. Supported values are 98, 11, and 14.")
set (CMAKE_CXX_STANDARD_REQUIRED ${SystemC_CXX_STANDARD_REQUIRED} CACHE BOOL
    "The with CMAKE_CXX_STANDARD selected C++ standard is a requirement.")

link_directories(../../src/config/build)
link_directories(../../src/core/build)
link_directories(../../src/quantum/build)

# add_executable(tb_mask_reg test_mask_reg.cpp ../../src/quantum/tech_ind/mask_reg_file.cpp)
# add_executable(tb_mask_reg test_mask_reg.cpp)
# add_executable(tb_q_pipe_ind test_ind.cpp)
add_executable(tb_q_pipe test_quantum_pipeline.cpp)

# target_link_libraries(tb_mask_reg SystemC::systemc lib_core lib_config)
# target_link_libraries(tb_mask_reg SystemC::systemc lib_core lib_config lib_quantum)
# target_link_libraries(tb_q_pipe_ind SystemC::systemc lib_core lib_config lib_quantum)
target_link_libraries(tb_q_pipe SystemC::systemc lib_core lib_config lib_quantum)

include_directories(../../lib/)
include_directories(../../src/config)
include_directories(../../src/core)
include_directories(../../src/quantum/)

# message("${Blue}Finsihed processing ${CMAKE_CURRENT_LIST_FILE}.${ColorRest}")
