//
// Created by Denis on 1/5/2026.
//

#include "CommandReader.h"
#include "../../Jobs/Operations/Handler/ReadJobHandler.h"
#include "../../Globals/globals.h"
#include "../../Jobs/Job.h"
#include "../../Jobs/JobReader.h"
#include <stdio.h>
#include <Windows.h>

#include "../../Logger/Logger.h"

static void printQueueJob(void *job) {
    printJob(job);
}

DWORD WINAPI readCommands(LPVOID lpParam) {
    FILE *commandFile = fopen("Resources\\commands.txt", "r");

    char log[1024];

    sprintf_s(log, 1024, "%s\n", "Starting reading commands");

    logData(log);

    if (commandFile == NULL) {
        perror("Error opening command file\n");
        exit(-1);
    }

    job_t job = NULL;

    while((job = readCommand(commandFile)) != NULL) {
        readJobHandler(job);
    }

    finishedReading = 1;
    fclose(commandFile);

    sprintf(log, "Finished reading commands\n");
    logData(log);
    return 0;
}