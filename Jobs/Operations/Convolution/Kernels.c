//
// Created by Denis on 1/18/2026.
//

#include "Kernels.h"

#include <stdlib.h>
#include <string.h>
#include "../../../Config/config.h"
#include "../../../Worker/Worker.h"

static void generateSharpenKernel(kernel_t *kernel) {
    kernel->values = allocateKernel(kernel->size);

    kernel->values[0][0] = 0;
    kernel->values[0][1] = -1;
    kernel->values[0][2] = 0;
    kernel->values[1][0] = -1;
    kernel->values[1][1] = 5;
    kernel->values[1][2] = -1;
    kernel->values[2][0] = 0;
    kernel->values[2][1] = -1;
    kernel->values[2][2] = 0;
}

static void generateHorizontalEdgeKernel(kernel_t *kernel) {
    kernel->values = allocateKernel(kernel->size);

    kernel->values[0][0] = -1;
    kernel->values[0][1] = -1;
    kernel->values[0][2] = -1;
    kernel->values[1][0] = 0;
    kernel->values[1][1] = 0;
    kernel->values[1][2] = 0;
    kernel->values[2][0] = -1;
    kernel->values[2][1] = -1;
    kernel->values[2][2] = -1;
}

static void generateBoxBlurKernel(kernel_t *kernel) {
    kernel->values = allocateKernel(kernel->size);

    kernel->values[0][0] = (float)1/9;
    kernel->values[0][1] = (float)1/9;
    kernel->values[0][2] = (float)1/9;
    kernel->values[1][0] = (float)1/9;
    kernel->values[1][1] = (float)1/9;
    kernel->values[1][2] = (float)1/9;
    kernel->values[2][0] = (float)1/9;
    kernel->values[2][1] = (float)1/9;
    kernel->values[2][2] = (float)1/9;
}

static void generateGaussianBlurKernel(kernel_t *kernel) {
    kernel->values = allocateKernel(kernel->size);

    kernel->values[0][0] = (float)1/256;
    kernel->values[0][1] = (float)4/256;
    kernel->values[0][2] = (float)6/256;
    kernel->values[0][3] = (float)4/256;
    kernel->values[0][4] = (float)1/256;

    // Row 1
    kernel->values[1][0] = (float)4/256;
    kernel->values[1][1] = (float)16/256;
    kernel->values[1][2] = (float)24/256;
    kernel->values[1][3] = (float)16/256;
    kernel->values[1][4] = (float)4/256;

    // Row 2 (The Center)
    kernel->values[2][0] = (float)6/256;
    kernel->values[2][1] = (float)24/256;
    kernel->values[2][2] = (float)36/256;
    kernel->values[2][3] = (float)24/256;
    kernel->values[2][4] = (float)6/256;

    // Row 3
    kernel->values[3][0] = (float)4/256;
    kernel->values[3][1] = (float)16/256;
    kernel->values[3][2] = (float)24/256;
    kernel->values[3][3] = (float)16/256;
    kernel->values[3][4] = (float)4/256;

    // Row 4
    kernel->values[4][0] = (float)1/256;
    kernel->values[4][1] = (float)4/256;
    kernel->values[4][2] = (float)6/256;
    kernel->values[4][3] = (float)4/256;
    kernel->values[4][4] = (float)1/256;
}

static void generateSimpleKernel(kernel_t *kernel) {
    kernel->values = allocateKernel(kernel->size);

    kernel->values[0][0] = 0;
    kernel->values[0][1] = 0;
    kernel->values[0][2] = 0;
    kernel->values[1][0] = 0;
    kernel->values[1][1] = 1;
    kernel->values[1][2] = 0;
    kernel->values[2][0] = 0;
    kernel->values[2][1] = 0;
    kernel->values[2][2] = 0;
}

float **allocateKernel(const size_t kernelSize) {
    float **kernel = malloc(kernelSize * sizeof(float *));

    if(kernel == NULL) {
        perror("malloc");
#if MODE == PARALLEL
        exitFailedWorker();
#else
        exit(1);
#endif
    }

    for(size_t i = 0; i < kernelSize; i++) {
        kernel[i] = malloc(kernelSize * sizeof(float));
        if(kernel[i] == NULL) {
            perror("malloc");
#if MODE == PARALLEL
            exitFailedWorker();
#else
            exit(1);
#endif
        }
    }

    return kernel;
}

void freeKernel(kernel_t kernel) {
    for(size_t i = 0; i < kernel.size; i++)
        free(kernel.values[i]);

    free(kernel.values);
}

kernel_t getKernel(char *type) {
    kernel_t kernel;

    if(strcmp(type, "sharpen") == 0) {
        kernel.size = 3;
        kernel.stride = 1;

        generateSharpenKernel(&kernel);

        return kernel;
    }

    if(strcmp(type, "horizontal-edge") == 0) {
        kernel.size = 3;
        kernel.stride = 1;

        generateHorizontalEdgeKernel(&kernel);

        return kernel;
    }

    if(strcmp(type, "box-blur") == 0) {
        kernel.size = 3;
        kernel.stride = 1;

        generateBoxBlurKernel(&kernel);

        return kernel;
    }

    if(strcmp(type, "gaussian-blur") == 0) {
        kernel.size = 5;
        kernel.stride = 1;

        generateGaussianBlurKernel(&kernel);

        return kernel;
    }

    kernel.size = 3;
    kernel.stride = 1;

    generateSimpleKernel(&kernel);

    return kernel;
}