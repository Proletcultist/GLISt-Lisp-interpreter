#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "interpreterAPI.h"
#include "generalFunctions.h"

void addOperator(context *ctx, char *name, lispObject* (*body)(void *global, void *local, lispCFunction *func, lispList *args), bool dirty){

	if (METHOD(str_obj_p_map, ctx->map, get, name) != NULL){
		fprintf(stderr, "[Loading lib] \033[93mWarning\033[0m Conflicting operators with name '%s'! First loaded will be used\n", name);
		return;
	}

	
	lispCFunction *new = malloc(sizeof(lispCFunction));
	new->ref_counter = 1;
	new->type = CFUNC_LISP;
	new->evalable = false;
	new->body = body;
	new->memoiz = malloc(sizeof(memo));
	*(memo*)new->memoiz = CONSTRUCT(memo);
	new->dirty = dirty;

	char *new_name = malloc(strlen(name) + 1);
	strcpy(new_name, name);

	METHOD(str_obj_p_map, ctx->map, set, new_name, (lispObject*)new);

	addAutocomplete(name);

	fprintf(stdout, "[Loading lib] \033[32mSuccessful\033[0m Loaded '%s' operator\n", name);
}

void bind(context *ctx, char *name, lispObject *obj){
	lispObject **try_find = METHOD(str_obj_p_map, ctx->map, get, name);

	char *bind_name = name;

	if (try_find != NULL){
		lispObject_destruct(*try_find);
	}
	else{
		bind_name = malloc(strlen(name) + 1);
		strcpy(bind_name, name);
		addAutocomplete(name);
	}

	METHOD(str_obj_p_map, ctx->map, set, bind_name, obj);
}

lispObject* lookupToMemo(lispCFunction *func, obj_p_vec key){
	if (func->dirty || func->memoiz == NULL){
		return NULL;
	}
	lispObject **try_find = METHOD(memo, *(memo*)func->memoiz, get, key);

	if (try_find == NULL){
		return NULL;
	}
	else{
		return *try_find;
	}
}

// Copies key vector, borrows objects in this vector and value
void addToMemo(lispCFunction *func, obj_p_vec key, lispObject *value){
	if (func->dirty || func->memoiz == NULL){
		return;
	}

	obj_p_vec new_key = COPY_CONSTRUCT(obj_p_vec, key);
	for (size_t i = 0; i < new_key.size; i++){
		new_key.arr[i] = lispObject_borrow(new_key.arr[i]);
	}

	METHOD(memo, *(memo*)func->memoiz, set, new_key, lispObject_borrow(value));
}
