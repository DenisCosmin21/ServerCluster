//
// Created by Denis on 1/18/2026.
//

#include "Kernels.h"

#include <stdlib.h>
#include <string.h>

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

    kernel->values[0][0] = (float)1/16;
    kernel->values[0][1] = (float)2/16;
    kernel->values[0][2] = (float)1/16;
    kernel->values[1][0] = (float)2/16;
    kernel->values[1][1] = (float)4/16;
    kernel->values[1][2] = (float)2/16;
    kernel->values[2][0] = (float)1/16;
    kernel->values[2][1] = (float)2/16;
    kernel->values[2][2] = (float)1/16;
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
        exit(EXIT_FAILURE);
    }

    for(size_t i = 0; i < kernelSize; i++) {
        kernel[i] = malloc(kernelSize * sizeof(float));
        if(kernel[i] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
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

        generateSharpenKernel(&kernel);

        return kernel;
    }

    if(strcmp(type, "box-blur") == 0) {
        kernel.size = 3;
        kernel.stride = 1;

        generateBoxBlurKernel(&kernel);

        return kernel;
    }

    if(strcmp(type, "gaussian-blur") == 0) {
        kernel.size = 3;
        kernel.stride = 1;

        generateGaussianBlurKernel(&kernel);

        return kernel;
    }

    kernel.size = 3;
    kernel.stride = 1;

    generateSimpleKernel(&kernel);

    return kernel;
}