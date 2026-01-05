//
// Created by Denis on 1/5/2026.
//

#include "Initialize.h"

#include "../Queue/DoubleLinkedListQueue.h"
#include <mpi.h>
#include <stdio.h>
#include "../Globals/globals.h"
#include "ResponseHandler.h"
#include "Commands/CommandDispatcher.h"
#include "Commands/CommandReader.h"

static void initAvailableWorkers(void) {
    printf("Initializing workers\n");
    fflush(stdout);
    queue_init(&availableWorkers);

    MPI_Comm_size(MPI_COMM_WORLD, &totalWorkers);

    workers = malloc(totalWorkers * sizeof(int));

    //We use this to keep in mind wihch job was executed by which worker. The memory used is small.
    assignedJobs = malloc(totalWorkers * sizeof(job_t));

    if(workers == NULL) {
        perror("Eroare alocare");
        exit(-1);
    }

    for(int i = 0; i < totalWorkers - 1; i++) {
        workers[i] = i + 1;
        assignedJobs[i] = NULL;
        enqueue(availableWorkers, &workers[i]);
    }
    printf("Finished Initializing workers\n");
    fflush(stdout);
}

void initializeDispatcher(HANDLE *threads) {
    printf("Initializing Dispatcher\n");
    fflush(stdout);
    queue_init(&jobQueue);
    queue_init(&responseQueue);
    initAvailableWorkers();

    //Initialize condition variables for different kind of events to not make 100% cpu usage by polling
    InitializeConditionVariable(&commandAvailableCondition);
    InitializeConditionVariable(&workerAvailableCondition);
    InitializeConditionVariable(&responseAvailableCondition);

    InitializeCriticalSection(&commandAvailableMutex);
    InitializeCriticalSection(&workerAvailableMutex);
    InitializeCriticalSection(&responseAvailableMutex);

    // Windows Handles for threads


    // CreateThreads
    threads[0] = CreateThread(NULL, 0, readCommands, NULL, 0, NULL);
    threads[1] = CreateThread(NULL, 0, dispatchCommands, NULL, 0, NULL);
    threads[2] = CreateThread(NULL, 0, getResponses, NULL, 0, NULL);
    threads[3] = CreateThread(NULL, 0, saveResponses, NULL, 0, NULL);

    // Check for creation errors
    for(int i = 0; i < 4; i++) {
        if (threads[i] == NULL) {
            fprintf(stderr, "Error creating thread %d\n", i);
            exit(-1);
        }
    }

    printf("Finished initializing dispatcher\n");
    fflush(stdout);
}
