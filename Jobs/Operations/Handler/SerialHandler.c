//
// Created by Denis on 1/14/2026.
//

#include "SerialHandler.h"

#include <stdio.h>
#include "../../../Logger/Logger.h"
#include "ExecuteJobHandler.h"
#include "../../../Config/config.h"
#include "../../../Globals/globals.h"

static char baseFileName[] = "Resources\\Responses\\response";

void executeJob(job_t job) {
    char log[2096];

    char *response = executeSerialJobHandler(job->jobType, job->params, job->additionalParam);

    sprintf(log, "Saving Job with Id = %llu; and chunk = %llu\n", job->jobId, job->chunkId);

    logData(log);

    char fileName[256];

    sprintf(fileName, "%s%llu.txt", baseFileName, job->jobId);

    FILE *responseFile = fopen(fileName, "a");

    if(responseFile == NULL) {
        perror("Error opening response file\n");
        exit(-1);
    }

    fprintf(responseFile, "%s\n", response);
    fflush(responseFile);

    fclose(responseFile);

    destructJob(job);

    free(response);
}
