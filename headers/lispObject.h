#pragma once

#include "parser.h"

typedef enum lispObject_type{
	LIST_LISP,
	INT_LISP,
	STR_LISP,
	SYMB_LISP
}lispObkect_type;

typedef struct lispList{
}lispList;

typedef struct lispInt{
}lispInt;

typedef struct lispStr{
}lispStr;

typedef struct lispSymb{
}lispSymb;

typedef union lispObject_value{
}lispObject_value;

typedef struct lispObject{
}lispObject;






typedef enum function_type{
	LISP_FUNCTION,
	C_FUNCTION
}function_type;

typedef enum macro_type{
	LISP_MACRO,
	C_MACRO
}macro_type;

typedef struct lispFunction{
	lispList args;
	lispObject body;
}lispFunction;

typedef struct lispMacro{
	lispList args;
	node body;
}lispMacro;

typedef lispObject (*cFunction)(/*env*/, lispList args);

typedef node (*cMacro)(/*env*/, node args);
