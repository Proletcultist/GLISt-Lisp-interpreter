#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include "eval.h"
#include "lispObject.h"
#include "lispContext.h"
#include "parser.h"
#include "generalFunctions.h"
#include "repl.h"

void addOperator(context *ctx, char *name, lispObject* (*body)(void *global, void *local, lispCFunction *func, lispList *args), bool dirty);

void bind(context *ctx, char *name, lispObject *obj);

lispObject* lookupToMemo(lispCFunction *func, obj_p_vec key);

void addToMemo(lispCFunction *func, obj_p_vec key, lispObject *value);

