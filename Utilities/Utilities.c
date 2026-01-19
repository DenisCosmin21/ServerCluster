//
// Created by Denis on 12/26/2025.
//

#include "Utilities.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void *allocate(void *buffer, size_t elem_size, size_t *old_size) {
    *old_size = *old_size < 4 ? *old_size + 1 : (*old_size / 2) * 3;

    void *new_buffer = realloc(buffer, *old_size * elem_size);

    if(new_buffer == NULL) {
        perror("Not enough memory");
#if MODE == PARALLEL
        MPI_Abort(MPI_COMM_WORLD, 1);
#else
        exit(1);
#endif
    }

    buffer = new_buffer;

    return buffer;
}

void printInt(void *data) {
    int *i = (int *)data;
    printf("%d ", *i);
}

void printString(void *data) {
    char *str = (char *)data;
    printf("%s\n", str);
}
