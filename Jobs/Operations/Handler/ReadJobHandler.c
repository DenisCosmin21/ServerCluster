//
// Created by Denis on 1/6/2026.
//

#include "ReadJobHandler.h"

#include "../Jobs/Operations/Handler/MatrixHandler.h"
#include <stdio.h>
#include <Windows.h>
#include "../../../Queue/DoubleLinkedListQueue.h"
#include "../../../Globals/globals.h"
#include "../Sleep/WaitJob.h"

void enqueueJob(job_t job) {
    EnterCriticalSection(&commandAvailableMutex);

#ifdef DEBUG
    printf("Locking commandAvailableMutex in enqueueJob\n");
    fflush(stdout);
#endif

    enqueue(jobQueue, job);
    WakeConditionVariable(&commandAvailableCondition);

    #ifdef DEBUG
    printf("unlocking commandAvailableMutex in enqueueJob\n");
    fflush(stdout);
    #endif

    LeaveCriticalSection(&commandAvailableMutex);


}

void readJobHandler(job_t job) {
    switch (job->jobType) {
        case WAIT : {
            waitJob(job->params);
            destructJob(job);
            return;
        }
        case MATRIXADD : {
            matrixAddHandler(job);
            return;
        }

        default: break;
    }

    enqueueJob(job);
}
