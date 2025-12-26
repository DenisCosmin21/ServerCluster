//
// Created by Denis on 12/26/2025.
//

#include "Utilities.h"

#include <stdlib.h>

void *allocate(void *buffer, size_t elem_size, size_t *old_size) {
    *old_size = *old_size < 4 ? *old_size + 1 : (*old_size * 2) / 3;

    void *new_buffer = realloc(buffer, *old_size * elem_size);

    if(new_buffer == NULL) {
        perror("realloc");
    }

    buffer = new_buffer;

    return buffer;
}
