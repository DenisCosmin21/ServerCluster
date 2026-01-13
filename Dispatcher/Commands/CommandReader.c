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

static void printQueueJob(void *job) {
    printJob(job);
}

DWORD WINAPI readCommands(LPVOID lpParam) {
    FILE *commandFile = fopen("Resources\\commands.txt", "r");

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
    return 0;
}