//
// Created by Denis on 1/5/2026.
//

#include "CommandReader.h"
#include "../../Queue/DoubleLinkedListQueue.h"
#include "../../Globals/globals.h"
#include "../../Jobs/Job.h"
#include "../../Jobs/JobReader.h"
#include <stdio.h>
#include <Windows.h>

static void printQueueJob(void *job) {
    printJob(job);
}

static void handleCommand(job_t job) {
    if(job->jobType == WAIT) {
        Sleep(strtol(job->params, NULL, 10) * 1000);
        return;
    }

    EnterCriticalSection(&commandAvailableMutex);
    enqueue(jobQueue, job);
    WakeConditionVariable(&commandAvailableCondition);
    LeaveCriticalSection(&commandAvailableMutex);
}

DWORD WINAPI readCommands(LPVOID lpParam) {
    printf("Started reading commands\n");
    fflush(stdout);
    FILE *commandFile = fopen("C:\\Users\\Denis\\CLionProjects\\ServerCluster\\Resources\\commands.txt", "r");

    if (commandFile == NULL) {
        perror("Error opening command file\n");
        exit(-1);
    }

    job_t job = NULL;

    while((job = readCommand(commandFile)) != NULL) {
        printf("Reading command\n");
        fflush(stdout);
        handleCommand(job);
    }

    finishedReading = 1;
    fclose(commandFile);
    printf("Finished reading commands\n");
    fflush(stdout);
    return 0;
}