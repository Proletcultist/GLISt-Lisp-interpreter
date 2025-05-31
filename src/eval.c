#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eval.h"
#include "lispContext.h"
#include "lispObject.h"
#include "ASTtools.h"

#define MAX_DEPTH 1000

static lispObject* evalList(context *global, context *local, lispList *list);
static lispObject* evalSymb(context *global, context *local, lispSymb *symbol);
static lispObject* applyLFunction(context *global, context *local, lispLFunction *func, lispList *list);
static lispObject* applyCFunction(context *global, context *local, lispCFunction *func, lispList *list);
static lispObject* applyAnonFunction(context *global, context *local, lispAnonFunction *func, lispList *list);

void throughError(node *source){
	if (istoken(*source)){
		source->value.token_val.type = ERROR_TOKEN;
	}
	else{
		source->value.nonterminal_val.type = ERROR_NT;
	}
	spreadError(source);
}

static lispObject* evalList(context *global, context *local, lispList *list){
	if (list->list.size == 0){
		lispSymb *out = malloc(sizeof(lispSymb));
		out->type = SYMB_LISP;
		out->evalable = false;
		out->ref_counter = 1;
		out->value = malloc(4);
		strcpy(out->value, "NIL");
		return (lispObject*)out;
	}

	lispObject *first = eval(global, local, list->list.arr[0]);
	if (first->type == ERROR_LISP){
		return &ERROR_OBJECT;
	}

	lispObject *out;
	if (first->type == LFUNC_LISP){
		out = applyLFunction(global, local, (lispLFunction*)first, list);
	}
	else if (first->type == CFUNC_LISP){
		out = applyCFunction(global, local, (lispCFunction*)first, list);
	}
	else if (first->type == ANON_FUNC_LISP){
		out = applyAnonFunction(global, local, (lispAnonFunction*)first, list);
	}
	else{
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Incorrect expression, first element isn't functional object\n");
		throughError(first->source);
		out = &ERROR_OBJECT;
	}

	lispObject_destruct(first);

	return out;
}

static lispObject* evalSymb(context *global, context *local, lispSymb *symbol){
	lispObject **try_find;

	if (local != NULL){
		try_find = METHOD(str_obj_p_map, local->map, get, symbol->value);

		if (try_find != NULL){
			return lispObject_borrow(*try_find);
		}
	}

	try_find = METHOD(str_obj_p_map, global->map, get, symbol->value);

	if (try_find != NULL){
		return lispObject_borrow(*try_find);
	}

	fprintf(stderr, "[Evaluating] \033[31mError\033[0m symbol is unbound\n");
	throughError(symbol->source);

	return &ERROR_OBJECT;
}

static lispObject* applyLFunction(context *global, context *local, lispLFunction *func, lispList *list){
	static size_t depth = 0;

	depth++;

	if (depth >= MAX_DEPTH){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Exceeded max depth!\n");
		throughError(list->source);

		depth--;
		return &ERROR_OBJECT;
	}

	if (list->list.size != func->args.size + 1){
		fprintf(stderr, "[Evaluatin] \033[31mError\033[0m Wrong amount of arguments, expected: %zu, got: %zu\n", func->args.size, list->list.size - 1);
		throughError(list->source);

		depth--;
		return &ERROR_OBJECT;
	}

	bool terminate = false;
	
	// Evaluating args
	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);
	for (size_t i = 0; i < func->args.size; i++){
		lispObject *buffer = eval(global, local, list->list.arr[i + 1]);
		if (buffer->type == ERROR_LISP){
			terminate = true;
			break;
		}

		METHOD(obj_p_vec, evaluated, push, buffer);
	}
	if (terminate){
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		depth--;
		return &ERROR_OBJECT;
	}

	// Lookup to memo
	bool add_to_memo = false;
	if (func->memoiz != NULL && !func->dirty){
		lispObject **try_find = METHOD(memo, *(memo*)func->memoiz, get, evaluated);

		if (try_find == NULL){
			add_to_memo = true;
		}
		else{
			for (size_t i = 0; i < evaluated.size; i++){
				lispObject_destruct(evaluated.arr[i]);
			}
			DESTRUCT(obj_p_vec, evaluated);
			depth--;
			return lispObject_borrow(*try_find);
		}
	}

	// Save previous bindings of args
	obj_p_vec olds = CONSTRUCT(obj_p_vec);
	for (size_t i = 0; i < func->args.size; i++){
		lispObject **old = METHOD(str_obj_p_map, ((context*)func->ctx)->map, get, func->args.arr[i].value);
		if (old == NULL){
			METHOD(obj_p_vec, olds, push, NULL);
		}
		else{
			METHOD(obj_p_vec, olds, push, *old);
		}
	}
	
	// Binding arguments in function context
	for (size_t i = 0; i < func->args.size; i++){
		METHOD(str_obj_p_map, ((context*)func->ctx)->map, set, func->args.arr[i].value, lispObject_borrow(evaluated.arr[i]));
	}
	massSetSource(func->body, func->source);
	lispObject *out = eval(global, (context*)func->ctx, func->body);

	// Return previous bindings
	for (size_t i = 0; i < func->args.size; i++){
		lispObject **buffer = METHOD(str_obj_p_map, ((context*)func->ctx)->map, get, func->args.arr[i].value);
		if (buffer != NULL){
			lispObject_destruct(*buffer);
		}

		if (olds.arr[i] != NULL){
			METHOD(str_obj_p_map, ((context*)func->ctx)->map, set, func->args.arr[i].value, olds.arr[i]);
		}
		else{
			METHOD(str_obj_p_map, ((context*)func->ctx)->map, remove, func->args.arr[i].value);
		}
	}
	DESTRUCT(obj_p_vec, olds);

	if (out->type == ERROR_LISP && depth == 1){
		fprintf(stderr, "when evaluating function\n");
	}

	if (add_to_memo && out->type != ERROR_LISP){
		lispObject *value = lispObject_borrow(out);
		METHOD(memo, *(memo*)func->memoiz, set, evaluated, value);
	}
	else{
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
	}

	depth--;
	return out;
}

