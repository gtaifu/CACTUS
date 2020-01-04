cmake_minimum_required(VERSION 3.0)
include(../../util.cmake)

set (CUR_LIB_NAME cactus)

message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")

find_package(PythonInterp 3.7 REQUIRED)
find_package(PythonLibs 3.7 REQUIRED)

add_executable(${CUR_LIB_NAME} quma_tb_base.cpp main.cpp tb_qvm.cpp)
target_link_libraries(${CUR_LIB_NAME} ${PYTHON_LIBRARIES} SystemC::systemc lib_core lib_quantum lib_classical lib_adi lib_quantumsim lib_qvm lib_QIcircuit)

target_include_directories(${CUR_LIB_NAME} PUBLIC ${PYTHON_INCLUDE_DIRS})
target_include_directories(${CUR_LIB_NAME} PUBLIC ../../lib/)
target_include_directories(${CUR_LIB_NAME} PUBLIC ../0_core/)
target_include_directories(${CUR_LIB_NAME} PUBLIC ../1_digital/cclight/)
target_include_directories(${CUR_LIB_NAME} PUBLIC ../1_digital/classical/)
target_include_directories(${CUR_LIB_NAME} PUBLIC ../1_digital/quantum/)
target_include_directories(${CUR_LIB_NAME} PUBLIC ../2_analog_digital_if/)
target_include_directories(${CUR_LIB_NAME} PUBLIC ../3_qubit_sim/quantumsim/)
target_include_directories(${CUR_LIB_NAME} PUBLIC ../3_qubit_sim/QIcircuit/)
target_include_directories(${CUR_LIB_NAME} PUBLIC ../4_qvm/)
