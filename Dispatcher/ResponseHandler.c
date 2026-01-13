//
// Created by Denis on 1/5/2026.
//

#include "ResponseHandler.h"

#include "../Queue/DoubleLinkedListQueue.h"
#include <mpi.h>
#include <stdio.h>
#include "../Globals/globals.h"
#include <Windows.h>

static int availableJobs(void) {
    return finishedReading == 0 || !is_empty(jobQueue) || get_size(availableWorkers) != (totalWorkers - 1);;
}

static int availableResponses(void) {
    return finishedReading == 0 || !is_empty(responseQueue) || !is_empty(jobQueue) || get_size(availableWorkers) != totalWorkers - 1;
}

static void handleResponse(job_t response, int worker) {
    EnterCriticalSection(&responseAvailableMutex);

    enqueue(responseQueue, response);

    WakeConditionVariable(&responseAvailableCondition);

    LeaveCriticalSection(&responseAvailableMutex);

    EnterCriticalSection(&workerAvailableMutex);

    enqueue(availableWorkers, &workers[worker - 1]);

    WakeConditionVariable(&workerAvailableCondition);

    LeaveCriticalSection(&workerAvailableMutex);
}

static job_t readResponseFromJob(MPI_Status *status) {
    char *response = NULL;
    int responseSize = 0;

    MPI_Get_count(status, MPI_CHAR, &responseSize);

    responseSize = responseSize + 1;

    response = malloc(responseSize * sizeof(char));

    if(response == NULL) {
        perror("Eroare alocare");
        exit(-1);
    }

    job_t finishedJob = assignedJobs[status->MPI_SOURCE - 1];

    MPI_Recv(response, responseSize, MPI_CHAR, status->MPI_SOURCE, status->MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    assignedJobs[status->MPI_SOURCE - 1] = NULL;
    free(finishedJob->params);

    finishedJob->params = response;

    //printf("Finish job by worker %d: \n", status->MPI_SOURCE);
    //fflush(stdout);

    return finishedJob;
}

DWORD WINAPI getResponses(LPVOID lpParam) {
    MPI_Status status;

    while(availableJobs()) {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        job_t response = readResponseFromJob(&status);

        handleResponse(response, status.MPI_SOURCE);
    }

    return 0;
}

static job_t dequeueJobResponse(void) {
    EnterCriticalSection(&responseAvailableMutex);

    job_t response = dequeue(responseQueue);

    LeaveCriticalSection(&responseAvailableMutex);

    return response;
}

static job_t waitForResponse(void) {
    job_t response = dequeueJobResponse();

    while(response == NULL) {
        SleepConditionVariableCS(&responseAvailableCondition, &responseAvailableMutex, INFINITE);
        response = dequeue(responseQueue);

        while(response->chunkId != 0) {
            SleepConditionVariableCS(&responseAvailableCondition, &responseAvailableMutex, INFINITE);
            enqueue(responseQueue, response);
            response = dequeueJobResponse();
        }
    }

    return response;
}

DWORD WINAPI saveResponses(LPVOID lpParam) {
    char baseFileName[256] = "C:\\Users\\Denis\\CLionProjects\\ServerCluster\\Resources\\response";



    while(availableResponses()) {
        //Wait for a response to exist to not poll
        EnterCriticalSection(&responseAvailableMutex);

        job_t response = waitForResponse();

        LeaveCriticalSection(&responseAvailableMutex);

        char fileName[256];

        sprintf(fileName, "%s%llu.txt", baseFileName, response->jobId);

        FILE *responseFile = fopen(fileName, "w");

        if(responseFile == NULL) {
            perror("Error opening response file\n");
            exit(-1);
        }

        fprintf(responseFile, "%s\n", response->params);
        fflush(responseFile);

        fclose(responseFile);

        destructJob(response);
    }


    return 0;
}