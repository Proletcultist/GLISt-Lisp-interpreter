#include <string.h>
#include "lispObject.h"
#include "generalFunctions.h"

int lispSymb_compare(lispSymb l, lispSymb r){
	return strcmp(l.value, r.value) == 0 ? 0 : 1;
}

#define NAME symb_vec
#define TYPE lispSymb

#include "def_vector.h"

int lispObject_p_compare(lispObject *l, lispObject *r){
	if (l->type != r->type){
		return 1;
	}

	if (l->type == LIST_LISP){
		if (((lispList*)l)->list.size != ((lispList*)r)->list.size){
			return 1;
		}
		for (size_t i = 0; i < ((lispList*)l)->list.size; i++){
			if (lispObject_p_compare(((lispList*)l)->list.arr[i], ((lispList*)r)->list.arr[i]) != 0){
				return lispObject_p_compare(((lispList*)l)->list.arr[i], ((lispList*)r)->list.arr[i]);
			}
		}

		return 0;
	}
	else if (l->type == INT_LISP){
		return ((lispInt*)l)->value == ((lispInt*)r)->value ? 0 : (((lispInt*)l)->value < ((lispInt*)r)->value ? -1 : 1);
	}
	else if (l->type == STR_LISP){
		return strcmp(((lispStr*)l)->value, ((lispStr*)r)->value);
	}
	else if (l->type == SYMB_LISP){
		return strcmp(((lispSymb*)l)->value, ((lispSymb*)r)->value);
	}
	else if (l->type == ANON_FUNC_LISP){
		return lispObject_p_compare(((lispAnonFunction*)l)->body, ((lispAnonFunction*)r)->body) == 0 && COMPARE(symb_vec, ((lispAnonFunction*)l)->args, ((lispAnonFunction*)r)->args) == 0 ? 0 : 1;
	}
	else if (l->type == LFUNC_LISP){
		return lispObject_p_compare(((lispLFunction*)l)->body, ((lispLFunction*)r)->body) == 0 && COMPARE(symb_vec, ((lispLFunction*)l)->args, ((lispLFunction*)r)->args) == 0 ? 0 : 1;
	}
	else if (l->type == CFUNC_LISP){
		return ((lispCFunction*)l)->body == ((lispCFunction*)r)->body ? 0 : 1;
	}
	else if (l->type == ERROR_LISP){
		return 1;
	}
}

static void jenkins_hash(const uint8_t* key, size_t len, size_t *hash) {
    for (size_t i = 0; i < len; i++) {
        *hash += key[i];
        *hash += *hash << 10;
        *hash ^= *hash >> 6;
    }
    *hash += *hash << 3;
    *hash ^= *hash >> 11;
    *hash += *hash << 15;
}

static void lispObject_p_hash_inner(lispObject *obj, size_t *out){
	if (obj->type == LIST_LISP){
		for (size_t i = 0; i < ((lispList*)obj)->list.size; i++){
			lispObject_p_hash_inner(((lispList*)obj)->list.arr[i], out);
		}
	}
	else if (obj->type == INT_LISP){
		jenkins_hash((uint8_t*)(&((lispInt*)obj)->value), sizeof(int32_t), out);
	}
	else if (obj->type == STR_LISP){
		jenkins_hash((uint8_t*)((lispStr*)obj)->value, strlen(((lispStr*)obj)->value), out);
	}
	else if (obj->type == SYMB_LISP){
		jenkins_hash((uint8_t*)((lispSymb*)obj)->value, strlen(((lispSymb*)obj)->value), out);
	}
	else if (obj->type == ANON_FUNC_LISP){
		lispObject_p_hash_inner(((lispAnonFunction*)obj)->body, out);
		for (size_t i = 0; i < ((lispAnonFunction*)obj)->args.size; i++){
			lispObject_p_hash_inner((lispObject*)(((lispAnonFunction*)obj)->args.arr + i), out);
		}
	}
	else if (obj->type == LFUNC_LISP){
		lispObject_p_hash_inner(((lispLFunction*)obj)->body, out);
		for (size_t i = 0; i < ((lispLFunction*)obj)->args.size; i++){
			lispObject_p_hash_inner((lispObject*)(((lispLFunction*)obj)->args.arr + i), out);
		}
	}
	else if (obj->type == CFUNC_LISP){
		jenkins_hash((uint8_t*)((lispCFunction*)obj)->body, sizeof(void*), out);
	}
	else if (obj->type == ERROR_LISP){
		return;
	}
}

