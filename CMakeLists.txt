cmake_minimum_required(VERSION 3.29)
project(ServerCluster C)

set(CMAKE_C_STANDARD 11)

set(MPI_INCLUDE_PATH "C:/Program Files (x86)/Microsoft SDKs/MPI/Include")
set(MPI_LIB_PATH     "C:/Program Files (x86)/Microsoft SDKs/MPI/Lib/x64")

find_package(Threads REQUIRED)

add_executable(ServerCluster main.c
        Queue/DoubleLinkedListQueue.c
        Dispatcher/JobDispatcher.c
        Utilities/Utilities.c
        Worker/Worker.c
        Jobs/JobReader.c
        Jobs/Job.c
        Dispatcher/ResponseHandler.c
        Globals/globals.c
        Dispatcher/Initialize.c
        Dispatcher/CleanDispatcher.c
        Dispatcher/Commands/CommandReader.c
        Dispatcher/Commands/CommandDispatcher.c)

target_include_directories(ServerCluster PRIVATE
        ${MPI_INCLUDE_PATH}
        Queue
        Dispatcher
        Dispatcher/Commands
        Utilities
        Worker
        Jobs
        Globals
        Config
)

target_link_directories(ServerCluster PRIVATE
        ${MPI_LIB_PATH}
)

target_link_libraries(
        ServerCluster PRIVATE
        msmpi
        Threads::Threads
    )