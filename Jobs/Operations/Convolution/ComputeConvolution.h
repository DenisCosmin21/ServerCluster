//
// Created by Denis on 1/18/2026.
//

#ifndef COMPUTECONVOLUTION_H
#define COMPUTECONVOLUTION_H
#include <stddef.h>
#include <stdlib.h>

typedef struct imageHeader {
    size_t width;
    size_t height;
    size_t paddedWidth;
    size_t paddedHeight;
    size_t totalWidth;
    size_t totalHeight;
    char *type;
}imageHeader_t;

char *computeConvolution(char *data, char *params, imageHeader_t *headerInfoResult);

imageHeader_t getHeaderInfo(const char *header);
#endif //COMPUTECONVOLUTION_H
