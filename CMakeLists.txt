cmake_minimum_required(VERSION 3.5)
project(PR)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES main.cc src/node.cc src/node.h src/utils.cc src/utils.h src/config.h src/config.cc src/common.h src/comm.cpp src/comm.h)

add_executable(PR ${SOURCE_FILES})
target_link_libraries(PR ${MPI_LIBRARIES})