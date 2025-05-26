#include <stdlib.h>
#include <stdio.h>
#include "eval.h"
#include "lispContext.h"
#include "lispObject.h"
#include "ASTtools.h"

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
		out->value = "NIL";
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
			return lispObject_copy_construct(*try_find);
		}
	}

	try_find = METHOD(str_obj_p_map, global->map, get, symbol->value);

	if (try_find != NULL){
		return lispObject_copy_construct(*try_find);
	}

	fprintf(stderr, "[Evaluating] \033[31mError\033[0m symbol is unbound\n");
	throughError(symbol->source);

	return &ERROR_OBJECT;
}

static lispObject* applyLFunction(context *global, context *local, lispLFunction *func, lispList *list){
	if (list->list.size != func->args.size + 1){
		fprintf(stderr, "[Evaluatin] \033[31mError\033[0m Wrong amount of arguments\n");
		throughError(list->source);
		return &ERROR_OBJECT;
	}

	// TODO: Lookup to the memo
	
	// Evaluating args
	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);
	for (size_t i = 0; i < func->args.size; i++){
		lispObject *buffer = eval(global, local, list->list.arr[i + 1]);
		if (buffer->type == ERROR_LISP){
			return &ERROR_OBJECT;
		}

		METHOD(obj_p_vec, evaluated, push, buffer);
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
		METHOD(str_obj_p_map, ((context*)func->ctx)->map, set, func->args.arr[i].value, evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);
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

	return out;
}

static lispObject* applyCFunction(context *global, context *local, lispCFunction *func, lispList *list){
	// TODO: Lookup to the memo
	return func->body(global, local, list);
}

static lispObject* applyAnonFunction(context *global, context *local, lispAnonFunction *func, lispList *list){
	if (list->list.size != func->args.size + 1){
		fprintf(stderr, "[Evaluatin] \033[31mError\033[0m Wrong amount of arguments\n");
		throughError(list->source);
		return &ERROR_OBJECT;
	}

	// Evaluating args
	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);
	for (size_t i = 0; i < func->args.size; i++){
		lispObject *buffer = eval(global, local, list->list.arr[i + 1]);
		if (buffer->type == ERROR_LISP){
			return &ERROR_OBJECT;
		}

		METHOD(obj_p_vec, evaluated, push, buffer);
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
		METHOD(str_obj_p_map, ((context*)func->ctx)->map, set, func->args.arr[i].value, evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);
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

	return out;
}

lispObject* eval(context *global, context *local, lispObject *obj){
	if (!obj->evalable){
		return lispObject_copy_construct(obj);
	}

	lispObject *out;
	if (obj->type == LIST_LISP){
		out = evalList(global, local, (lispList*)obj);
	}
	else if (obj->type == INT_LISP){
		out = lispObject_copy_construct(obj);
	}
	else if (obj->type == STR_LISP){
		out = lispObject_copy_construct(obj);
	}
	else if (obj->type == SYMB_LISP){
		out = evalSymb(global, local, (lispSymb*)obj);
	}
	else if (obj->type == ERROR_LISP){
		return &ERROR_OBJECT;
	}
	else{
		return lispObject_copy_construct(obj);
	}
	
	if (out->type == ERROR_LISP){
		return &ERROR_OBJECT;
	}

	out->source = obj->source;

	return out;
}
