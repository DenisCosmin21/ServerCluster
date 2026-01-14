//
// Created by Denis on 1/5/2026.
//

#include "CleanDispatcher.h"

#include <mpi.h>
#include <stdio.h>
#include "../Jobs/Job.h"
#include "../Globals/globals.h"
#include "../Logger/Logger.h"
#include "../Config/config.h"

static void finishWorkers(void) {
    char test = 0;

    for(int i = 0; i < totalWorkers - 1; i++) {
        printf("Finishing worker %d\n", workers[i]);
        fflush(stdout);
        MPI_Send(&test, 1, MPI_CHAR, workers[i], STOP_WORKING, MPI_COMM_WORLD);
    }
}

void cleanDispatcher(HANDLE *threads) {
#if MODE == PARALLEL
    // Wait for all threads to finish (Equivalent to pthread_join)
    WaitForMultipleObjects(4, threads, TRUE, INFINITE);

    QueryPerformanceCounter(&end);

    double interval = (double)(end.QuadPart - start.QuadPart) / (double)frequency.QuadPart;
    printf("Execution time: %lf\n", interval);
    fflush(stdout);

    // Close handles to release resources
    for(int i = 0; i < 4; i++) {
        CloseHandle(threads[i]);
    }

    DeleteCriticalSection(&commandAvailableMutex);
    DeleteCriticalSection(&workerAvailableMutex);
    DeleteCriticalSection(&responseAvailableMutex);

    finishWorkers();
#else
    WaitForMultipleObjects(1, threads, TRUE, INFINITE);

    CloseHandle(threads[0]);
#endif

    freeLogger();
}