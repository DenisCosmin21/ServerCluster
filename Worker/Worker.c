//
// Created by Denis on 12/27/2025.
//

#include "Worker.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void handleCommand(char *command) {
    strtok(command, " ");

    char *param = strtok(NULL, " ");
    MPI_Send(param, strlen(param) + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
}

void runWorker() {
    char *command;
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

        command = malloc(size * sizeof(char));

        if(command == NULL) {
            perror("Eroare alocare");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        MPI_Recv(command, size + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);

        handleCommand(command);

        free(command);
    }
}
