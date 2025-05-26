#pragma once

#include "generalFunctions.h"
#include "lispObject.h"

typedef char *str;

int str_compare(const char *l, const char *r);
size_t str_hash(char *str);

#define NAME str_obj_p_map
#define KEY_TYPE str
#define VALUE_TYPE lispObject_p

#include "decl_hashMap.h"

typedef struct context{
	str_obj_p_map map;
	size_t ref_count;
}context;

context *context_construct();
context* derive_context(context *ctx);
void putContext(context *ctx);
