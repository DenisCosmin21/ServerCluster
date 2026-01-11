//
// Created by Denis on 1/11/2026.
//

#include "ExecuteJobHandler.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Primes/Primes.h"
#include "../PrimeDivizors/PrimeDivisors.h"
#include "../../Job.h"
#include "Operations/Anagrams/Anagrams.h"


void executeJobHandler(jobType_t jobType, char *params) {
    char *result;

    switch (jobType) {
        case PRIMES: {
            ssize_t maxPrime = strtol(params, NULL, 10);
            result = computePrimes(maxPrime);
            break;
        }
        case PRIMEDIVISORS: {
            ssize_t number = strtol(params, NULL, 10);
            result = computePrimeDivizorsCount(number);
            break;
        }
        case ANAGRAMS: {
            result = computeAnagrams(params);
            break;
        }
        default: {
            result = malloc(20 * sizeof(char));
            strcpy(result, "Job not supported");
            break;
        }
    }

    fflush(stdout);
    MPI_Send(result, strlen(result) + 1, MPI_CHAR, 0, jobType, MPI_COMM_WORLD);
    free(result);
}
