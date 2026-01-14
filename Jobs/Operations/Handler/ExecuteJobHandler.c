//
// Created by Denis on 1/11/2026.
//

#include "ExecuteJobHandler.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Worker/Worker.h"
#include "../Primes/Primes.h"
#include "../PrimeDivizors/PrimeDivisors.h"
#include "../../Job.h"
#include "Operations/Anagrams/Anagrams.h"
#include "Operations/Matrix/MatrixAdd/MatrixAdd.h"
#include "../../Globals/globals.h"
#include "Operations/Matrix/MatrixProduct/MatrixMult.h"

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
        case MATRIXADD: {
            char *additionalParams = listenForData(NULL);
            result = computeMatrixAdd(params, additionalParams);
            free(additionalParams);
            break;
        }
        case MATRIXMULT: {
            char *additionalParams = listenForData(NULL);
            result = computeMatrixMult(params, additionalParams);
            free(additionalParams);
            break;
        }
        default: {
            result = malloc(20 * sizeof(char));
            strcpy(result, "Job not supported");
            break;
        }
    }

    MPI_Send(result, strlen(result) + 1, MPI_CHAR, 0, jobType, MPI_COMM_WORLD);

    free(result);
}

char *executeSerialJobHandler(jobType_t jobType, char *params, char *additionalParams) {
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
        case MATRIXADD: {
            result = computeMatrixAdd(params, additionalParams);
            break;
        }
        case MATRIXMULT: {
            result = computeMatrixMult(params, additionalParams);
            break;
        }
        default: {
            result = malloc(20 * sizeof(char));
            strcpy(result, "Job not supported");
            break;
        }
    }

    return result;
}
