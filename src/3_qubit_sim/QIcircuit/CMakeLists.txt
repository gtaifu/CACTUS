cmake_minimum_required(VERSION 3.0)
include(../../../util.cmake)

message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")

set (CUR_LIB_NAME lib_QIcircuit)

#compile with /bigobj
if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif (WIN32)

# find the python interpreter at first to get a consistent version of Python libraries.
# See https://cmake.org/cmake/help/v3.11/module/FindPythonInterp.html
find_package(PythonInterp 3.7 REQUIRED)
find_package(PythonLibs 3.7 REQUIRED)
# message("${Green}PYTHONLIBS_FOUND: ${PYTHONLIBS_FOUND}${ColorReset}")
# message("${Green}PYTHON_LIBRARIES: ${PYTHON_LIBRARIES}${ColorReset}")
# message("${Green}PYTHON_INCLUDE_DIRS: ${PYTHON_INCLUDE_DIRS}${ColorReset}")
# message("${Green}PYTHONLIBS_VERSION_STRING: ${PYTHONLIBS_VERSION_STRING}${ColorReset}")
# message("${Green}PYTHON_EXECUTABLE: ${PYTHON_EXECUTABLE}${ColorReset}")

#Add an executable from all sources
set(SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE SOURCES "${SRC_PATH}/*.cpp")

add_library(${CUR_LIB_NAME} ${SOURCES})
target_link_libraries(${CUR_LIB_NAME} ${PYTHON_LIBRARIES} SystemC::systemc lib_core)

#include the python headers
target_include_directories(${CUR_LIB_NAME} PUBLIC ${PYTHON_INCLUDE_DIRS})
target_include_directories(${CUR_LIB_NAME} PUBLIC ../../../lib/)
target_include_directories(${CUR_LIB_NAME} PUBLIC ../../0_core/)
target_include_directories(${CUR_LIB_NAME} PUBLIC ../../2_analog_digital_if/)

#Copy the interface python file
CONFIGURE_FILE(${SRC_PATH}/interface_QIcircuit.py ${PROJECT_BINARY_DIR}/bin/interface_QIcircuit.py COPYONLY)
CONFIGURE_FILE(${SRC_PATH}/network_api/gates.py ${PROJECT_BINARY_DIR}/bin/gates.py COPYONLY)
CONFIGURE_FILE(${SRC_PATH}/network_api/qusim_server.py ${PROJECT_BINARY_DIR}/bin/qusim_server.py COPYONLY)
