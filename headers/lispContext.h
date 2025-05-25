#pragma once

#include "generalFunctions.h"
#include "lispObject.h"

typedef char *str;

int str_compare(const char *l, const char *r);
size_t str_hash(char *str);

#define NAME context
#define KEY_TYPE str
#define VALUE_TYPE lispObject_p

#include "decl_hashMap.h"

context* derive_context(context *ctx);
void destructAllObjects(context *ctx);
