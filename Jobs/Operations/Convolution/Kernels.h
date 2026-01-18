//
// Created by Denis on 1/18/2026.
//

#ifndef KERNELS_H
#define KERNELS_H
#include <stddef.h>

typedef struct Kernel {
    float **values;
    size_t size;
    size_t stride;
}kernel_t;

void freeKernel(kernel_t kernel);

kernel_t getKernel(char *type);

float **allocateKernel(const size_t kernelSize);
#endif //KERNELS_H
