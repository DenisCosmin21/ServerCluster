//
// Created by Denis on 1/12/2026.
//

#include "PrimeDivisors.h"

#include <stdio.h>

#include "../../../Globals/globals.h"

char *computePrimeDivizorsCount(size_t n) {

    char *buffer = malloc(sizeof(char) * 20);

    if(buffer == NULL) {
        perror("Eroare alocare");
        exit(-1);
    }

    size_t result = 0;

    if(n % 2 == 0 && n != 2) {
        result = 1;
    }

    for(size_t i = 3;i * i <= n;i+=2) {
        if(n % i == 0) {
            if(i >= maxN)
                computePrimes(n / 2 + 1); //Compute all other values if i exceds the last element computed to not recompute each time.
            //We can sacrifice a little memory for some performance, and we might need it later anyways

            if(primes[i].notPrime == 0)
                result++;

            if(n/i >= maxN)
                computePrimes(n / 2 + 1);

            if(primes[n / i].notPrime == 0)
                result++;
        }
    }

    snprintf(buffer, sizeof(buffer), "%llu", result);

    return buffer;
}