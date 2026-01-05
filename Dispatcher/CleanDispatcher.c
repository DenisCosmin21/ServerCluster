//
// Created by Denis on 1/5/2026.
//

#include "CleanDispatcher.h"

#include <mpi.h>
#include <stdio.h>
#include "../Jobs/Job.h"
#include "../Globals/globals.h"

static void finishWorkers(void) {
    char test = 0;

    for(int i = 0; i < totalWorkers - 1; i++) {
        printf("Finishing worker %d\n", workers[i]);
        fflush(stdout);
        MPI_Send(&test, 1, MPI_CHAR, workers[i], STOP_WORKING, MPI_COMM_WORLD);
    }
}

void cleanDispatcher(HANDLE *threads) {
    // Wait for all threads to finish (Equivalent to pthread_join)
    WaitForMultipleObjects(4, threads, TRUE, INFINITE);

    // Close handles to release resources
    for(int i = 0; i < 4; i++) {
        CloseHandle(threads[i]);
    }

    DeleteCriticalSection(&commandAvailableMutex);
    DeleteCriticalSection(&workerAvailableMutex);
    DeleteCriticalSection(&responseAvailableMutex);

    finishWorkers();
}