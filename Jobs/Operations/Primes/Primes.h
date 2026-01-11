//
// Created by Denis on 1/11/2026.
//

#ifndef PRIMES_H
#define PRIMES_H
#include <stddef.h>

typedef struct {
    size_t countUpTo;
    short notPrime;
} prime_t;

char *computePrimes(size_t n);

void cleanupPrimes(void);
#endif //PRIMES_H
