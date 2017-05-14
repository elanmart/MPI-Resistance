cmake_minimum_required(VERSION 3.5)
project(PR)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(SOURCE_FILES main.c src/node.c src/node.h src/utils.c src/utils.h src/config.h src/config.c src/interface.c src/interface.h)

add_executable(PR ${SOURCE_FILES})
target_link_libraries(PR ${MPI_LIBRARIES})