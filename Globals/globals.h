//
// Created by Denis on 1/5/2026.
//

#ifndef GLOBALS_H
#define GLOBALS_H
#include "../Queue/DoubleLinkedListQueue.h"
#include "../Jobs/Job.h"
#include <Windows.h>
#include "Primes.h"

extern int * workers;

extern job_t *assignedJobs;

extern int totalWorkers;

extern queue_t jobQueue;

extern queue_t responseQueue;

extern queue_t availableWorkers;

extern char finishedReading;

extern CONDITION_VARIABLE commandAvailableCondition;
extern CONDITION_VARIABLE workerAvailableCondition;
extern CONDITION_VARIABLE responseAvailableCondition;

extern CRITICAL_SECTION commandAvailableMutex;
extern CRITICAL_SECTION workerAvailableMutex;
extern CRITICAL_SECTION responseAvailableMutex;

extern prime_t *primes;
extern ssize_t maxN;

extern int rank;

extern int chunksReachedPerJob[1024];

extern const char resourcesDirectory[];
#endif //GLOBALS_H
