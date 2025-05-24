#include "lispObject.h"

int lispSymb_compare(lispSymb l, lispSymb r){ return 0; }

#define NAME symb_vec
#define TYPE lispSymb

#include "def_vector.h"

int lispObject_p_compare(lispObject *l, lispObject *r){ return 0; }

#define NAME obj_p_vec
#define TYPE lispObject_p

#include "def_vector.h"

void lispObject_destruct(lispObject *obj);
