//
// Created by Denis on 1/5/2026.
//

#include "ResponseHandler.h"

#include "../Queue/DoubleLinkedListQueue.h"
#include <mpi.h>
#include <stdio.h>
#include "../Globals/globals.h"
#include <Windows.h>

#include "../Logger/Logger.h"

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

    char log[2048];

    MPI_Get_count(status, MPI_CHAR, &responseSize);

    responseSize = responseSize + 1;

    response = malloc(responseSize * sizeof(char));

    if(response == NULL) {
        perror("Eroare alocare");
        exit(-1);
    }

    job_t finishedJob = assignedJobs[status->MPI_SOURCE - 1];

    MPI_Recv(response, responseSize, MPI_CHAR, status->MPI_SOURCE, status->MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    sprintf(log, "Received response for job[Type = %s; jobId = %llu ; chunkId = %llu] from worker %d\n", getJobType(finishedJob), finishedJob->jobId, finishedJob->chunkId, status->MPI_SOURCE);

    logData(log);

    sprintf(log, "Response from job : %s\n", response);

    logData(log);

    assignedJobs[status->MPI_SOURCE - 1] = NULL;
    free(finishedJob->params);

    finishedJob->params = response;

    return finishedJob;
}

DWORD WINAPI getResponses(LPVOID lpParam) {
    char log[1024];

    sprintf(log, "%s\n", "Starting reading responses from jobs");

    logData(log);

    MPI_Status status;

    while(availableJobs()) {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        job_t response = readResponseFromJob(&status);

        handleResponse(response, status.MPI_SOURCE);
    }

    sprintf(log, "Finished reading responses from jobs");
    logData(log);
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

    while(response == NULL || response->chunkId != 0) {
        if(response != NULL) {
            if(chunksReachedPerJob[response->jobId] == response->chunkId - 1) {
                chunksReachedPerJob[response->jobId] = response->chunkId;
                break;
            }
            if(get_size(responseQueue) == 0)
                SleepConditionVariableCS(&responseAvailableCondition, &responseAvailableMutex, INFINITE);

            enqueue(responseQueue, response);
            response = dequeueJobResponse();
        }
        else {
            SleepConditionVariableCS(&responseAvailableCondition, &responseAvailableMutex, INFINITE);
            response = dequeueJobResponse();;
        }
    }

    return response;
}

DWORD WINAPI saveResponses(LPVOID lpParam) {
    char log[1024];

    sprintf(log, "%s\n", "Starting saving responses from jobs");

    logData(log);
    char baseFileName[] = "Resources\\Responses\\response";

    while(availableResponses()) {
        //Wait for a response to exist to not poll
        EnterCriticalSection(&responseAvailableMutex);

        job_t response = waitForResponse();

        LeaveCriticalSection(&responseAvailableMutex);

        char fileName[256];

        sprintf(fileName, "%s%llu.txt", baseFileName, response->jobId);

        FILE *responseFile = fopen(fileName, "a");

        if(responseFile == NULL) {
            perror("Error opening response file\n");
            exit(-1);
        }

        fprintf(responseFile, "%s\n", response->params);
        fflush(responseFile);

        fclose(responseFile);

        destructJob(response);
    }

    sprintf(log, "Finished saving responses from jobs");
    logData(log);

    return 0;
}