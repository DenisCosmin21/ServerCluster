//
// Created by Denis on 1/6/2026.
//

#include "ReadJobHandler.h"
#include <Windows.h>
#include "../../../Queue/DoubleLinkedListQueue.h"
#include "../../../Globals/globals.h"
#include "../Sleep/WaitJob.h"

void readJobHandler(job_t job) {
    switch (job->jobType) {
        case WAIT : {
            waitJob(job->params);
            return;
        }

        default: break;
    }

    EnterCriticalSection(&commandAvailableMutex);
    enqueue(jobQueue, job);
    WakeConditionVariable(&commandAvailableCondition);
    LeaveCriticalSection(&commandAvailableMutex);
}
