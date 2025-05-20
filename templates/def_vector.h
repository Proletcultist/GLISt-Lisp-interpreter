#include <stdlib.h>
#include <memory.h>

#ifndef _CONCAT
	#define _CONCAT(a,b) a##b
#endif

#ifndef CONCAT
	#define CONCAT(a,b) _CONCAT(a,b)
#endif

#ifndef CALLMACRO
	#define CALLMACRO(m, ...) m(__VA_ARGS__)
#endif

#if defined(NAME) && defined(TYPE)

NAME CONCAT(NAME, _construct)(){ 
	return (NAME){NULL, 0, 0}; 
} 

void CONCAT(NAME, _destruct)(NAME vec){ 
	if (vec.arr != NULL){ 
		free(vec.arr); 
		vec.arr = NULL; 
		vec.cap = 0; 
		vec.size = 0; 
	} 
} 

NAME CONCAT(NAME, _copy_construct)(NAME vec){ 
	NAME out = CALLMACRO(CONSTRUCT, NAME); 
	
	CALLMACRO(METHOD, NAME, out, insert, 0, vec.arr, vec.size); 
	
	return out; 
} 

NAME CONCAT(NAME, _move_construct)(NAME vec){ 
	NAME out = CALLMACRO(CONSTRUCT, NAME); 
	out.arr = vec.arr; 
	out.size = vec.size; 
	out.cap = vec.cap; 
	
	vec.arr = NULL; 
	vec.cap = 0; 
	vec.size = 0; 
	
	return out; 
} 

int CONCAT(NAME, _compare)(NAME left, NAME right){
	if (left.size < right.size){
		return -1;
	}
	else if (left.size > right.size){
		return 1;
	}

	for (size_t i = 0; i < left.size; i++){
		if (CALLMACRO(COMPARE, TYPE, left.arr[i], right.arr[i]) < 0){
			return -1;
		}
		else if (CALLMACRO(COMPARE, TYPE, left.arr[i], right.arr[i]) < 0){
			return 1;
		}
	}

	return 0;
}

size_t CONCAT(NAME, _push)(NAME *vec, TYPE val){ 
	if (vec->cap == 0){ 
		vec->arr = malloc(sizeof(TYPE)); 
		vec->cap = 1; 
	} 
	else if (vec->cap == vec->size){ 
		vec->arr = realloc(vec->arr, vec->cap * 2 * sizeof(TYPE)); 
		vec->cap *= 2; 
	} 
	vec->arr[vec->size] = val; 

	return vec->size++; 
} 

TYPE CONCAT(NAME, _pop)(NAME *vec){ 
	return vec->arr[--vec->size]; 
} 

TYPE CONCAT(NAME, _peek)(NAME *vec){ 
	return vec->arr[vec->size - 1]; 
} 

void CONCAT(NAME, _reserve)(NAME *vec, size_t cap){ 
	if (vec->cap < cap){ 
		vec->arr = realloc(vec->arr, cap * sizeof(TYPE)); 
		vec->cap = cap; 
	} 
} 

void CONCAT(NAME, _resize)(NAME *vec, size_t newCnt, TYPE fill){ 
	CONCAT(NAME, _reserve)(vec, newCnt); 

	while (vec->size < newCnt){ 
		vec->arr[vec->size++] = fill; 
	} 

	vec->size = newCnt; 
} 

void CONCAT(NAME, _insert)(NAME *vec, size_t where, TYPE *arr, size_t num){ 
	if (num <= 0){ 
		return; 
	} 

	CONCAT(NAME, _reserve)(vec, vec->size + num); 

	memmove(vec->arr + where + num, vec->arr + where, (vec->size - where) * sizeof(TYPE)); 
	memcpy(vec->arr + where, arr, num * sizeof(TYPE)); 

	vec->size += num; 
} 

void CONCAT(NAME, _erase)(NAME *vec, size_t where, size_t num){ 
	if (vec->size >= num && num > 0){ 
		memmove(vec->arr + where, vec->arr + where + num, (vec->size - where - num) * sizeof(TYPE)); 
		vec->size -= num; 
	} 
} 

#undef NAME
#undef TYPE

#endif
