#include <string.h>
#include "lispObject.h"
#include "generalFunctions.h"

int lispSymb_compare(lispSymb l, lispSymb r){ return 0; }

#define NAME symb_vec
#define TYPE lispSymb

#include "def_vector.h"

int lispObject_p_compare(lispObject *l, lispObject *r){ return 0; }

#define NAME obj_p_vec
#define TYPE lispObject_p

#include "def_vector.h"

lispObject ERROR_OBJECT = {ERROR_LISP, false, NULL};

lispObject* lispObject_copy_construct(lispObject *obj){
	if (obj->type == LIST_LISP){
		lispList *out = malloc(sizeof(lispList));
		memcpy(out, obj, sizeof(lispList));
		out->list = COPY_CONSTRUCT(obj_p_vec, out->list);

		for (size_t i = 0; i < out->list.size; i++){
			out->list.arr[i] = lispObject_copy_construct(out->list.arr[i]);
		}

		return (lispObject*)out;
	}
	else if (obj->type == INT_LISP){
		lispInt *out = malloc(sizeof(lispInt));
		memcpy(out, obj, sizeof(lispInt));
		return (lispObject*)out;
	}
	else if (obj->type == STR_LISP){
		lispStr *out = malloc(sizeof(lispStr));
		memcpy(out, obj, sizeof(lispStr));

		out->value = malloc(strlen(((lispStr*)obj)->value) + 1);
		strcpy(out->value, ((lispStr*)obj)->value);
		return (lispObject*)out;
	}
	else if (obj->type == SYMB_LISP){
		lispSymb *out = malloc(sizeof(lispSymb));
		memcpy(out, obj, sizeof(lispSymb));

		out->value = malloc(strlen(((lispSymb*)obj)->value) + 1);
		strcpy(out->value, ((lispSymb*)obj)->value);
		return (lispObject*)out;
	}
	else if (obj->type == ANON_FUNC_LISP){
		lispAnonFunction *out = malloc(sizeof(lispAnonFunction));
		memcpy(out, obj, sizeof(lispAnonFunction));

		out->body = lispObject_copy_construct(out->body);
		out->args = COPY_CONSTRUCT(symb_vec, out->args);

		return (lispObject*)out;
	}
	else if (obj->type == LFUNC_LISP){
		lispLFunction *out = malloc(sizeof(lispLFunction));
		memcpy(out, obj, sizeof(lispLFunction));

		out->args = COPY_CONSTRUCT(symb_vec, out->args);
		out->body = lispObject_copy_construct(out->body);

		return (lispObject*)out;
	}
	else if (obj->type == CFUNC_LISP){
		lispCFunction *out = malloc(sizeof(lispCFunction));
		memcpy(out, obj, sizeof(lispCFunction));

		return (lispObject*)out;
	}
}

void printObject(FILE *stream, lispObject *obj){
	if (obj->type == LIST_LISP){
		fprintf(stream, "<LIST>");
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
		DESTRUCT(symb_vec, ((lispAnonFunction*)obj)->args);
		destructAllObjects((context*)((lispAnonFunction*)obj)->ctx);
		lispObject_destruct(((lispAnonFunction*)obj)->body);
		free(obj);
	}
	else if (obj->type == LFUNC_LISP){
		DESTRUCT(symb_vec, ((lispLFunction*)obj)->args);
		destructAllObjects((context*)((lispLFunction*)obj)->ctx);
		lispObject_destruct(((lispLFunction*)obj)->body);
		free(obj);
	}
	else if (obj->type == CFUNC_LISP){
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
