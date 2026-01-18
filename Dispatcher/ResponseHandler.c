//
// Created by Denis on 1/5/2026.
//

#include "ResponseHandler.h"

#include "../Queue/DoubleLinkedListQueue.h"
#include <mpi.h>
#include <stdio.h>
#include "../Globals/globals.h"
#include <Windows.h>
#include "../Jobs/Operations/Handler/ConvolutionHandler.h"
#include "../Logger/Logger.h"

static int availableJobs(void) {
    EnterCriticalSection(&commandAvailableMutex);
#ifdef DEBUG
    printf("Locking command mutex in availableJobs\n");
    fflush(stdout);
#endif
    int partialResp = finishedReading == 0 || !is_empty(jobQueue);

    #ifdef DEBUG
    printf("Unlocking command mutex in availabel jobs : %d\n", partialResp);
    #endif

    LeaveCriticalSection(&commandAvailableMutex);

    EnterCriticalSection(&workerAvailableMutex);

    #ifdef DEBUG
    printf("Locking worker mutex in availableJobs\n");
    fflush(stdout);
    #endif

    int resp = partialResp || (get_size(availableWorkers) != (totalWorkers - 1));

    #ifdef DEBUG
    printf("Unlocking worker mutex in available jobs : %d\n", resp);
    fflush(stdout);
    #endif

    LeaveCriticalSection(&workerAvailableMutex);



    return resp;
}

static int availableResponses(void) {
    EnterCriticalSection(&commandAvailableMutex);

    #ifdef DEBUG
    printf("Locking commandAvailableMutex in availableResponses\n");
    fflush(stdout);
    #endif

    int partialResp = finishedReading == 0 || !is_empty(jobQueue);

    #ifdef DEBUG
    printf("Unlocking AvailableMutex mutex in availableResponses : %d\n", partialResp);
    fflush(stdout);
    #endif

    LeaveCriticalSection(&commandAvailableMutex);

    EnterCriticalSection(&workerAvailableMutex);

    #ifdef DEBUG
    printf("Locking worker mutex in availableResponsess\n");
    fflush(stdout);
    #endif

    partialResp = partialResp || (get_size(availableWorkers) != (totalWorkers - 1));

    #ifdef DEBUG
    printf("Unlocking worker mutex in availableResponses : %d\n", partialResp);
    fflush(stdout);
    #endif

    LeaveCriticalSection(&workerAvailableMutex);

    EnterCriticalSection(&responseAvailableMutex);
    #ifdef DEBUG
    printf("Locking responses mutex in availableResponses\n");
    fflush(stdout);
    #endif
    int resp = partialResp || !is_empty(responseQueue);
    #ifdef DEBUG
    printf("Unlcoking responses mutex in availableResponses : %d\n", resp);
    fflush(stdout);
    #endif
    LeaveCriticalSection(&responseAvailableMutex);

    return resp;
}

static void handleResponse(job_t response, int worker) {
    EnterCriticalSection(&responseAvailableMutex);

    #ifdef DEBUG
    printf("Locking response mutex in handleResponse\n");
    fflush(stdout);
    #endif

    enqueue(responseQueue, response);
    WakeConditionVariable(&responseAvailableCondition);

    #ifdef DEBUG
    printf("Unlocking response mutex in handleResponse\n");
    fflush(stdout);
    #endif

    LeaveCriticalSection(&responseAvailableMutex);

    EnterCriticalSection(&workerAvailableMutex);

    #ifdef DEBUG
    printf("Locking worker mutex in handleResponse\n");
    fflush(stdout);
    #endif

    enqueue(availableWorkers, &workers[worker - 1]);
    WakeConditionVariable(&workerAvailableCondition);

    #ifdef DEBUG
    printf("Unlocking worker mutex in handleResponse : %d\n", worker);
    fflush(stdout);
    #endif

    LeaveCriticalSection(&workerAvailableMutex);
}

static job_t readResponseFromJob(MPI_Status *status) {
    char *response = NULL;
    int responseSize = 0;

    char log[4096];

    MPI_Get_count(status, MPI_CHAR, &responseSize);

    responseSize = responseSize + 1;

    response = malloc(responseSize * sizeof(char));

    if(response == NULL) {
        perror("Eroare alocare");
        exit(-1);
    }

    job_t finishedJob = assignedJobs[status->MPI_SOURCE - 1];

    MPI_Recv(response, responseSize, MPI_CHAR, status->MPI_SOURCE, status->MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if(responseSize > 4096) {
        char *responseLogBuffer = malloc((responseSize * sizeof(char) * 3) / 2);
        sprintf(responseLogBuffer, "Received response for job[Type = %s; jobId = %llu ; chunkId = %llu] from worker %d\nResponse from job : %s\n", getJobType(finishedJob), finishedJob->jobId, finishedJob->chunkId, status->MPI_SOURCE, response);

        logData(responseLogBuffer);

        free(responseLogBuffer);
    }
    else {
        sprintf(log, "Received response for job[Type = %s; jobId = %llu ; chunkId = %llu] from worker %d\nResponse from job : %s\n", getJobType(finishedJob), finishedJob->jobId, finishedJob->chunkId, status->MPI_SOURCE, response);

        logData(log);
    }

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

    #ifdef DEBUG
    printf("Locking response mutex in dequeueJobResponse\n");
    fflush(stdout);
    #endif

    job_t response = dequeue(responseQueue);

#ifdef DEBUG
    printf("Unlocking response mutex in dequeueJobResponse\n");
    fflush(stdout);
    #endif

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

#ifdef DEBUG
            printf("Waiting for valid responses in waitForResponses\n");
            fflush(stdout);
#endif

            EnterCriticalSection(&responseAvailableMutex);

            while(get_size(responseQueue) == 0) {
                SleepConditionVariableCS(&responseAvailableCondition, &responseAvailableMutex, INFINITE);
            }

            LeaveCriticalSection(&responseAvailableMutex);

            #ifdef DEBUG
            printf("Found a possible response in waitForResponses\n");
            fflush(stdout);
            #endif

            enqueue(responseQueue, response);
            response = dequeueJobResponse();
        }
        else {
            #ifdef DEBUG
            printf("Waiting for valid responses in waitForResponses\n");
            fflush(stdout);
            #endif

            EnterCriticalSection(&responseAvailableMutex);

            SleepConditionVariableCS(&responseAvailableCondition, &responseAvailableMutex, INFINITE);
            response = dequeueJobResponse();;

            LeaveCriticalSection(&responseAvailableMutex);

            #ifdef DEBUG
            printf("Found a possible response in waitForResponses\n");
            fflush(stdout);
            #endif
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
        job_t response = waitForResponse();

        sprintf(log, "Saving Job with Id = %llu; and chunk = %llu\n", response->jobId, response->chunkId);

        logData(log);

#ifdef DEBUG
            printf("Writing job response\n");
            fflush(stdout);
#endif

        char fileName[256];

        if(response->jobType == CONVOLUTION) {
            sprintf(fileName, "%s%llu.bmp", baseFileName, response->jobId);
            saveConvolution(fileName, response->params, response->additionalParam);
        }
        else {
            sprintf(fileName, "%s%llu.txt", baseFileName, response->jobId);

            FILE *responseFile = fopen(fileName, "a");

            if(responseFile == NULL) {
                perror("Error opening response file\n");
                exit(-1);
            }

            fprintf(responseFile, "%s\n", response->params);
            fflush(responseFile);

            fclose(responseFile);
        }

        destructJob(response);
    }

    sprintf(log, "Finished saving responses from jobs");
    logData(log);

    return 0;
}