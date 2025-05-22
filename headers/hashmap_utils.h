#pragma once

#include <stddef.h>

int isPrime(size_t n);

size_t nextPrime(size_t n);

void swapSize_t(size_t *l, size_t *r);

size_t calcProbDist(size_t pos, size_t probe_start, size_t size);
