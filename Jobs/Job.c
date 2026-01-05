//
// Created by Denis on 12/27/2025.
//
#include "Job.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
job_t newJob(jobType_t jobType, char *params, uint64_t jobId, uint64_t chunkId) {
    job_t newJob = malloc(sizeof(struct job));

    if(newJob == NULL) {
        perror("eroare de alocare");
        //MPI_Abort(MPI_COMM_WORLD, 1);
        exit(-1);
    }

    newJob->jobType = jobType;
    newJob->params = params;
    newJob->jobId = jobId;
    newJob->chunkId = chunkId;

    return newJob;
}

jobType_t getJobType(const char *command) {
    if(strcmp(command, "PRIMES") == 0)
        return PRIMES;

    if(strcmp(command, "PRIMEDIVISORS") == 0)
        return PRIMEDIVISORS;

    if(strcmp(command, "ANAGRAMS") == 0)
        return ANAGRAMS;

    if(strcmp(command, "MATRIXADD") == 0)
        return MATRIXADD;

    if(strcmp(command, "MATRIXMULT") == 0)
        return MATRIXMULT;

    return WAIT;
}

void printJob(job_t job) {
    if(job == NULL)
        return;

    printf("Job :\n");
    printf("\tJob Type: %d\n", job->jobType);
    printf("\tJob params: %s\n", job->params);
    printf("\tJob ID: %llu\n", job->jobId);
    printf("\tChunk ID: %llu\n", job->chunkId);

    fflush(stdout);
}