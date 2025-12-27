//
// Created by Denis on 12/27/2025.
//
#include "Job.h"

#include <mpi.h>
#include <stdlib.h>
#include <string.h>

job_t newJob(jobType_t jobType, char *params) {
    job_t newJob = malloc(sizeof(job_t));

    if(newJob == NULL) {
        perror("eroare de alocare");
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(-1);
    }

    newJob->jobType = jobType;
    newJob->params = params;

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
