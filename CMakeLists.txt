project(cactus)

# The version number
set (CACTUS_VERSION_MAJOR 1)
set (CACTUS_VERSION_MINOR 1)
set (CACTUS_VERSION_PATCH 1)
set (CACTUS_RELEASE_DATE 2020.01.02)

# configure a header file to pass some of the CMake settings to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/src/cactus_ver.h.in"
  "${PROJECT_BINARY_DIR}/cactus_ver.h"
  )

# add the binary tree to the search path for include files so that we will find cactus_ver.h
include_directories("${PROJECT_BINARY_DIR}")


cmake_minimum_required(VERSION 3.10)

include(./util.cmake)

# required by fopen_s
set (CMAKE_CXX_STANDARD 11)

find_package(SystemCLanguage CONFIG REQUIRED)

message("${Green}-- SystemC_TARGET_ARCH: ${SystemC_TARGET_ARCH}${ColorReset}")
message("${Green}-- SystemC_CXX_STANDARD: ${SystemC_CXX_STANDARD}${ColorReset}")
message("${Green}-- SystemC_CXX_STANDARD_REQUIRED: ${SystemC_CXX_STANDARD_REQUIRED}${ColorReset}")

message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")

message(STATUS "Start adding subdirectories...")
add_subdirectory(src/0_core/)
add_subdirectory(src/1_digital/)
add_subdirectory(src/2_analog_digital_if/)
add_subdirectory(src/3_qubit_sim/)
add_subdirectory(src/4_qvm/)
add_subdirectory(src/5_tb/)
add_subdirectory(src/6_qvm_server/)
# add_subdirectory(src/tests)

message(STATUS "Finished adding subdirectories.")
