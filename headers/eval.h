#pragma once

#include "lispObject.h"

lispObject* eval(context *global, context *local, lispObject *obj);
bool check(context *global, context *local, lispObject *obj);
void throughError(node *source);
