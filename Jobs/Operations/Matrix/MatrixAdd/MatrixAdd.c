//
// Created by Denis on 1/13/2026.
//

#include "MatrixAdd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../Config/config.h"
#include "../../../Worker/Worker.h"

char *computeMatrixAdd(char *matrix1, char *matrix2) {
    size_t totalBufferSize = strlen(matrix1) + strlen(matrix2) + 100;

    char *outputBuffer = malloc(strlen(matrix1) + strlen(matrix2)); //A little more space to make sure it won't overflow

    if (outputBuffer == NULL) {
        perror("malloc");
#if MODE == PARALLEL
        exitFailedWorker();
#else
        exit(1);
#endif
    }

    long elem1 = 0, elem2 = 0, result = 0;

    size_t bufferPos = 0;

    char *nextElem1, *nextElem2;

    char *ptrElem1 = matrix1;

    char *ptrElem2 = matrix2;

    while(*ptrElem1 != '\0') {
        elem1 = strtol(ptrElem1, &nextElem1, 10);

        elem2 = strtol(ptrElem2, &nextElem2, 10);

        size_t resultSize = (nextElem1 - ptrElem1) + (nextElem2 - ptrElem2);

        if(ptrElem1 == nextElem1) {
            ptrElem1++;
            ptrElem2++;
            continue;
        }

        result = elem1 + elem2;

        int written = snprintf(outputBuffer + bufferPos, totalBufferSize - bufferPos , "%ld ", result);

        if (written > 0) {
            bufferPos += written;
        }

        ptrElem1 = nextElem1;
        ptrElem2 = nextElem2;

        if(*ptrElem1 == '\n') {
            strcpy(outputBuffer + bufferPos, "\n");
            bufferPos++;
            strcpy(outputBuffer + bufferPos, "\0");
        }
    }

    outputBuffer[bufferPos - 1] = '\0';
    return outputBuffer;
}
