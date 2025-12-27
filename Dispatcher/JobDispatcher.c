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

static int *workers = NULL;
static int totalWorkers = 0;
static queue_t jobQueue;
static queue_t responseQueue;
static queue_t availableWorkers;
static char finishedReading = 0;

static void printInt(void *data) {
    int *i = (int *)data;
    printf("%d\n", *i);
}

static void printString(void *data) {
    char *str = (char *)data;
    printf("%s\n", str);
}

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

static void *readCommands(void *dummy) {
    FILE *commandFile = fopen("../Resources/commands.txt", "r");

    if (commandFile == NULL) {
        perror("Error opening command file\n");
        exit(-1);
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
        int *worker = peek(availableWorkers);
        //Wait until a worker is available
        if(worker == NULL)
            continue;

        char *command = dequeue(jobQueue);

        if(command != NULL){
            worker = dequeue(availableWorkers);
            printf("%d\n", *worker);
            printf("%s\n", command);
        }
    }

    return NULL;
}

static void *saveResponses(void *dummy) {
    FILE *responseFile = fopen("../Resources/responses.txt", "w");

    if(responseFile == NULL) {
        perror("Error opening response file\n");
        exit(-1);
    }

/*We can have response if we haven't finished reading, or if we haven't finished dispathcing all jobs,
 or if we haven't printed all responses*/

    while(finishedReading == 0 || !is_empty(responseQueue) || !is_empty(jobQueue) || get_size(availableWorkers) != totalWorkers) {
        char *response = dequeue(responseQueue);
        if(response != NULL)
            fprintf(responseFile, "%s\n", response);
    }

    fclose(responseFile);

    return NULL;
}

static void *getResponses(void *dummy) {
    char *response = NULL;
    int responseSize = 0;
    int rank = 0;

    MPI_Status status;

    /*We can have response if we haven't finished reading, or if we haven't finished dispathcing all jobs*/

    while(finishedReading == 0 || !is_empty(jobQueue)) {
        MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

        MPI_Get_count(&status, MPI_CHAR, &responseSize);

        rank = status.MPI_SOURCE;

        response = malloc(responseSize * sizeof(char));

        if(response == NULL) {
            perror("Eroare alocare");
            exit(-1);
        }

        enqueue(responseQueue, response);

        enqueue(availableWorkers, &workers[rank - 1]);
    }

    return NULL;
}

static void initAvailableWorkers(void) {
    queue_init(&availableWorkers);

    MPI_Comm_size(MPI_COMM_WORLD, &totalWorkers);

    workers = malloc(totalWorkers * sizeof(int));
    if(workers == NULL) {
        perror("Eroare alocare");
        exit(-1);
    }

    for(int i = 0; i < totalWorkers - 1; i++) {
        workers[i] = i + 1;
        enqueue(availableWorkers, &workers[i]);
    }
}

void runDispatcher(void) { //used to initialize the queue for the dispatcher and start the threads and wait for them
    queue_init(&jobQueue);
    queue_init(&responseQueue);
    initAvailableWorkers();
    pthread_t readThread;
    pthread_t dispatchThread;
    pthread_create(&readThread, NULL, readCommands, NULL);
    pthread_create(&dispatchThread, NULL, dispatchCommands, NULL);
    pthread_join(readThread, NULL);
    pthread_join(dispatchThread, NULL);
}