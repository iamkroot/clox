cmake_minimum_required(VERSION 3.14)
project(clox C)

set(CMAKE_C_STANDARD 11)

add_executable(clox main.c chunk.h common.h chunk.c memory.h memory.c)