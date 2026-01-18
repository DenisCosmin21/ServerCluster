//
// Created by Denis on 12/27/2025.
//

#ifndef JOB_H
#define JOB_H
#include <stdint.h>

typedef enum {
    STOP = 1,
    WAIT,
    PRIMES,
    PRIMEDIVISORS,
    ANAGRAMS,
    MATRIXADD,
    MATRIXMULT,
    CONVOLUTION
} jobType_t;

struct job{
    jobType_t jobType;
    char *params;
    char *additionalParam;
    uint64_t jobId;
    uint64_t chunkId;
};

typedef struct job * job_t;

job_t newJob(jobType_t jobType, char *params, char *additionalParam, uint64_t jobId, uint64_t chunkId);

jobType_t getJobTypeFromCommand(const char *command);

char* getJobType(job_t job);

void printJob(job_t job);

void destructJob(job_t job);
#endif //JOB_H
