//
// Created by Denis on 12/27/2025.
//

#include "Worker.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Jobs/Job.h"

static void handleCommand(job_t job) {
    char *param = job->params;
    char result[100];
    sprintf(result, "%d %s", job->jobType, param);
    printf("%s\n", result);
    fflush(stdout);
    MPI_Send(result, strlen(result) + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
}

void runWorker() {
    int size = 0;

    MPI_Status status;

    while(1) {
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        //Finnish the work if the stop work tag is sent
        if(status.MPI_TAG == STOP_WORKING) {
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

        MPI_Recv(params, size + 1, MPI_CHAR, 0, status.MPI_TAG, MPI_COMM_WORLD, &status);

        jobType_t jobType = status.MPI_TAG;

        job_t job = newJob(jobType, params);

        handleCommand(job);

        free(job->params);
        free(job);
    }
}
