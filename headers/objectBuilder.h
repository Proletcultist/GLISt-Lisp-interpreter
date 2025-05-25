#pragma once

#include "parser.h"
#include "lispObject.h"

lispObject* visitExpr(node *src);

lispObject* visitList(node *src);

void visitListContent(node *src, lispList *list);

lispObject* visitValue(node *src);
