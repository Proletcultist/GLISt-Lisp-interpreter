#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include "parser.h"

typedef enum lispObject_type{
	LIST_LISP,
	INT_LISP,
	STR_LISP,
	SYMB_LISP,
	ANON_LISP
}lispObject_type;

typedef struct lispObject{
	lispObject_type type;
	bool evalable;
}lispObject;

// derived from lispObject

typedef struct lispInt{
	lispObject_type type;
	bool evalable;
	int32_t value;
}lispInt;

typedef struct lispStr{
	lispObject_type type;
	bool evalable;
	char *value;
}lispStr;

typedef struct lispSymb{
	lispObject_type type;
	bool evalable;
	char *value;
}lispSymb;

#define NAME symb_vec
#define TYPE lispSymb

#include "decl_vector.h"

typedef lispObject *lispObject_p;

#define NAME obj_p_vec
#define TYPE lispObject_p

#include "decl_vector.h"

typedef struct lispAnonFunction{
	lispObject_type type;
	bool evalable;
	symb_vec args;
	lispObject *body;
}lispAnonFunction;

typedef struct lispList{
	lispObject_type type;
	bool evalable;
	obj_p_vec list;
}lispList;







typedef struct lispFunction{
	symb_vec args;
	lispObject *body;
	// memo
	bool dirty;
}lispFunction;

typedef struct lispMacro{
	symb_vec args;
	node body;
}lispMacro;

typedef struct cFunction{
	lispObject* (*body)(void *ctx, lispList args);
	bool dirty;
}cFunction;

typedef node (*cMacro)(void *ctx, node args);

void lispObject_destruct(lispObject *obj);