static lispObject* applyCFunction(context *global, context *local, lispCFunction *func, lispList *list){
	// Memo should be added by author of function
	return func->body(global, local, func, list);
}

static lispObject* applyAnonFunction(context *global, context *local, lispAnonFunction *func, lispList *list){
	static size_t depth = 0;

	depth++;

	if (depth >= MAX_DEPTH){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Exceeded max depth!\n");
		throughError(list->source);

		depth--;
		return &ERROR_OBJECT;
	}

	if (list->list.size != func->args.size + 1){
		fprintf(stderr, "[Evaluatin] \033[31mError\033[0m Wrong amount of arguments, expected: %zu, got: %zu\n", func->args.size, list->list.size - 1);
		throughError(list->source);
		depth--;
		return &ERROR_OBJECT;
	}

	bool terminate = false;
	
	// Evaluating args
	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);
	for (size_t i = 0; i < func->args.size; i++){
		lispObject *buffer = eval(global, local, list->list.arr[i + 1]);
		if (buffer->type == ERROR_LISP){
			terminate = true;
			break;
		}

		METHOD(obj_p_vec, evaluated, push, buffer);
	}
	if (terminate){
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		depth--;
		return &ERROR_OBJECT;
	}

	// Save previous bindings of args
	obj_p_vec olds = CONSTRUCT(obj_p_vec);
	for (size_t i = 0; i < func->args.size; i++){
		lispObject **old = METHOD(str_obj_p_map, ((context*)func->ctx)->map, get, func->args.arr[i].value);
		if (old == NULL){
			METHOD(obj_p_vec, olds, push, NULL);
		}
		else{
			METHOD(obj_p_vec, olds, push, *old);
		}
	}
	
	// Binding arguments in function context
	for (size_t i = 0; i < func->args.size; i++){
		METHOD(str_obj_p_map, ((context*)func->ctx)->map, set, func->args.arr[i].value, lispObject_borrow(evaluated.arr[i]));
	}
	massSetSource(func->body, func->source);
	lispObject *out = eval(global, (context*)func->ctx, func->body);

	// Return previous bindings
	for (size_t i = 0; i < func->args.size; i++){
		lispObject **buffer = METHOD(str_obj_p_map, ((context*)func->ctx)->map, get, func->args.arr[i].value);
		if (buffer != NULL){
			lispObject_destruct(*buffer);
		}

		if (olds.arr[i] != NULL){
			METHOD(str_obj_p_map, ((context*)func->ctx)->map, set, func->args.arr[i].value, olds.arr[i]);
		}
		else{
			METHOD(str_obj_p_map, ((context*)func->ctx)->map, remove, func->args.arr[i].value);
		}
	}
	DESTRUCT(obj_p_vec, olds);

	if (out->type == ERROR_LISP){
		fprintf(stderr, "when evaluating function\n");
	}

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	depth--;
	return out;
}

lispObject* eval(context *global, context *local, lispObject *obj){
	if (!obj->evalable){
		return lispObject_borrow(obj);
	}

	lispObject *out;
	if (obj->type == LIST_LISP){
		out = evalList(global, local, (lispList*)obj);
	}
	else if (obj->type == INT_LISP){
		out = lispObject_borrow(obj);
	}
	else if (obj->type == STR_LISP){
		out = lispObject_borrow(obj);
	}
	else if (obj->type == SYMB_LISP){
		out = evalSymb(global, local, (lispSymb*)obj);
	}
	else if (obj->type == ERROR_LISP){
		return &ERROR_OBJECT;
	}
	else{
		return lispObject_borrow(obj);
	}
	
	if (out->type == ERROR_LISP){
		return &ERROR_OBJECT;
	}

	out->source = obj->source;

	return out;
}
