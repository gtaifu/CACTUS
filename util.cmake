  string(ASCII 27 ESCAPE)
  set(ColorReset "${ESCAPE}[0m")
  set(ColorBold  "${ESCAPE}[1m")
  set(Red         "${ESCAPE}[31m")
  set(Green       "${ESCAPE}[32m")
  set(Yellow      "${ESCAPE}[33m")
  set(Blue        "${ESCAPE}[34m")
  set(Magenta     "${ESCAPE}[35m")
  set(Cyan        "${ESCAPE}[36m")
  set(White       "${ESCAPE}[37m")
  set(BoldRed     "${ESCAPE}[1;31m")
  set(BoldGreen   "${ESCAPE}[1;32m")
  set(BoldYellow  "${ESCAPE}[1;33m")
  set(BoldBlue    "${ESCAPE}[1;34m")
  set(BoldMagenta "${ESCAPE}[1;35m")
  set(BoldCyan    "${ESCAPE}[1;36m")
  set(BoldWhite   "${ESCAPE}[1;37m")

# CPP standard 11 is required by CACTUS
set (CMAKE_CXX_STANDARD 11)

# SystemC is widely used
find_package(SystemCLanguage CONFIG REQUIRED)
set (CMAKE_CXX_STANDARD ${SystemC_CXX_STANDARD} CACHE STRING "C++ standard to build all targets. Supported values are 98, 11, and 14.")
set (CMAKE_CXX_STANDARD_REQUIRED ${SystemC_CXX_STANDARD_REQUIRED} CACHE BOOL
    "The with CMAKE_CXX_STANDARD selected C++ standard is a requirement.")

# all static library will be found at a single place
link_directories(${PROJECT_BINARY_DIR}/lib)

# output directories should be specified before add_library or add_executable
# First for the generic no-config case (e.g. with mingw)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
# Second, for multi-config builds (e.g. msvc)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/lib)
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
