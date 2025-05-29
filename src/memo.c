#include "memo.h"

#include <stdlib.h>
#include "lispObject.h"

#define NAME memo
#define KEY_TYPE obj_p_vec
#define VALUE_TYPE lispObject_p

#include "def_hashMap.h"

static void destructAllObjects(memo *m){
	for (size_t i = 0; i < m->size; i++){
		if (m->arr[i].type == VALUE_NODE){
			lispObject_destruct(m->arr[i].value);
			for (size_t j = 0; j < m->arr[i].key.size; j++){
				lispObject_destruct(m->arr[i].key.arr[j]);
			}
			DESTRUCT(obj_p_vec, m->arr[i].key);
		}
	}
}

void memo_p_destruct(memo *m){
	if (m == NULL){
		return;
	}
	destructAllObjects(m);
	DESTRUCT(memo, *m);
	free(m);
}
