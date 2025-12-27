//
// Created by Denis on 12/26/2025.
//

#include "JobDispatcher.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Utilities/Utilities.h"
#include "../Queue/DoubleLinkedListQueue.h"
#include <Windows.h>
#include "../Jobs/Job.h"
#include "../Jobs/JobReader.h"

static int *workers = NULL;
static int totalWorkers = 0;
static queue_t jobQueue;
static queue_t responseQueue;
static queue_t availableWorkers;
static char finishedReading = 0;

static CONDITION_VARIABLE commandAvailableCondition;
static CONDITION_VARIABLE workerAvailableCondition;
static CONDITION_VARIABLE responseAvailableCondition;

static CRITICAL_SECTION commandAvailableMutex;
static CRITICAL_SECTION workerAvailableMutex;
static CRITICAL_SECTION responseAvailableMutex;

static void handleCommand(const job_t job) {
    if(job->jobType == WAIT) {
        Sleep(strtol(job->params, NULL, 10) * 1000);
        return;
    }

    EnterCriticalSection(&commandAvailableMutex);
    enqueue(jobQueue, job);
    WakeConditionVariable(&commandAvailableCondition);
    LeaveCriticalSection(&commandAvailableMutex);
}

static DWORD WINAPI readCommands(LPVOID lpParam) {
    FILE *commandFile = fopen("C:\\Users\\Denis\\CLionProjects\\ServerCluster\\Resources\\commands.txt", "r");

    if (commandFile == NULL) {
        perror("Error opening command file\n");
        exit(-1);
    }

    job_t job = NULL;

    while((job = readCommand(commandFile)) != NULL) {
        handleCommand(job);
    }

    finishedReading = 1;
    fclose(commandFile);
    return 0;
}

static DWORD WINAPI dispatchCommands(LPVOID lpParam) {
    while(finishedReading == 0 || !is_empty(jobQueue)) {
        //Wait untill a worker is ready to work
        EnterCriticalSection(&workerAvailableMutex);

        int *worker = dequeue(availableWorkers);

        while(worker == NULL) {
            SleepConditionVariableCS(&workerAvailableCondition, &workerAvailableMutex, INFINITE);
            worker = dequeue(availableWorkers);
        }

        LeaveCriticalSection(&workerAvailableMutex);

        //Wait untill a command is received
        EnterCriticalSection(&commandAvailableMutex);

        job_t job = dequeue(jobQueue);

        while(job == NULL) {
            SleepConditionVariableCS(&commandAvailableCondition, &commandAvailableMutex, INFINITE);
            job = dequeue(jobQueue);
        }
        LeaveCriticalSection(&commandAvailableMutex);

        MPI_Send(job->params, strlen(job->params) + 1, MPI_CHAR, *worker, job->jobType, MPI_COMM_WORLD);
        free(job->params);
        free(job);
    }

    return 0;
}

static DWORD WINAPI getResponses(LPVOID lpParam) {
    char *response = NULL;
    int responseSize = 0;
    int rank = 0;
    MPI_Status status;

    while(finishedReading == 0 || !is_empty(jobQueue)) {
        MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

        MPI_Get_count(&status, MPI_CHAR, &responseSize);

        responseSize = responseSize + 1;

        rank = status.MPI_SOURCE;

        response = malloc(responseSize * sizeof(char));

        if(response == NULL) {
            perror("Eroare alocare");
            exit(-1);
        }

        MPI_Recv(response, responseSize, MPI_CHAR, rank, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        EnterCriticalSection(&responseAvailableMutex);

        enqueue(responseQueue, response);

        WakeConditionVariable(&responseAvailableCondition);

        LeaveCriticalSection(&responseAvailableMutex);

        EnterCriticalSection(&workerAvailableMutex);

        enqueue(availableWorkers, &workers[rank - 1]);

        WakeConditionVariable(&workerAvailableCondition);

        LeaveCriticalSection(&workerAvailableMutex);
    }
    return 0;
}

static DWORD WINAPI saveResponses(LPVOID lpParam) {
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
    return 0;
}

static void initAvailableWorkers(void) {
    queue_init(&availableWorkers);

    MPI_Comm_size(MPI_COMM_WORLD, &totalWorkers);

    workers = malloc(totalWorkers * sizeof(int));
    if(workers == NULL) {
        perror("Eroare alocare");
        exit(-1);
    }

    for(int i = 0; i < totalWorkers - 1; i++) {
        workers[i] = i + 1;
        enqueue(availableWorkers, &workers[i]);
    }
}

static void finishWorkers(void) {
    char test = 0;

    for(int i = 0; i < totalWorkers - 1; i++) {
        printf("Finishing worker %d\n", workers[i]);
        fflush(stdout);
        MPI_Send(&test, 1, MPI_CHAR, workers[i], STOP_WORKING, MPI_COMM_WORLD);
    }
}

void runDispatcher(void) {
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
    HANDLE threads[4];

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