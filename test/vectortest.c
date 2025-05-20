#include <stdio.h>
#include "generalFunctions.h"

#define NAME int_vec
#define TYPE int

#include "decl_vector.h"

int int_compare(int l, int r){
	return (l < r ? -1 : (l > r ? 1 : 0));
}

#define NAME int_vec
#define TYPE int

#include "def_vector.h"

int main(){
	int_vec vec = CONSTRUCT(int_vec);

	METHOD(int_vec, vec, push, 1);
	METHOD(int_vec, vec, push, 2);

	printf("%d\n", METHOD(int_vec, vec, pop));

	for (size_t i = 0; i < vec.size; i++){
		printf("%d ", vec.arr[i]);
	}
	printf("\n");

	DESTRUCT(int_vec, vec);
	return 0;
}
