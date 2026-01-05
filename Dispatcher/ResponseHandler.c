//
// Created by Denis on 1/5/2026.
//

#include "ResponseHandler.h"

#include "../Queue/DoubleLinkedListQueue.h"
#include <mpi.h>
#include <stdio.h>
#include "../Globals/globals.h"
#include <Windows.h>

static void handleResponse(char *response, int worker) {
    EnterCriticalSection(&responseAvailableMutex);

    enqueue(responseQueue, response);

    WakeConditionVariable(&responseAvailableCondition);

    LeaveCriticalSection(&responseAvailableMutex);

    EnterCriticalSection(&workerAvailableMutex);

    enqueue(availableWorkers, &workers[worker - 1]);

    WakeConditionVariable(&workerAvailableCondition);

    LeaveCriticalSection(&workerAvailableMutex);
}

    job_t finishedJob = assignedJobs[status->MPI_SOURCE - 1];

    assignedJobs[status->MPI_SOURCE - 1] = NULL;
    free(finishedJob->params);

    finishedJob->params = response;

    printf("Finish job by worker %d: \n", status->MPI_SOURCE);
    fflush(stdout);
DWORD WINAPI getResponses(LPVOID lpParam) {
    printf("Started getting responses\n");
    fflush(stdout);

    char *response = NULL;
    int responseSize = 0;
    MPI_Status status;

    while(finishedReading == 0 || !is_empty(jobQueue)) {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        job_t response = readResponseFromJob(&status);

        handleResponse(response, status.MPI_SOURCE);
    }

    printf("Finished getting responses\n");
    fflush(stdout);
    return 0;
}

DWORD WINAPI saveResponses(LPVOID lpParam) {
    printf("Started saving responses\n");
    fflush(stdout);
    FILE *responseFile = fopen("C:\\Users\\Denis\\CLionProjects\\ServerCluster\\Resources\\resources.txt", "w");

    if(responseFile == NULL) {
        perror("Error opening response file\n");
        exit(-1);
    }

    while(finishedReading == 0 || !is_empty(responseQueue) || !is_empty(jobQueue) || get_size(availableWorkers) != totalWorkers - 1) {
        //Wait for a response to exist to not poll
        EnterCriticalSection(&responseAvailableMutex);

        job_t response = waitForResponse();

        while(response == NULL) {
            SleepConditionVariableCS(&responseAvailableCondition, &responseAvailableMutex, INFINITE);
            response = dequeue(responseQueue);
        }
        LeaveCriticalSection(&responseAvailableMutex);
        fprintf(responseFile, "%s\n", response);
        fprintf(responseFile, "%s\n", response->params);
        fflush(responseFile);

        free(response->params);
        free(response);
    }
    fclose(responseFile);
    printf("Finished saving responses\n");
    fflush(stdout);
    return 0;
}