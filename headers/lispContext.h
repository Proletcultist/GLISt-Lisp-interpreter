#pragma once

#include "generalFunctions.h"
#include "lispObject.h"

typedef char *str;

int str_compare(const char *l, const char *r);
size_t str_hash(char *str);

#define NAME str_lispFunc_map
#define KEY_TYPE str
#define VALUE_TYPE lispFunction

#include "decl_hashMap.h"

#define NAME str_lispMacro_map
#define KEY_TYPE str
#define VALUE_TYPE lispMacro

#include "decl_hashMap.h"

#define NAME str_cFunc_map
#define KEY_TYPE str
#define VALUE_TYPE cFunction

#include "decl_hashMap.h"

#define NAME str_cMacro_map
#define KEY_TYPE str
#define VALUE_TYPE cMacro

#include "decl_hashMap.h"

typedef struct context{
	str_lispFunc_map lFuncs;
	str_cFunc_map cFuncs;
	str_lispMacro_map lMacros;
	str_cMacro_map cMacros;
}context;
