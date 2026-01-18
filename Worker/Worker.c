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
#include "Operations/Anagrams/Anagrams.h"
#include "../../Globals/globals.h"

char *listenForData(MPI_Status *status) {
    if(status == NULL) {
        MPI_Status localStatus;
        status = &localStatus;
    }

    int size = 0;

    MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, status);

    MPI_Get_count(status, MPI_CHAR, &size);

    char *params = malloc(size * sizeof(char));

    if(params == NULL) {
        perror("Eroare alocare");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Recv(params, size, MPI_CHAR, 0, status->MPI_TAG, MPI_COMM_WORLD, status);

    return params;
}

void runWorker() {
    initAnagrams();
    while(1) {
        MPI_Status status;

        char *params = listenForData(&status);

        jobType_t jobType = status.MPI_TAG;

        executeJobHandler(jobType, params);

        free(params);
    }
}
