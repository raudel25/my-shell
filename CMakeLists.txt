cmake_minimum_required(VERSION 3.22)
project(my_sh)

set(CMAKE_C_STANDARD 17)
SET( COMPILE_FLAGS "-lreadline" )
add_executable(my_sh src/main.c src/list.c src/execute.c src/utils.c src/builtin.c src/glist.c)
target_link_libraries(my_sh -lreadline)