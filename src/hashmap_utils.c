#include <stddef.h>

int isPrime(size_t n){
	for (size_t i = 2;i * i <= n;i++){
		if (n % i == 0){
			return 0;
		}
	}
	return 1;
}

size_t nextPrime(size_t n){
	while (!isPrime(n)){
		n++;
	}
	return n;
}

void swapSize_t(size_t *l, size_t *r){
	*l ^= *r;
	*r ^= *l;
	*l ^= *r;
}

size_t calcProbDist(size_t pos, size_t probe_start, size_t size){
	if (pos >= probe_start){
		return pos - probe_start;
	}
	else{
		return (size - probe_start + pos);
	}
}
