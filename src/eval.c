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
		try_find = METHOD(context, *local, get, symbol->value);

		if (try_find != NULL){
			return lispObject_copy_construct(*try_find);
		}
	}

	try_find = METHOD(context, *global, get, symbol->value);

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
	
	// Binding arguments in function context
	for (size_t i = 0; i < func->args.size; i++){
		lispObject *buffer = eval(global, local, list->list.arr[i + 1]);
		if (buffer->type == ERROR_LISP){
			return &ERROR_OBJECT;
		}

		lispObject **old = METHOD(context, *((context*)func->ctx), get, func->args.arr[i].value);
		if (old != NULL){
			lispObject_destruct(*old);
		}

		METHOD(context, *((context*)func->ctx), set, func->args.arr[i].value, buffer);
	}

	massSetSource(func->body, func->source);

	return eval(global, (context*)func->ctx, func->body);
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

	// Binding arguments in function context
	for (size_t i = 0; i < func->args.size; i++){
		lispObject *buffer = eval(global, local, list->list.arr[i + 1]);
		if (buffer->type == ERROR_LISP){
			return &ERROR_OBJECT;
		}

		lispObject **old = METHOD(context, *((context*)func->ctx), get, func->args.arr[i].value);
		if (old != NULL){
			lispObject_destruct(*old);
		}

		METHOD(context, *((context*)func->ctx), set, func->args.arr[i].value, buffer);
	}

	return eval(global, (context*)func->ctx, func->body);
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
	
	if (out->type == ERROR_LISP){
		return &ERROR_OBJECT;
	}

	out->source = obj->source;

	return out;
}
