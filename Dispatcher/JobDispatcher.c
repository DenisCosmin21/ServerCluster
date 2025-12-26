//
// Created by Denis on 12/26/2025.
//

#include "JobDispatcher.h"
#include <pthread_time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include "../Utilities/Utilities.h"
#include "../Queue/DoubleLinkedListQueue.h"
#include <pthread.h>

static queue_t jobQueue;
static char finishedReading = 0;

static char *readCommand(FILE* file) {
    if(feof(file))
        return NULL;

    char *line = malloc(10 * sizeof(char)); //Use a allocation strategy for each line, so that we won't allocate too much space

    size_t max_length = 10;

    size_t current_length = 0;

    if(line == NULL) {
        perror("Eroare alocare");
        exit(-1);
    }

    line[current_length++] = fgetc(file);

    while(line[current_length - 1] != '\n' && line[current_length - 1] != EOF) {
        if(current_length >= max_length)
            line = allocate(line, sizeof(char), &max_length);

        line[current_length++] = fgetc(file);
    }

    line[current_length - 1] = '\0';

    return line;
}

static void handleCommand(char *command) {
    if(strstr(command, "WAIT")) {
        strtok(command, " ");
        char *timeToSleep = strtok(NULL, " ");
        printf("Waiting for %s seconds\n", timeToSleep);
        Sleep(strtol(timeToSleep, NULL, 10) * 1000);
        return;
    }

    enqueue(jobQueue, command);
}

static void *runDispatcher(void *dummy) {
    FILE *commandFile = fopen("../Resources/commands.txt", "r");

    if (commandFile == NULL) {
        printf("Error opening command file\n");
        return NULL;
    }

    char *line = NULL;

    while((line = readCommand(commandFile)) != NULL) {
        handleCommand(line);
    }

    finishedReading = 1;

    fclose(commandFile);

    return NULL;
}

static void *dispatchCommands(void *dummy) {
    while(finishedReading == 0 || !is_empty(jobQueue)) {
        char *command = dequeue(jobQueue);
        if(command != NULL)
            printf("%s\n", command);
    }

    return NULL;
}

void initializeDispatcher(void) { //used to initialize the queue for the dispatcher and start the threads
    queue_init(&jobQueue);
    pthread_t readThread;
    pthread_t dispatchThread;
    pthread_create(&readThread, NULL, runDispatcher, NULL);
    pthread_create(&dispatchThread, NULL, dispatchCommands, NULL);
    pthread_join(readThread, NULL);
    pthread_join(dispatchThread, NULL);
}