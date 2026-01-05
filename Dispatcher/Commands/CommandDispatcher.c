//
// Created by Denis on 1/5/2026.
//

#include "CommandDispatcher.h"

#include <mpi.h>
#include <stdio.h>
#include "../../Globals/globals.h"
#include "../../Queue/DoubleLinkedListQueue.h"
#include "../../Jobs/Job.h"

DWORD WINAPI dispatchCommands(LPVOID lpParam) {
    printf("Started dispatching commands\n");
    fflush(stdout);
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

        printJob(job);
        while(job == NULL) {
            SleepConditionVariableCS(&commandAvailableCondition, &commandAvailableMutex, INFINITE);
            job = dequeue(jobQueue);
            assignedJobs[*worker - 1] = job;
        }
        LeaveCriticalSection(&commandAvailableMutex);

        MPI_Send(job->params, strlen(job->params) + 1, MPI_CHAR, *worker, job->jobType, MPI_COMM_WORLD);
    }
    printf("Finished dispatching commands\n");
    fflush(stdout);
    return 0;
}
