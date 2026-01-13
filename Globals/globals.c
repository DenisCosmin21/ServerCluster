//
// Created by Denis on 1/5/2026.
//

#include "globals.h"
#include <stddef.h>
#include "../Jobs/Job.h"
#include <Windows.h>

int *workers = NULL;
int totalWorkers = 0;
job_t *assignedJobs = NULL;

queue_t jobQueue;
queue_t responseQueue;
queue_t availableWorkers;

char finishedReading = 0;

CONDITION_VARIABLE commandAvailableCondition;
CONDITION_VARIABLE workerAvailableCondition;
CONDITION_VARIABLE responseAvailableCondition;

CRITICAL_SECTION commandAvailableMutex;
CRITICAL_SECTION workerAvailableMutex;
CRITICAL_SECTION responseAvailableMutex;

prime_t *primes = NULL;
ssize_t maxN = 0;

int rank = 0;

int chunksReachedPerJob[1024];

const char resourcesDirectory[] = "Resources/Responses";