//
// Created by Denis on 1/13/2026.
//

#include "MatrixMult.h"

#include <globals.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../Config/config.h"
#include "../../../Worker/Worker.h"

static size_t getMatrixSize(const char *matrix2, char **matrixAfterSize) {
    return strtol(matrix2, matrixAfterSize, 10);
}

static void printMatrix(const long **matrix, size_t matrixSize, size_t columnSize) {
    for(size_t i = 0;i < columnSize;i++) {
        for(size_t j = 0;j < matrixSize;j++)
            printf("%ld ", matrix[i][j]);
        printf("\n");
    }
}

static size_t computeMatrixProductAsLong(long** resultMatrix, char* matrix1, char* matrix2, const size_t matrixSize) {
    char *ptrElem1 = matrix1;
    char *ptrElem2 = matrix2;
    char *nextElem1 = NULL;
    char *nextElem2 = matrix2;
    long elem1 = 0;
    long elem2 = 0;
    size_t posMatrix1 = 0;
    size_t posMatrix2 = 0;

    resultMatrix[0] = (long *)calloc(matrixSize, sizeof(long));

    memset(resultMatrix[0], 0, matrixSize * sizeof(long));

    while(*ptrElem1 != '\0') {

        elem1 = strtol(ptrElem1, &nextElem1, 10);

        posMatrix2 = 0;

        while(*nextElem2 != '\n' && *nextElem2 != '\0') {
            elem2 = strtol(ptrElem2, &nextElem2, 10);
            resultMatrix[posMatrix1][posMatrix2] += elem1 * elem2;
            posMatrix2++;
            ptrElem2 = nextElem2;
        }

        nextElem2++;

        ptrElem1 = nextElem1;

        if(*nextElem1 == '\n'){
            ptrElem1++;
            nextElem2 = matrix2;
            ptrElem2 = matrix2;
            posMatrix1++;

            resultMatrix[posMatrix1] = (long *)calloc(matrixSize, sizeof(long));

            if(resultMatrix[posMatrix1] == NULL) {
                perror("Error allocating resultMatrix");
#if MODE == PARALLEL
                exitFailedWorker();
#else
                exit(1);
#endif
            }

            memset(resultMatrix[posMatrix1], 0, matrixSize * sizeof(long));
        }
    }

    return posMatrix1;
}

static char *transformLongMatrixToString(long **matrix, size_t matrixSize, size_t columnSize) {
    char *buffer = (char *)malloc(matrixSize * sizeof(char) * columnSize * 9 + matrixSize * columnSize);

    size_t bytesWritten = 0;

    for(size_t i = 0;i <= columnSize;i++) {
        for(size_t j = 0;j < matrixSize;j++) {
            bytesWritten += snprintf(buffer + bytesWritten,  matrixSize * columnSize * 9 + matrixSize * columnSize,"%ld ", matrix[i][j]);
        }
        bytesWritten += snprintf(buffer + bytesWritten, matrixSize * columnSize * 9 + matrixSize * columnSize, "\n");
    }

    buffer[bytesWritten - 1] = '\0';

    return buffer;
}

char *computeMatrixMult(char *matrix1, char *matrix2) {
    char *matrixAfterSize = NULL;

    size_t matrixSize = getMatrixSize(matrix2, &matrixAfterSize);

    matrixAfterSize++;

    matrix2 = matrixAfterSize;

    long **resultMatrix = (long **)malloc(matrixSize * sizeof(long*));

    if(resultMatrix == NULL) {
        perror("Failed to allocate memory for resultMatrix");
        #if MODE == PARALLEL
        exitFailedWorker();
#else
        exit(1);
#endif
    }

    size_t columnSize = computeMatrixProductAsLong(resultMatrix, matrix1, matrix2, matrixSize);

    char *result = transformLongMatrixToString(resultMatrix, matrixSize, columnSize);

    for(size_t i = 0;i < columnSize;i++) {
        free(resultMatrix[i]);
    }

    free(resultMatrix);

    return result;
}