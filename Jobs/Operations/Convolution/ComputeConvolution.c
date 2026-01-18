//
// Created by Denis on 1/18/2026.
//

#include "ComputeConvolution.h"

#include <stdio.h>
#include <string.h>
#include "Kernels.h"
#include "../../../Config/config.h"

imageHeader_t getHeaderInfo(const char *header) {
    imageHeader_t headerInfo;

    char *nextElemPointer = NULL;

    headerInfo.width = strtol(header, &nextElemPointer, 10);

    headerInfo.height = strtol(nextElemPointer, &nextElemPointer, 10);

    headerInfo.paddedWidth = strtol(nextElemPointer, &nextElemPointer, 10);

    headerInfo.paddedHeight = strtol(nextElemPointer, &nextElemPointer, 10);

    while(*nextElemPointer == ' ') {nextElemPointer++;}
    headerInfo.type = nextElemPointer;

    return headerInfo;
}

char *computeConvolution(char *data, char *params, imageHeader_t *headerInfoResult) {
    imageHeader_t headerInfo = getHeaderInfo(params);
    *headerInfoResult = headerInfo;

    kernel_t kernel = getKernel(headerInfo.type);

    char *output = malloc(headerInfo.paddedHeight * headerInfo.paddedWidth * sizeof(char) * 3);

    if(output == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int radius = kernel.size / 2;

#pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < headerInfo.paddedHeight; y++) {
        for (int x = 0; x < headerInfo.paddedWidth; x++) {
            float sumR = 0, sumG = 0, sumB = 0;

            for (int ky = 0; ky < kernel.size; ky++) {
                for (int kx = 0; kx < kernel.size; kx++) {
                    int ix = x + (kx - radius);
                    int iy = y + (ky - radius);

                    if (iy < 0 || iy >= (int)headerInfo.paddedHeight) continue;

                    int rowOffset = iy * (int)headerInfo.paddedWidth;
                    float kVal = kernel.values[ky][kx];

                    if (ix >= 0 && ix < (int)headerInfo.paddedWidth && iy >= 0 && iy < headerInfo.paddedHeight) {
                        int pixelIdx = (iy * headerInfo.paddedWidth + ix) * 3;

                        sumB += (unsigned char)data[pixelIdx + 0] * kVal;
                        sumG += (unsigned char)data[pixelIdx + 1] * kVal;
                        sumR += (unsigned char)data[pixelIdx + 2] * kVal;
                    }
                }
            }

            int outIdx = (y * headerInfo.paddedWidth + x) * 3;
            output[outIdx + 0] = (unsigned char)(sumB < 0 ? 0 : (sumB > 255 ? 255 : sumB));
            output[outIdx + 1] = (unsigned char)(sumG < 0 ? 0 : (sumG > 255 ? 255 : sumG));
            output[outIdx + 2] = (unsigned char)(sumR < 0 ? 0 : (sumR > 255 ? 255 : sumR));
        }
    }

    freeKernel(kernel);

    return output;
}