size_t obj_p_vec_hash(obj_p_vec vec){
	size_t out = 0;
	for (size_t i = 0; i < vec.size; i++){
		lispObject_p_hash_inner(vec.arr[i], &out);
	}
	return out;
}

size_t lispObject_p_hash(lispObject *obj){
	size_t out = 0;
	lispObject_p_hash_inner(obj, &out);
	return out;
}

#define NAME obj_p_vec
#define TYPE lispObject_p

#include "def_vector.h"

lispObject ERROR_OBJECT = {ERROR_LISP, false, NULL};

/*
lispObject* lispObject_copy_construct(lispObject *obj){
	if (obj->type == LIST_LISP){
		lispList *out = malloc(sizeof(lispList));
		memcpy(out, obj, sizeof(lispList));
		out->list = COPY_CONSTRUCT(obj_p_vec, out->list);

		for (size_t i = 0; i < out->list.size; i++){
			out->list.arr[i] = lispObject_copy_construct(out->list.arr[i]);
		}

		out->ref_counter = 1;
		return (lispObject*)out;
	}
	else if (obj->type == INT_LISP){
		lispInt *out = malloc(sizeof(lispInt));
		memcpy(out, obj, sizeof(lispInt));
		out->ref_counter = 1;
		return (lispObject*)out;
	}
	else if (obj->type == STR_LISP){
		lispStr *out = malloc(sizeof(lispStr));
		memcpy(out, obj, sizeof(lispStr));

		out->value = malloc(strlen(((lispStr*)obj)->value) + 1);
		strcpy(out->value, ((lispStr*)obj)->value);
		out->ref_counter = 1;
		return (lispObject*)out;
	}
	else if (obj->type == SYMB_LISP){
		lispSymb *out = malloc(sizeof(lispSymb));
		memcpy(out, obj, sizeof(lispSymb));

		out->value = malloc(strlen(((lispSymb*)obj)->value) + 1);
		strcpy(out->value, ((lispSymb*)obj)->value);
		out->ref_counter = 1;
		return (lispObject*)out;
	}
	else if (obj->type == ANON_FUNC_LISP){
		lispAnonFunction *out = malloc(sizeof(lispAnonFunction));
		memcpy(out, obj, sizeof(lispAnonFunction));

		((context*)out->ctx)->ref_count++;
		out->body = lispObject_copy_construct(out->body);
		out->args = COPY_CONSTRUCT(symb_vec, out->args);
		for (size_t i = 0; i < out->args.size; i++){
			char *buffer = out->args.arr[i].value;
			out->args.arr[i].value = malloc(strlen(out->args.arr[i].value) + 1);
			strcpy(out->args.arr[i].value, buffer);
		}

		out->ref_counter = 1;
		return (lispObject*)out;
	}
	else if (obj->type == LFUNC_LISP){
		lispLFunction *out = malloc(sizeof(lispLFunction));
		memcpy(out, obj, sizeof(lispLFunction));

		((context*)out->ctx)->ref_count++;
		out->args = COPY_CONSTRUCT(symb_vec, out->args);
		out->body = lispObject_copy_construct(out->body);
		for (size_t i = 0; i < out->args.size; i++){
			char *buffer = out->args.arr[i].value;
			out->args.arr[i].value = malloc(strlen(out->args.arr[i].value) + 1);
			strcpy(out->args.arr[i].value, buffer);
		}

		out->ref_counter = 1;
		return (lispObject*)out;
	}
	else if (obj->type == CFUNC_LISP){
		lispCFunction *out = malloc(sizeof(lispCFunction));
		memcpy(out, obj, sizeof(lispCFunction));

		out->ref_counter = 1;
		return (lispObject*)out;
	}
	else if (obj->type == ERROR_LISP){
		return obj;
	}
}
*/

