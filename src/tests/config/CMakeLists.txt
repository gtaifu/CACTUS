message("${Green}Start processing ${CMAKE_CURRENT_LIST_FILE}...${ColorReset}")
include_directories(../../src/config)
include_directories(../../lib/)
link_directories(../../src/config)

add_executable(tb_config main.cpp)

# get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
# foreach(dir ${dirs})
#   message(STATUS "main inc_dir= '${dir}'")
# endforeach()


target_link_libraries(tb_config lib_config)
# message("${Blue}Finsihed processing ${CMAKE_CURRENT_LIST_FILE}.${ColorRest}")
