#include <Job.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Dispatcher/JobDispatcher.h"
#include "Worker/Worker.h"
#include "Globals/globals.h"
#include "Config/config.h"
#include <windows.h>

int main(void) {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    QueryPerformanceCounter(&start);

#if MODE == PARALLEL
    int provided = 0;

    MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);

    if (provided < MPI_THREAD_MULTIPLE) {
        exit(1);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0)
        runDispatcher();
    else
        runWorker();

    QueryPerformanceCounter(&end);

    if(rank == 0) {

        fflush(stdout);
    }
    MPI_Finalize();

#else
    runDispatcher();
    #endif


    return 0;

}
