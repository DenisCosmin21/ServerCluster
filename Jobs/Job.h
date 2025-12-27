//
// Created by Denis on 12/27/2025.
//

#ifndef JOB_H
#define JOB_H

typedef enum {
    WAIT = 2,
    PRIMES,
    PRIMEDIVISORS,
    ANAGRAMS,
    MATRIXADD,
    MATRIXMULT
} jobType_t;

struct job{
    jobType_t jobType;
    char *params;
};

typedef struct job * job_t;

job_t newJob(jobType_t jobType, char *params);

jobType_t getJobType(const char *command);
#endif //JOB_H
