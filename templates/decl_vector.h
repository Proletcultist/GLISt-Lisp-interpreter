#ifndef _CONCAT
	#define _CONCAT(a,b) a##b
#endif

#ifndef CONCAT
	#define CONCAT(a,b) _CONCAT(a,b)
#endif

#if defined(NAME) && defined(TYPE)

typedef struct NAME{
	TYPE *arr;
	size_t cap, size;
} NAME;

NAME CONCAT(NAME, _construct)();
void CONCAT(NAME, _destruct)(NAME vec);
NAME CONCAT(NAME, _copy_construct)(NAME vec);
NAME CONCAT(NAME, _move_construct)(NAME vec);
int CONCAT(NAME, _compare)(NAME left, NAME right);
size_t CONCAT(NAME, _push)(NAME *vec, TYPE val);
TYPE CONCAT(NAME, _pop)(NAME *vec);
TYPE CONCAT(NAME, _peek)(NAME *vec);
void CONCAT(NAME, _reserve)(NAME *vec, size_t cap);
void CONCAT(NAME, _resize)(NAME *vec, size_t newCnt, TYPE fill);
void CONCAT(NAME, _insert)(NAME *vec, size_t where, TYPE *arr, size_t num);
void CONCAT(NAME, _erase)(NAME *vec, size_t where, size_t num);

#undef NAME
#undef TYPE

#endif
