//
// Created by Denis on 12/26/2025.
//

#include "JobDispatcher.h"
#include <pthread_time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include "../Utilities/Utilities.h"
#include "../Queue/DoubleLinkedListQueue.h"
#include <Windows.h> // Essential for Windows threads

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

static void printInt(void *data) {
    int *i = (int *)data;
    printf("%d ", *i);
}

static void printString(void *data) {
    char *str = (char *)data;
    printf("%s\n", str);
}

static char *readCommand(FILE* file) {
    if(feof(file))
        return NULL;

    char *line = malloc(10 * sizeof(char)); //Use a allocation strategy for each line, so that we won't allocate too much space

    size_t max_length = 10;

    size_t current_length = 0;

    if(line == NULL) {
        perror("Eroare alocare");
        exit(-1);
    }

    line[current_length++] = fgetc(file);

    while(line[current_length - 1] != '\n' && line[current_length - 1] != EOF) {
        if(current_length >= max_length)
            line = allocate(line, sizeof(char), &max_length);

        line[current_length++] = fgetc(file);
    }

    line[current_length - 1] = '\0';

    return line;
}

static void handleCommand(char *command) {
    if(strstr(command, "WAIT")) {
        strtok(command, " ");
        char *timeToSleep = strtok(NULL, " ");
        Sleep(strtol(timeToSleep, NULL, 10) * 1000);
        return;
    }

    EnterCriticalSection(&commandAvailableMutex);
    enqueue(jobQueue, command);
    WakeConditionVariable(&commandAvailableCondition);
    LeaveCriticalSection(&commandAvailableMutex);
}

static DWORD WINAPI readCommands(LPVOID lpParam) {
    FILE *commandFile = fopen("C:\\Users\\Denis\\CLionProjects\\ServerCluster\\Resources\\commands.txt", "r");

    if (commandFile == NULL) {
        perror("Error opening command file\n");
        exit(-1);
    }

    char *line = NULL;

    while((line = readCommand(commandFile)) != NULL) {
        handleCommand(line);
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

        char *command = dequeue(jobQueue);

        while(command == NULL) {
            SleepConditionVariableCS(&commandAvailableCondition, &commandAvailableMutex, INFINITE);
            command = dequeue(jobQueue);
        }
        LeaveCriticalSection(&commandAvailableMutex);

        MPI_Send(command, strlen(command) + 1, MPI_CHAR, *worker, WORK, MPI_COMM_WORLD);
        free(command);
    }

    return 0;
}

static DWORD WINAPI getResponses(LPVOID lpParam) {
    char *response = NULL;
    int responseSize = 0;
    int rank = 0;

    MPI_Status status;

    /*We can have response if we haven't finished reading, or if we haven't finished dispathcing all jobs*/

    while(finishedReading == 0 || !is_empty(jobQueue)) {
        MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

        MPI_Get_count(&status, MPI_CHAR, &responseSize);

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

void runDispatcher(void) { //used to initialize the queue for the dispatcher and start the threads and wait for them
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