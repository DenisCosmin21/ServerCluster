#include "Primes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../Config/config.h"
#include "../../../Globals/globals.h"
#define PARALLEL_THRESHOLD 2000

static void markPrimes(ssize_t currentPrime) {
    ssize_t start = currentPrime * currentPrime;
    if (start >= maxN) return;

    ssize_t numElements = (maxN - start) / currentPrime;

    if (numElements > PARALLEL_THRESHOLD) {
        #pragma omp for schedule(static)
        for (ssize_t i = start; i < maxN; i += currentPrime) {
            primes[i].notPrime = 1;
        }
    } else {
        for (ssize_t i = start; i < maxN; i += currentPrime) {
            primes[i].notPrime = 1;
        }
    }
}

static void markPrimesFromPosition(ssize_t currentPrime, ssize_t lastMax) {
    ssize_t start = ((lastMax + currentPrime - 1) / currentPrime) * currentPrime;

    if (start <= currentPrime) start = currentPrime * 2;
    if (start >= maxN) return;

    ssize_t numElements = (maxN - start) / currentPrime;

    if (numElements > PARALLEL_THRESHOLD) {
        #pragma omp for schedule(static)
        for (ssize_t i = start; i < maxN; i += currentPrime) {
            primes[i].notPrime = 1;
        }
    } else {
        for (ssize_t i = start; i < maxN; i += currentPrime) {
            primes[i].notPrime = 1;
        }
    }
}

char *computePrimes(size_t n) {
    char *buffer = malloc(sizeof(char) * 20);

    if(buffer == NULL) {
        perror("Eroare alocare");
        exit(-1);
    }

    if (primes != NULL && (ssize_t)n < maxN) {
        snprintf(buffer, sizeof(buffer), "%llu", primes[n].countUpTo);
        return buffer;
    }

    ssize_t lastMax = maxN;
    maxN = n + 1;

    prime_t *newPrimes = realloc(primes, maxN * sizeof(prime_t));
    if (newPrimes == NULL) {
        perror("Eroare alocare realloc");
        exit(-1);
    }
    primes = newPrimes;

    memset(primes + lastMax, 0, (maxN - lastMax) * sizeof(prime_t));

    if (lastMax == 0) {
        primes[0].notPrime = 1;
        primes[1].notPrime = 1;
        lastMax = 2;
    }

    #pragma omp parallel num_threads(THREAD_COUNT) default(none) \
                         shared(primes, maxN, lastMax)
    {
        for (ssize_t i = 2; i < lastMax; i++) {
            if (primes[i].notPrime == 0) {
                markPrimesFromPosition(i, lastMax);
            }
        }

        for (ssize_t i = lastMax; i * i < maxN; i++) {
            if (primes[i].notPrime == 0) {
                markPrimes(i);
            }
        }
    }

    for (ssize_t i = (lastMax > 1 ? lastMax : 2); i < maxN; i++) {
        primes[i].countUpTo = primes[i - 1].countUpTo + (primes[i].notPrime == 0 ? 1 : 0);
    }

    snprintf(buffer, 20, "%llu", primes[n].countUpTo);
    return buffer;
}

void cleanupPrimes(void) {
    if(primes != NULL) {
        free(primes);
        maxN = 0;
    }
}