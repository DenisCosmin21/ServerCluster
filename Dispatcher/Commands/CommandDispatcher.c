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
    return finishedReading == 0 || !is_empty(jobQueue);
}

static int findAvailableWorker(void) {
    EnterCriticalSection(&workerAvailableMutex);

    int *worker = dequeue(availableWorkers);

    //Wait untill a worker is ready to work

    while(worker == NULL) {
        SleepConditionVariableCS(&workerAvailableCondition, &workerAvailableMutex, INFINITE);
        worker = dequeue(availableWorkers);
    }

    LeaveCriticalSection(&workerAvailableMutex);

    return *worker;
}

static job_t getNextJob() {
    EnterCriticalSection(&commandAvailableMutex);

    job_t job = dequeue(jobQueue);

    //Wait untill a command is received

    while(job == NULL) {
        SleepConditionVariableCS(&commandAvailableCondition, &commandAvailableMutex, INFINITE);
        job = dequeue(jobQueue);
    }

    LeaveCriticalSection(&commandAvailableMutex);

    return job;
}

DWORD WINAPI dispatchCommands(LPVOID lpParam) {
    char log[1024];

    sprintf(log, "Starting dispatching jobs\n");

    logData(log);

    while(availableCommands()) {
        int worker = findAvailableWorker();

        job_t job = getNextJob();

        assignedJobs[worker - 1] = job;

        sprintf(log, "Dispatching job[Type = %s; JobId = %llu; ChunkId = %llu] to worker %d\n", getJobType(job), job->jobId, job->chunkId, worker);

        logData(log);

        MPI_Send(job->params, strlen(job->params) + 1, MPI_CHAR, worker, job->jobType, MPI_COMM_WORLD);

        if(job->additionalParam != NULL)
            MPI_Send(job->additionalParam, strlen(job->additionalParam) + 1, MPI_CHAR, worker, job->jobType, MPI_COMM_WORLD);
    }

    sprintf(log, "Finished dispatching jobs\n");
    logData(log);
    return 0;
}
