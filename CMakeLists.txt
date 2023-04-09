cmake_minimum_required(VERSION 3.22)
project(shell C)

set(CMAKE_C_STANDARD 17)
SET( COMPILE_FLAGS "-lreadline" )
add_executable(main.c decode.h decode.c execute.h execute.c main.c builtin.h builtin.c list.c list.h glist.h glist.c)
target_link_libraries(main.c -lreadline)