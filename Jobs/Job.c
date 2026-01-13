//
// Created by Denis on 12/27/2025.
//
#include "Job.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

job_t newJob(jobType_t jobType, char *params, char *additionalParam, uint64_t jobId, uint64_t chunkId) {
    job_t newJob = malloc(sizeof(struct job));

    if(newJob == NULL) {
        perror("eroare de alocare");
        //MPI_Abort(MPI_COMM_WORLD, 1);
        exit(-1);
    }

    newJob->jobType = jobType;
    newJob->params = params;
    newJob->additionalParam = additionalParam;
    newJob->jobId = jobId;
    newJob->chunkId = chunkId;

    return newJob;
}

jobType_t getJobTypeFromCommand(const char *command) {
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

char *getJobType(job_t job) {
    switch(job->jobType) {
        case PRIMES: return "PRIMES";
        case PRIMEDIVISORS: return "PRIMEDIVISORS";
        case ANAGRAMS: return "ANAGRAMS";
        case MATRIXADD: return "MATRIXADD";
        case MATRIXMULT: return "MATRIXMULT";
        case WAIT: return "WAIT";
        default: return "WAIT";
    }
}
void destructJob(job_t job) {
    if(job->params != NULL)
        free(job->params);

    if(job->additionalParam != NULL)
        free(job->additionalParam);

    free(job);
}

void printJob(job_t job) {
    if(job == NULL)
        return;

    printf("Job :\n");
    printf("\tJob Type: %d\n", job->jobType);
    printf("\tJob params: %s\n", job->params);
    if(job->additionalParam != NULL)
        printf("\tAdditional param: %s\n", job->additionalParam);
    printf("\tJob ID: %llu\n", job->jobId);
    printf("\tChunk ID: %llu\n", job->chunkId);

    fflush(stdout);
}