lispObject* lispObject_borrow(lispObject *obj){
	obj->ref_counter++;
	return obj;
}

void printObject(FILE *stream, lispObject *obj){
	if (obj->type == LIST_LISP){
		fprintf(stream, "(");

		if (((lispList*)obj)->list.size != 0){
			printObject(stream, ((lispList*)obj)->list.arr[0]);
		}

		for (size_t i = 1; i < ((lispList*)obj)->list.size; i++){
			fprintf(stream, " ");
			printObject(stream, ((lispList*)obj)->list.arr[i]);
		}

		fprintf(stream, ")");
	}
	else if (obj->type == INT_LISP){
		fprintf(stream, "%"PRId32, ((lispInt*)obj)->value);
	}
	else if (obj->type == STR_LISP){
		fprintf(stream, "%s", ((lispStr*)obj)->value);
	}
	else if (obj->type == SYMB_LISP){
		fprintf(stream, "%s", ((lispSymb*)obj)->value);
	}
	else if (obj->type == ANON_FUNC_LISP){
		fprintf(stream, "<ANON_FUNC>");
	}
	else if (obj->type == LFUNC_LISP){
		fprintf(stream, "<LFUNC>");
	}
	else if (obj->type == CFUNC_LISP){
		fprintf(stream, "<CFUNC>");
	}
	else if (obj->type == ERROR_LISP){
		fprintf(stream, "<ERROR>");
	}
}

void lispObject_destruct(lispObject *obj){

	obj->ref_counter--;
	if (obj->ref_counter != 0){
		return;
	}

	if (obj->type == LIST_LISP){
		for (size_t i = 0; i < ((lispList*)obj)->list.size; i++){
			lispObject_destruct(((lispList*)obj)->list.arr[i]);
		}
		DESTRUCT(obj_p_vec, ((lispList*)obj)->list);
		free(obj);
	}
	else if (obj->type == INT_LISP){
		free(obj);
	}
	else if (obj->type == STR_LISP){
		free(((lispStr*)obj)->value);
		free(obj);
	}
	else if (obj->type == SYMB_LISP){
		free(((lispSymb*)obj)->value);
		free(obj);
	}
	else if (obj->type == ANON_FUNC_LISP){
		for (size_t i = 0; i < ((lispAnonFunction*)obj)->args.size; i++){
			free(((lispAnonFunction*)obj)->args.arr[i].value);
		}
		DESTRUCT(symb_vec, ((lispAnonFunction*)obj)->args);
		putContext((context*)((lispAnonFunction*)obj)->ctx);
		lispObject_destruct(((lispAnonFunction*)obj)->body);
		free(obj);
	}
	else if (obj->type == LFUNC_LISP){
		for (size_t i = 0; i < ((lispLFunction*)obj)->args.size; i++){
			free(((lispLFunction*)obj)->args.arr[i].value);
		}
		DESTRUCT(symb_vec, ((lispLFunction*)obj)->args);
		putContext((context*)((lispLFunction*)obj)->ctx);
		lispObject_destruct(((lispLFunction*)obj)->body);
		memo_p_destruct(((lispLFunction*)obj)->memoiz);
		free(obj);
	}
	else if (obj->type == CFUNC_LISP){
		memo_p_destruct(((lispCFunction*)obj)->memoiz);
		free(obj);
	}
	else if (obj->type == ERROR_LISP){
		return;
	}
}
void massSetSource(lispObject *obj, node *src){
	obj->source = src;
	if (obj->type == LIST_LISP){
		for (size_t i = 0; i < ((lispList*)obj)->list.size; i++){
			massSetSource(((lispList*)obj)->list.arr[i], src);
		}
	}
}
