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
#include "../Logger/Logger.h"

static void initAvailableWorkers(void) {
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

    memset(chunksReachedPerJob, 0, sizeof(chunksReachedPerJob));
}

static void clearResponseFiles(void) {
    WIN32_FIND_DATA findFileData;
    char searchPath[MAX_PATH];

    // Construct the search pattern (e.g., "Resources\*response*")
    snprintf(searchPath, sizeof(searchPath), "%s\\*response*", resourcesDirectory);

    HANDLE hFind = FindFirstFile(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        // No files found or directory doesn't exist
        return;
    }

    do {
        // Ensure we are deleting a file and not a directory
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            char filePath[MAX_PATH];
            snprintf(filePath, sizeof(filePath), "%s\\%s", resourcesDirectory, findFileData.cFileName);

            if (!DeleteFile(filePath)) {
                fprintf(stderr, "Failed to delete: %s (Error: %lu)\n", filePath, GetLastError());
            } else {
                // Optional: printf("Deleted old response file: %s\n", filePath);
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}

void initializeDispatcher(HANDLE *threads) {
    clearResponseFiles();

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
}
