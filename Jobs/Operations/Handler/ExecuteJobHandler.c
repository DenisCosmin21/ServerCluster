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
#include "Operations/Convolution/ComputeConvolution.h"
#include "Operations/Matrix/MatrixProduct/MatrixMult.h"

void cleanupWorker(void) {
    cleanupPrimes();
}

void executeJobHandler(jobType_t jobType, char *params) {
    char *result;
    size_t responseLength = 0;

    switch (jobType) {
        //Finnish the work if the stop work tag is sent
        case STOP: {
            cleanupWorker();
            MPI_Finalize();
            exit(EXIT_SUCCESS);
        }
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
        case CONVOLUTION: {
            char *additionalParams = listenForData(NULL);
            imageHeader_t headerInfo;
            result = computeConvolution(params, additionalParams, &headerInfo);
            responseLength = headerInfo.height * headerInfo.width * 3;
            free(additionalParams);
            break;
        }
        default: {
            result = malloc(20 * sizeof(char));
            strcpy(result, "Job not supported");
            break;
        }
    }

    if(jobType != CONVOLUTION)
        responseLength = strlen(result) + 1;

    MPI_Send(result, responseLength, MPI_CHAR, 0, jobType, MPI_COMM_WORLD);
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
