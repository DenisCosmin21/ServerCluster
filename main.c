#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

#include "Dispatcher/JobDispatcher.h"
#include "Worker/Worker.h"

int main(void) {
    int rank = 0;
    int provided = 0;
    int version = 0;
    int subversion = 0;

    MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);

    if (provided < MPI_THREAD_MULTIPLE) {
        // Note: Some versions of MS-MPI only support up to MPI_THREAD_SERIALIZED
        exit(-1);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0)
        runDispatcher();
    else
        runWorker();

    printf("Finished program\n");
    fflush(stdout);
    MPI_Finalize();

    return 0;
}
