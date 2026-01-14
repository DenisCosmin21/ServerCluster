//
// Created by Denis on 1/5/2026.
//

#include "CommandDispatcher.h"

#include <mpi.h>
#include <stdio.h>
#include "../../Globals/globals.h"
#include "../../Queue/DoubleLinkedListQueue.h"
#include "../../Jobs/Job.h"
#include "../../Logger/Logger.h"
static int availableCommands(void) {

    EnterCriticalSection(&commandAvailableMutex);

    #ifdef DEBUG
    printf("Locking available commands mutex in availableCommands\n");
    fflush(stdout);
    #endif

    int resp = finishedReading == 0 || !is_empty(jobQueue);

    #ifdef DEBUG
    printf("Unlocking available commands mutex in availableCommands : %d\n", resp);
    fflush(stdout);
    #endif

    LeaveCriticalSection(&commandAvailableMutex);

    return resp;
}

static int findAvailableWorker(void) {
    EnterCriticalSection(&workerAvailableMutex);

#ifdef DEBUG
    printf("Locking available worker mutex in findAvailableWorker\n");
    fflush(stdout);
#endif

    int *worker = dequeue(availableWorkers);

    //Wait untill a worker is ready to work

    while(worker == NULL) {
        #ifdef DEBUG
        printf("No available workers in findAvailableWorker\n");
        fflush(stdout);
        #endif
        SleepConditionVariableCS(&workerAvailableCondition, &workerAvailableMutex, INFINITE);
        worker = dequeue(availableWorkers);
    }
#ifdef DEBUG
    printf("Worker found in findAvailableWorker: %d\n", *worker);
    printf("Unlocking available workers mutex in findAvailableWorker\n");
    fflush(stdout);
    #endif
    LeaveCriticalSection(&workerAvailableMutex);

    return *worker;
}

static job_t getNextJob() {
    EnterCriticalSection(&commandAvailableMutex);
#ifdef DEBUG
    printf("Locking available commands mutex in getNextJob\n");
    fflush(stdout);
    #endif
    job_t job = dequeue(jobQueue);

    //Wait untill a command is received

    while(job == NULL) {
        #ifdef DEBUG
        printf("No available jobs in getNextJob\n");
        fflush(stdout);
        #endif
        SleepConditionVariableCS(&commandAvailableCondition, &commandAvailableMutex, INFINITE);
        job = dequeue(jobQueue);
    }

    #ifdef DEBUG
    printf("Job found in getNextJobs\n");
    printf("Unlocking availableCommandsMutex in getNextJob\n");
    fflush(stdout);
    #endif
    LeaveCriticalSection(&commandAvailableMutex);

    return job;
}

DWORD WINAPI dispatchCommands(LPVOID lpParam) {
    char log[2048];

    sprintf(log, "Starting dispatching jobs\n");

    logData(log);

    while(availableCommands()) {
        int worker = findAvailableWorker();

        job_t job = getNextJob();

        assignedJobs[worker - 1] = job;

        sprintf(log, "Dispatching job[Type = %s; JobId = %llu; ChunkId = %llu] to worker %d\n", getJobType(job), job->jobId, job->chunkId, worker);

        logData(log);

        char *logBuffer = NULL;

        if(strlen(job->params) > 2048) {
            logBuffer = malloc(strlen(job->params) + 30);
            *logBuffer = 0;
            sprintf(logBuffer, "Parameters : %s\n", job->params);
            logData(logBuffer);
        }
        else {
            sprintf(log, "Parameters : %s\n", job->params);

            logData(log);
        }

        if(job->additionalParam != NULL) {
            if(strlen(job->additionalParam) > 2048) {
                if(strlen(job->additionalParam) > strlen(logBuffer))
                    logBuffer = realloc(logBuffer, strlen(job->additionalParam) + 30);

                sprintf(logBuffer, "Additional parameters : %s\n", job->additionalParam);
                logData(logBuffer);

                free(logBuffer);
            }
            else {
                sprintf(log, "Additional parameters : %s\n", job->additionalParam);
                logData(log);
            }
        }

        MPI_Send(job->params, strlen(job->params) + 1, MPI_CHAR, worker, job->jobType, MPI_COMM_WORLD);

        if(job->additionalParam != NULL)
            MPI_Send(job->additionalParam, strlen(job->additionalParam) + 1, MPI_CHAR, worker, job->jobType, MPI_COMM_WORLD);
    }

    sprintf(log, "Finished dispatching jobs\n");
    logData(log);
    return 0;
}
