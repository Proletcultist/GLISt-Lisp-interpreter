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
	LMACRO_LISP,
	CMACRO_LISP
}lispObject_type;

typedef struct lispObject{
	lispObject_type type;
	bool evalable;
}lispObject;

// derived from lispObject

typedef struct lispInt{
	lispObject_type type;
	bool evalable;
	node *source;
	int32_t value;
}lispInt;

typedef struct lispStr{
	lispObject_type type;
	bool evalable;
	node *source;
	char *value;
}lispStr;

typedef struct lispSymb{
	lispObject_type type;
	bool evalable;
	node *source;
	char *value;
}lispSymb;

int lispSymb_compare(lispSymb l, lispSymb r);

#define NAME symb_vec
#define TYPE lispSymb

#include "decl_vector.h"

typedef lispObject *lispObject_p;
int lispObject_p_compare(lispObject *l, lispObject *r);

#define NAME obj_p_vec
#define TYPE lispObject_p

#include "decl_vector.h"

typedef struct lispList{
	lispObject_type type;
	bool evalable;
	node *source;

	obj_p_vec list;
}lispList;

typedef struct lispAnonFunction{
	lispObject_type type;
	bool evalable;
	node *source;

	symb_vec args;
	lispObject *body;
}lispAnonFunction;

typedef struct lispLFunction{
	lispObject_type type;
	bool evalable;
	node *source;

	symb_vec args;
	lispObject *body;
	bool dirty;
	// memo
}lispLFunction;

typedef struct lispCFunction{
	lispObject_type type;
	bool evalable;

	lispObject* (*body)(void *ctx, lispList *args);
	bool dirty;
}lispCFunction;

typedef struct lispLMacro{
	lispObject_type type;
	bool evalable;

	symb_vec args;
	node* body;
}lispLMacro;

typedef struct lispCMacro{
	lispObject_type type;
	bool evalable;

	node* (*body)(void *ctx, node *args);
}lispCMacro;

void lispObject_destruct(lispObject *obj);
