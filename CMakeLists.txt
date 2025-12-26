cmake_minimum_required(VERSION 3.29)
project(ServerCluster C)

set(CMAKE_C_STANDARD 11)

set(MPI_INCLUDE_PATH "C:/Program Files (x86)/Microsoft SDKs/MPI/Include")
set(MPI_LIB_PATH     "C:/Program Files (x86)/Microsoft SDKs/MPI/Lib/x64")

include_directories("${MPI_INCLUDE_PATH}")

link_directories("${MPI_LIB_PATH}")

add_executable(ServerCluster main.c
        Queue/DoubleLinkedListQueue.c
        Queue/DoubleLinkedListQueue.h
        Dispatcher/JobDispatcher.c
        Dispatcher/JobDispatcher.h
        Utilities/Utilities.c
        Utilities/Utilities.h)

target_link_libraries(ServerCluster msmpi)