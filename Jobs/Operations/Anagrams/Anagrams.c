//
// Created by Denis on 1/12/2026.
//

#include "Anagrams.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t factorials[9];

static int compare(const void *a, const void *b) {
    return *(char *)a - *(char *)b;
}

void initAnagrams(void) {
    factorials[0] = 1;
    factorials[1] = 1;
}

static size_t factorial(size_t number) {
    if(factorials[number] == 0)
        factorials[number] = factorial(number - 1) * number;

    return factorials[number];
}

static void generateRecursive(char *currentStr, int left, int right, char *buffer, size_t *index_ptr, size_t str_len) {
    if (left == right) {
        // Critical section to ensure only one thread writes to the index at a time
#pragma omp critical
        {
            memcpy(buffer + (*index_ptr * (str_len + 1)), currentStr, str_len);
            buffer[(*index_ptr * (str_len + 1)) + str_len] = '\n';
            (*index_ptr)++;
        }
        return;
    }

    for (int i = left; i <= right; i++) {
        int duplicate = 0;
        for (int j = left; j < i; j++) {
            if (currentStr[j] == currentStr[i]) {
                duplicate = 1;
                break;
            }
        }

        if (!duplicate) {
            char temp = currentStr[left];
            currentStr[left] = currentStr[i];
            currentStr[i] = temp;

            generateRecursive(currentStr, left + 1, right, buffer, index_ptr, str_len);

            // Backtrack
            temp = currentStr[left];
            currentStr[left] = currentStr[i];
            currentStr[i] = temp;
        }
    }
}

char *computeAnagrams(char *input) {
    size_t n = strlen(input);

    for(size_t i = 0; i < n; i++) {
        if(input[i] >= 'A' && input[i] <= 'Z') input[i] += 32;
    }
    qsort(input, n, sizeof(char), compare);

    size_t divisor = 1;
    size_t letterCount = 0;
    for(size_t i = 0; i <= n; i++) {
        if(i > 0 && (i == n || input[i] != input[i-1])) {
            divisor *= factorial(letterCount);
            letterCount = 0;
        }
        if (i < n) letterCount++;
    }
    size_t totalPermutations = factorial(n) / divisor;

    char *buffer = malloc(totalPermutations * (n + 1) + 1);
    if (!buffer) return NULL;

    size_t global_index = 0;

    // 4. Parallel Generation
    // We parallelize the first letter choice to distribute the work
    #pragma omp parallel for schedule(dynamic) default(none) shared(n, input, global_index, buffer)
        for (int i = 0; i < n; i++) {
            // Check for duplicates at the top level
            int is_dup = 0;
            for (int j = 0; j < i; j++) {
                if (input[j] == input[i]) {
                    is_dup = 1;
                    break;
                }
            }

            if (!is_dup) {
                char *local_str = strdup(input); // Each thread needs its own copy
                char temp = local_str[0];
                local_str[0] = local_str[i];
                local_str[i] = temp;

                generateRecursive(local_str, 1, n - 1, buffer, &global_index, n);
                free(local_str);
            }
        }

    buffer[totalPermutations * (n + 1)] = '\0'; // Null terminate the massive buffer
    return buffer;
}