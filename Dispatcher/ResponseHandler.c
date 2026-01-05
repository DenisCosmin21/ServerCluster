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

DWORD WINAPI getResponses(LPVOID lpParam) {
    printf("Started getting responses\n");
    fflush(stdout);

    char *response = NULL;
    int responseSize = 0;
    MPI_Status status;

    while(finishedReading == 0 || !is_empty(jobQueue)) {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        MPI_Get_count(&status, MPI_CHAR, &responseSize);

        responseSize = responseSize + 1;

        response = malloc(responseSize * sizeof(char));

        if(response == NULL) {
            perror("Eroare alocare");
            exit(-1);
        }

        //job_t finishedJob = assignedJobs[status.MPI_SOURCE - 1];

        MPI_Recv(response, responseSize, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //printJob(finishedJob);

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

        char *response = dequeue(responseQueue);

        while(response == NULL) {
            SleepConditionVariableCS(&responseAvailableCondition, &responseAvailableMutex, INFINITE);
            response = dequeue(responseQueue);
        }
        LeaveCriticalSection(&responseAvailableMutex);
        fprintf(responseFile, "%s\n", response);
        fflush(responseFile);
    }
    fclose(responseFile);
    printf("Finished saving responses\n");
    fflush(stdout);
    return 0;
}