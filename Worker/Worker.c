//
// Created by Denis on 12/27/2025.
//

#include "Worker.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Jobs/Job.h"
#include "../Jobs/Operations/Handler/ExecuteJobHandler.h"
#include "../Jobs/Operations/Primes/Primes.h"

static void cleanupWorker(void) {
    cleanupPrimes();
}

void runWorker() {
    int size = 0;

    MPI_Status status;

    while(1) {
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        //Finnish the work if the stop work tag is sent
        if(status.MPI_TAG == STOP_WORKING) {
            cleanupWorker();
            char finishBuff;
            MPI_Recv(&finishBuff, 1, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            break;
        }

        MPI_Get_count(&status, MPI_CHAR, &size);

        char *params = malloc(size * sizeof(char));

        if(params == NULL) {
            perror("Eroare alocare");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        MPI_Recv(params, size, MPI_CHAR, 0, status.MPI_TAG, MPI_COMM_WORLD, &status);

        jobType_t jobType = status.MPI_TAG;

        executeJobHandler(jobType, params);
    }
}
