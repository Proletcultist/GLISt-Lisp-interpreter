#pragma once

#include "lispObject.h"

#define NAME memo
#define KEY_TYPE obj_p_vec
#define VALUE_TYPE lispObject_p

#include "decl_hashMap.h"

void memo_p_destruct(memo *m);
