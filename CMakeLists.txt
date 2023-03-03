cmake_minimum_required(VERSION 3.10)
project(file_manager)

set(SOURCE_FILES main.c errors.c functions.c colors.c window.c)

add_library(extensions SHARED extensions.c)
set_target_properties(extensions PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(extensions dl)

add_executable(file_manager ${SOURCE_FILES})

target_link_libraries(file_manager ncurses extensions)

set_target_properties(file_manager PROPERTIES
  ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}:${LD_LIBRARY_PATH}"
)
