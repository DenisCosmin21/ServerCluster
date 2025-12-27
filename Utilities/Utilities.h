//
// Created by Denis on 12/26/2025.
//

#ifndef UTILITIES_H
#define UTILITIES_H
#define STOP_WORKING 1
#include <stddef.h>

void *allocate(void *buffer, size_t elem_size, size_t *old_size);

void printInt(void *data);

void printString(void *data);
#endif //UTILITIES_H
