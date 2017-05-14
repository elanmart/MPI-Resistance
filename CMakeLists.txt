cmake_minimum_required(VERSION 3.5)
project(PR)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp src/node.c src/node.h src/utils.c)
include_directories(usr/include/mpi/)

add_executable(PR ${SOURCE_FILES})