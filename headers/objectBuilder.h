#pragma once

#include "parser.h"
#include "lispObject.h"

lispObject* visitExpr();

lispObject* visitList();

void visitListContent(lispList *list);

lispObject* visitValue();

lispObject* visitAnonFunc();

void visitSymbList(symb_vec *symbols);
