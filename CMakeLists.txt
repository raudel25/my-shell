cmake_minimum_required(VERSION 3.22)
project(shell C)

set(CMAKE_C_STANDARD 17)

add_executable(main.c decod.h decod.c execute.h execute.c main.c builtin.h builtin.c list.c list.h glist.h glist.c)
