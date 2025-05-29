#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include "parser.h"

typedef enum lispObject_type{
	LIST_LISP,
	INT_LISP,
	STR_LISP,
	SYMB_LISP,
	ANON_FUNC_LISP,
	LFUNC_LISP,
	CFUNC_LISP,
	ERROR_LISP
}lispObject_type;

typedef struct lispObject{
	lispObject_type type;
	bool evalable;
	node *source;
	size_t ref_counter;
}lispObject;

extern lispObject ERROR_OBJECT;

typedef lispObject *lispObject_p;

// Returns only 0 if equal and 1 otherwise
int lispObject_p_compare(lispObject *l, lispObject *r);
size_t lispObject_p_hash(lispObject *obj);

#include "lispContext.h"

// derived from lispObject

typedef struct lispInt{
	lispObject_type type;
	bool evalable;
	node *source;
	size_t ref_counter;
	int32_t value;
}lispInt;

typedef struct lispStr{
	lispObject_type type;
	bool evalable;
	node *source;
	size_t ref_counter;
	char *value;
}lispStr;

typedef struct lispSymb{
	lispObject_type type;
	bool evalable;
	node *source;
	size_t ref_counter;
	char *value;
}lispSymb;

int lispSymb_compare(lispSymb l, lispSymb r);

#define NAME symb_vec
#define TYPE lispSymb

#include "decl_vector.h"

#define NAME obj_p_vec
#define TYPE lispObject_p

#include "decl_vector.h"

size_t obj_p_vec_hash(obj_p_vec vec);

#include "memo.h"

typedef struct lispList{
	lispObject_type type;
	bool evalable;
	node *source;
	size_t ref_counter;

	obj_p_vec list;
}lispList;

typedef struct lispAnonFunction{
	lispObject_type type;
	bool evalable;
	node *source;
	size_t ref_counter;

	void *ctx;
	symb_vec args;
	lispObject *body;
}lispAnonFunction;

typedef struct lispLFunction{
	lispObject_type type;
	bool evalable;
	node *source;
	size_t ref_counter;

	symb_vec args;
	void *ctx;
	lispObject *body;
	bool dirty;
	void *memoiz;
}lispLFunction;

typedef struct lispCFunction{
	lispObject_type type;
	bool evalable;
	node *source;
	size_t ref_counter;

	lispObject* (*body)(void *global, void *local, struct lispCFunction *func, lispList *args);
	bool dirty;
	void *memoiz;
}lispCFunction;

void massSetSource(lispObject *obj, node *src);
void printObject(FILE *stream, lispObject *obj);
void lispObject_destruct(lispObject *obj);
//lispObject* lispObject_copy_construct(lispObject *obj);
lispObject* lispObject_borrow(lispObject *obj);
