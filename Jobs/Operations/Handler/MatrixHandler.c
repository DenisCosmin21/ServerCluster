//
// Created by Denis on 1/13/2026.
//

#include "MatrixHandler.h"

#include "../Handler/ReadJobHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../Config/config.h"
#include "../../Job.h"

static void readMatrixChunk(FILE *file, char *buffer, size_t chunkSize, size_t matrixSize) {
    size_t charsRead = 0;

    for(size_t i = 0; i < chunkSize; i++) {
        fgets(buffer + charsRead, matrixSize * 9, file);
        charsRead = strlen(buffer);
    }
}

static size_t getMatrixSize(FILE *file) { //First 2 elements of the matrix represent the size
    size_t size = 0;

    fscanf(file, "%llu\n", &size);

    return size;
}

void MatrixMultHandler(job_t job) {
    char *params = job->params;
    char *fileName1 = strtok(params, " ");
    char *fileName2 = strtok(NULL, " ");

    FILE *file2 = fopen(fileName2, "r");

    if (file2 == NULL) {
        perror("File not found");
        exit(-1);
    }

    size_t matrixSize = getMatrixSize(file2);

    char *secondMatrix = malloc(matrixSize * sizeof(char) * 9 * matrixSize);

    readMatrixChunk(file2, secondMatrix, matrixSize, matrixSize);

    fclose(file2);

    FILE *file1 = fopen(fileName1, "r");

    matrixSize = getMatrixSize(file1);

    char *firstMatrix = NULL;

    if(matrixSize < MATRIX_SIZE_THRESHOLD) {
        firstMatrix = malloc(matrixSize * sizeof(char) * 9 * matrixSize);

        if (firstMatrix == NULL) {
            perror("malloc");
            exit(-1);
        }

        readMatrixChunk(file1, firstMatrix, matrixSize, matrixSize);

        job_t jobToAdd = newJob(MATRIXMULT, firstMatrix, secondMatrix, job->jobId, 0);

        enqueueJob(jobToAdd);
    }
    else {
        size_t chunk = 1;

        size_t chunkSize = matrixSize / MAX_JOBS_PER_JOB;
        size_t lastChunk = matrixSize % MAX_JOBS_PER_JOB + chunkSize;

        while(chunk <= MAX_JOBS_PER_JOB) {
            firstMatrix = malloc(chunkSize * sizeof(char) * 9 * matrixSize);

            if(firstMatrix == NULL) {
                perror("malloc");
                exit(-1);
            }

            if(chunk == MAX_JOBS_PER_JOB)
                readMatrixChunk(file1, firstMatrix, lastChunk, matrixSize); //If we are on the last job created we should span it on all the lines
            else
                readMatrixChunk(file1, firstMatrix, chunkSize, matrixSize);

            job_t jobToAdd = newJob(MATRIXMULT, firstMatrix, secondMatrix, job->jobId, chunk);

            enqueueJob(jobToAdd);
            chunk++;
        }
    }

     destructJob(job);
}

void matrixAddHandler(job_t job) {

    char *params = job->params;

    char *fileName1 = strtok(params, " ");
    char *fileName2 = strtok(NULL, " ");

    char *firstMatrix = NULL;
    char *secondMatrix = NULL;

    size_t matrixSize = 0;

    FILE *file1 = fopen(fileName1, "r");

    if (file1 == NULL) {
        perror("File not found");
        exit(-1);
    }

    FILE *file2 = fopen(fileName2, "r");

    if (file2 == NULL) {
        perror("File not found");
        exit(-1);
    }

    matrixSize = getMatrixSize(file1);

    if(matrixSize < MATRIX_SIZE_THRESHOLD) {
        matrixSize = getMatrixSize(file2);

        firstMatrix = malloc(matrixSize * sizeof(char) * 9 * matrixSize);

        if (firstMatrix == NULL) {
            perror("malloc");
            exit(-1);
        }

        readMatrixChunk(file1, firstMatrix, matrixSize, matrixSize);

        secondMatrix = malloc(matrixSize * sizeof(char) * 9 * matrixSize);

        if (secondMatrix == NULL) {
            perror("malloc");
            exit(-1);
        }

        readMatrixChunk(file2, secondMatrix, matrixSize, matrixSize);

        job_t jobToAdd = newJob(MATRIXADD, firstMatrix, secondMatrix, job->jobId, 0);

        enqueueJob(jobToAdd);

        destructJob(job);

        return;
    }

    size_t chunk = 1;

    size_t chunkSize = matrixSize / MAX_JOBS_PER_JOB;
    size_t lastChunk = matrixSize % MAX_JOBS_PER_JOB + chunkSize;

    while(chunk <= MAX_JOBS_PER_JOB) {
        firstMatrix = malloc(chunkSize * sizeof(char) * 9 * matrixSize);

        if(firstMatrix == NULL) {
            perror("malloc");
            exit(-1);
        }

        secondMatrix = malloc(chunkSize * sizeof(char) * 9 * matrixSize);

        if(secondMatrix == NULL) {
            perror("malloc");
            exit(-1);
        }

        //If we are on the last job created we should span it on all the lines
        if(chunk == MAX_JOBS_PER_JOB) {
            readMatrixChunk(file1, firstMatrix, lastChunk, matrixSize);
            readMatrixChunk(file2, secondMatrix, lastChunk, matrixSize);
        }
        else {
            readMatrixChunk(file1, firstMatrix, chunkSize, matrixSize);
            readMatrixChunk(file2, secondMatrix, lastChunk, matrixSize);
        }

        job_t jobToAdd = newJob(MATRIXMULT, firstMatrix, secondMatrix, job->jobId, chunk);

        enqueueJob(jobToAdd);

        chunk++;
    }

    destructJob(job);
}


