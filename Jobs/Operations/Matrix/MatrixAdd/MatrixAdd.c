//
// Created by Denis on 1/13/2026.
//

#include "MatrixAdd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *computeMatrixAdd(char *matrix1, char *matrix2) {
    char *outputBuffer = malloc(strlen(matrix1) + strlen(matrix2)); //A little more space to make sure it won't overflow

    if (outputBuffer == NULL) {
        perror("malloc");
        exit(-1);
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

        bufferPos += snprintf(outputBuffer + bufferPos, resultSize + 1 , "%ld ", result);

        ptrElem1 = nextElem1;
        ptrElem2 = nextElem2;

        if(*ptrElem1 == '\n') {
            strcpy(outputBuffer + bufferPos, "\n");
            bufferPos++;
            strcpy(outputBuffer + bufferPos, "\0");
        }
    }

    return outputBuffer;
}
