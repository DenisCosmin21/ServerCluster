//
// Created by Denis on 12/27/2025.
//

#include "JobReader.h"

#include <stdio.h>
#include <stdlib.h>
#include <Utilities.h>
#include "Job.h"

job_t readCommand(FILE* file) {
    if(feof(file))
        return NULL;

    char *command = malloc(10 * sizeof(char)); //Use a allocation strategy for each line, so that we won't allocate too much space

    size_t max_length = 10;

    size_t current_length = 0;

    if(command == NULL) {
        perror("Eroare alocare");
        exit(-1);
    }

    command[current_length++] = fgetc(file);

    while(command[current_length - 1] != '\n' && command[current_length - 1] != EOF && command[current_length - 1] != ' ') {
        if(current_length >= max_length)
            command = allocate(command, sizeof(char), &max_length);

        command[current_length++] = fgetc(file);
    }

    command[current_length - 1] = '\0';

    jobType_t jobType = getJobType(command);

    current_length = 0;

    while(command[current_length - 1] != '\n' && command[current_length - 1] != EOF) {
        if(current_length >= max_length)
            command = allocate(command, sizeof(char), &max_length);

        command[current_length++] = fgetc(file);
    }

    command[current_length - 1] = '\0';

    return newJob(jobType, command);
}
