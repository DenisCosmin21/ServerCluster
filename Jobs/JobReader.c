//
// Created by Denis on 12/27/2025.
//

#include "JobReader.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "../Utilities/Utilities.h"
#include "Job.h"
#include "../Config/config.h"
static uint64_t currentJobId = 0;

job_t readCommand(FILE* file) {
    if(feof(file))
        return NULL;

    char *command = malloc(10 * sizeof(char)); //Use a allocation strategy for each line, so that we won't allocate too much space

    size_t max_length = 10;

    size_t current_length = 0;

    if(command == NULL) {
        perror("Eroare alocare");
#if MODE == PARALLEL
        MPI_Abort(MPI_COMM_WORLD, 1);
#else
        exit(1);
#endif
    }

    command[current_length++] = fgetc(file);

    while(command[current_length - 1] != '\n' && command[current_length - 1] != EOF && command[current_length - 1] != ' ') {
        if(current_length >= max_length)
            command = allocate(command, sizeof(char), &max_length);

        command[current_length++] = fgetc(file);
    }

    command[current_length - 1] = '\0';

    jobType_t jobType = getJobTypeFromCommand(command);

    current_length = 0;

    command[current_length++] = fgetc(file);

    while(command[current_length - 1] != '\n' && command[current_length - 1] != EOF) {
        if(current_length >= max_length)
            command = allocate(command, sizeof(char), &max_length);

        command[current_length++] = fgetc(file);
    }

    command[current_length - 1] = '\0';

    return newJob(jobType, command, 0x00, currentJobId++, 0);
}
