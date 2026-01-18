cmake_minimum_required(VERSION 3.29)
project(ServerCluster C)

set(CMAKE_C_STANDARD 11)

set(MPI_INCLUDE_PATH "C:/Program Files (x86)/Microsoft SDKs/MPI/Include")
set(MPI_LIB_PATH     "C:/Program Files (x86)/Microsoft SDKs/MPI/Lib/x64")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")

find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)

add_executable(ServerCluster main.c
        Config/config.h
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
        Dispatcher/Commands/CommandDispatcher.c
        Jobs/Operations/Sleep/WaitJob.c
        Jobs/Operations/Handler/ReadJobHandler.c
        Jobs/Operations/Primes/Primes.c
        Jobs/Operations/PrimeDivizors/PrimeDivisors.c
        Jobs/Operations/Handler/ExecuteJobHandler.c
        Jobs/Operations/Handler/ExecuteJobHandler.h
        Jobs/Operations/Anagrams/Anagrams.c
        Jobs/Operations/Anagrams/Anagrams.h
        Jobs/Operations/Matrix/MatrixAdd/MatrixAdd.c
        Jobs/Operations/Handler/MatrixHandler.c
        Jobs/Operations/Handler/MatrixHandler.c
        Jobs/Operations/Matrix/MatrixProduct/MatrixMult.c
        Jobs/Operations/Matrix/MatrixProduct/MatrixMult.h
        Logger/Logger.c
        Logger/Logger.h
        Jobs/Operations/Handler/SerialHandler.c
        Jobs/Operations/Handler/SerialHandler.h
        Jobs/Operations/Handler/ConvolutionHandler.c
        Jobs/Operations/Handler/ConvolutionHandler.h
        Jobs/Operations/Convolution/ComputeConvolution.c
        Jobs/Operations/Convolution/ComputeConvolution.h
        Jobs/Operations/Convolution/Kernels.c
        Jobs/Operations/Convolution/Kernels.h
)

target_include_directories(ServerCluster PRIVATE
        ${MPI_INCLUDE_PATH}
        Queue
        Dispatcher
        Dispatcher/Commands
        Utilities
        Worker
        Jobs
        Jobs/Operations/Handler
        Jobs/Operations/Sleep
        Jobs/Operations/Primes
        Globals
        Config
        Jobs/Operations/PrimeDivizors
)

target_link_directories(ServerCluster PRIVATE
        ${MPI_LIB_PATH}
)

target_link_libraries(
        ServerCluster PRIVATE
        msmpi
        Threads::Threads
        OpenMP::OpenMP_C
    )