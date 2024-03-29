cmake_minimum_required(VERSION 3.5)
project(PR)

find_package(MPI REQUIRED)
find_package(Threads)

include_directories(${MPI_INCLUDE_PATH})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES
        main.cc
        src/node.cc
        src/node.h
        src/utils.h
        src/config.h
        src/config.cc
        src/common.h
        src/comm.cc
        src/comm.h
        src/message.cc
        src/message.h
        src/ack_queue.cc
        src/ack_queue.h
        )

add_executable(PR ${SOURCE_FILES})
target_link_libraries(PR ${MPI_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})