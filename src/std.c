#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "interpreterAPI.h"

lispObject* add(void *global, void *local, lispCFunction *func, lispList *args){
	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
		if (buffer->type == ERROR_LISP){
			terminate = true;
			break;
		}
		else if (buffer->type != INT_LISP){
			fprintf(stderr, "[Evaluating] \033[31mError\033[0m expected int\n");
			throughError(args->list.arr[i]->source);
			lispObject_destruct(buffer);
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
		return &ERROR_OBJECT;
	}

	lispObject *lookup = lookupToMemo(func, evaluated);
	if (lookup != NULL){
		printf("MEMO!\n");
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	lispInt *out = malloc(sizeof(lispInt));
	out->ref_counter = 1;
	out->type = INT_LISP;
	out->evalable = false;
	out->value = 0;

	for (size_t i = 0; i < evaluated.size; i++){
		out->value += ((lispInt*)evaluated.arr[i])->value;
	}

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

lispObject* sub(void *global, void *local, lispCFunction *func, lispList *args){
	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
		if (buffer->type == ERROR_LISP){
			terminate = true;
			break;
		}
		else if (buffer->type != INT_LISP){
			fprintf(stderr, "[Evaluating] \033[31mError\033[0m expected int\n");
			throughError(args->list.arr[i]->source);
			lispObject_destruct(buffer);
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
		return &ERROR_OBJECT;
	}

	lispObject *lookup = lookupToMemo(func, evaluated);
	if (lookup != NULL){
		printf("MEMO!\n");
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	lispInt *out = malloc(sizeof(lispInt));
	out->ref_counter = 1;
	out->type = INT_LISP;
	out->evalable = false;
	out->value = 0;

	if (evaluated.size == 0){
		addToMemo(func, evaluated, (lispObject*)out);
		DESTRUCT(obj_p_vec, evaluated);
		return (lispObject*)out;
	}

	out->value = ((lispInt*)evaluated.arr[0])->value;
	for (size_t i = 1; i < evaluated.size; i++){
		out->value -= ((lispInt*)evaluated.arr[i])->value;
	}

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

lispObject* setq(void *global, void *local, lispCFunction *func, lispList *args){

	obj_p_vec symbols = CONSTRUCT(obj_p_vec);
	obj_p_vec values = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i += 2){
		if (args->list.arr[i]->type != SYMB_LISP){
			fprintf(stderr, "[Evaluating] \033[31mError\033[0m expected symbol\n");
			throughError(args->list.arr[i]->source);
			terminate = true;
			break;
		}

		lispObject **curr_binding = NULL;
		if (local != NULL){
			curr_binding = METHOD(str_obj_p_map, ((context*)local)->map, get, ((lispSymb*)args->list.arr[i])->value);
		}
		if (curr_binding == NULL){
			curr_binding = METHOD(str_obj_p_map, ((context*)global)->map, get, ((lispSymb*)args->list.arr[i])->value);
		}

		if (curr_binding != NULL && ((*curr_binding)->type == CFUNC_LISP || (*curr_binding)->type == LFUNC_LISP || (*curr_binding)->type == ANON_FUNC_LISP)){
			fprintf(stderr, "[Evaluating] \033[31mError\033[0m Can't override functional object\n");
			throughError(args->list.arr[i]->source);
			return &ERROR_OBJECT;
		}
		
		// Objects in symbols isn't borrowed, because only string from them is needed
		METHOD(obj_p_vec, symbols, push, args->list.arr[i]);
	}

	if (terminate){
		// Objects in symbolds isn't destructed, because they're not borrowed
		DESTRUCT(obj_p_vec, symbols);
		DESTRUCT(obj_p_vec, values);
		return &ERROR_OBJECT;
	}

	// Amount of arguments is odd => there is symbol without value
	if ((args->list.size - 1) % 2 != 0){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m no value to bind\n");
		throughError(args->list.arr[args->list.size - 1]->source);
		DESTRUCT(obj_p_vec, symbols);
		DESTRUCT(obj_p_vec, values);
		return &ERROR_OBJECT;
	}

	for (size_t i = 2; i < args->list.size; i += 2){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
		if (buffer->type == ERROR_LISP){
			terminate = true;
			break;
		}
		METHOD(obj_p_vec, values, push, buffer);
	}

	if (terminate){
		for (size_t i = 0; i < values.size; i++){
			lispObject_destruct(values.arr[i]);
		}
		DESTRUCT(obj_p_vec, symbols);
		DESTRUCT(obj_p_vec, values);
		return &ERROR_OBJECT;
	}

	// Binding all symbols
	for (size_t i = 0;i < symbols.size; i++){
		context *ctx;	// Context to bind

		// Check if local context exists
		// And if there is defined var in this context
		if (local != NULL && METHOD(str_obj_p_map, ((context*)local)->map, get, ((lispSymb*)symbols.arr[i])->value) != NULL){
			ctx = (context*)local;
		}
		else{
			ctx = (context*)global;
		}

		bind(ctx, ((lispSymb*)symbols.arr[i])->value, values.arr[i]);
	}

	lispObject *out;
	if (values.size == 0){
		out = malloc(sizeof(lispSymb));
		((lispSymb*)out)->type = SYMB_LISP;
		((lispSymb*)out)->evalable = false;
		((lispSymb*)out)->ref_counter = 1;
		((lispSymb*)out)->value = malloc(4);
		strcpy(((lispSymb*)out)->value, "NIL");
	}
	else{
		out = lispObject_borrow(values.arr[values.size - 1]);
	}

	DESTRUCT(obj_p_vec, symbols);
	DESTRUCT(obj_p_vec, values);

	return out;
}

typedef struct body_info{
	bool dirty;
	bool error;
}body_info;
static body_info checkBody(void *global, void *local, lispObject *obj){

	if (obj->type == LIST_LISP && obj->evalable){
		body_info out = {false, false};

		if (((lispList*)obj)->list.size == 0){
			return (body_info){false, false};
		}

		lispObject *first = eval(global, local, ((lispList*)obj)->list.arr[0]);

		if (first->type != CFUNC_LISP && first->type != LFUNC_LISP && first->type != ANON_FUNC_LISP){
			fprintf(stderr, "[Evaluating] \033[31mError\033[0m Incorrect expression, first element isn't functional object\n");
			throughError(first->source);
			return (body_info){false, true};
		}

		if (first->type == CFUNC_LISP && ((lispCFunction*)first)->dirty){
			out.dirty = true;
		}
		else if (first->type == LFUNC_LISP && ((lispLFunction*)first)->dirty){
			out.dirty = true;
		}

		lispObject_destruct(first);

		for (size_t i = 1; i < ((lispList*)obj)->list.size; i++){
			body_info buffer = checkBody(global, local, ((lispList*)obj)->list.arr[i]);
			if (buffer.error){
				return (body_info){false, true};
			}
			if (buffer.dirty){
				out.dirty = true;
			}
		}

		return out;
	}
	else{
		return (body_info){false, false};
	}
}


lispObject* defun(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 4){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong amount of arguments, expected: 3, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	lispObject *symb = args->list.arr[1];
	if (symb->type != SYMB_LISP){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Expected symbol\n");
		throughError(symb->source);
		return &ERROR_OBJECT;
	}

	lispObject **curr_binding = METHOD(str_obj_p_map, ((context*)global)->map, get, ((lispSymb*)symb)->value);
	if (curr_binding != NULL && ((*curr_binding)->type == CFUNC_LISP || (*curr_binding)->type == LFUNC_LISP || (*curr_binding)->type == ANON_FUNC_LISP)){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Can't override functional object\n");
		throughError(symb->source);
		return &ERROR_OBJECT;
	}

	if (args->list.arr[2]->type != LIST_LISP){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m expected list with symbols\n");
		throughError(args->list.arr[2]->source);
		return &ERROR_OBJECT;
	}

	obj_p_vec args_list = CONSTRUCT(obj_p_vec);
	for (size_t i = 0; i < ((lispList*)args->list.arr[2])->list.size; i++){
		if (((lispList*)args->list.arr[2])->list.arr[i]->type != SYMB_LISP){
			fprintf(stderr, "[Evaluating] \033[31mError\033[0m expected symbol\n");
			throughError(((lispList*)args->list.arr[2])->list.arr[i]->source);
			DESTRUCT(obj_p_vec, args_list);
			return &ERROR_OBJECT;
		}
		METHOD(obj_p_vec, args_list, push, ((lispList*)args->list.arr[2])->list.arr[i]);
	}

	lispObject *body = args->list.arr[3];

	body_info info = checkBody(global, local, body);
	if (info.error){
		DESTRUCT(obj_p_vec, args_list);
		return &ERROR_OBJECT;
	}

	lispLFunction *out = malloc(sizeof(lispLFunction));
	out->type = LFUNC_LISP;
	out->ref_counter = 1;
	out->evalable = false;
	out->dirty = info.dirty;
	out->memoiz = malloc(sizeof(memo));
	*(memo*)out->memoiz = CONSTRUCT(memo);
	out->body = lispObject_borrow(body);


	if (local == NULL){
		out->ctx = derive_context(NULL);
	}
	else{
		out->ctx = borrow_context(local);
	}

	out->args = CONSTRUCT(symb_vec);
	for (size_t i = 0; i < args_list.size; i++){
		lispSymb buffer;
		buffer.type = SYMB_LISP;
		buffer.evalable = false;
		buffer.ref_counter = 0;
		buffer.value = malloc(strlen(((lispSymb*)args_list.arr[i])->value) + 1);
		strcpy(buffer.value, ((lispSymb*)args_list.arr[i])->value);

		METHOD(symb_vec, out->args, push, buffer);
	}
	DESTRUCT(obj_p_vec, args_list);

	bind((context*)global, ((lispSymb*)symb)->value, (lispObject*)out);

	return lispObject_borrow((lispObject*)out);
}

lispObject* if_clause(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 4){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong amount of arguments, expected: 3, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	lispObject *clause = eval(global, local, args->list.arr[1]);
	if (clause->type == ERROR_LISP){
		return &ERROR_OBJECT;
	}

	if (clause->type != SYMB_LISP || (strcmp(((lispSymb*)clause)->value, "NIL") != 0 && strcmp(((lispSymb*)clause)->value, "T") != 0)){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Expected symbols NUL or T\n");
		throughError(clause->source);
		lispObject_destruct(clause);
		return &ERROR_OBJECT;
	}

	if (strcmp(((lispSymb*)clause)->value, "T") == 0){
		return eval(global, local, args->list.arr[2]);
	}
	else{
		return eval(global, local, args->list.arr[3]);
	}
}

lispObject* eq(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size <= 1){
		lispSymb *out = malloc(sizeof(lispSymb));
		out->type = SYMB_LISP;
		out->evalable = false;
		out->ref_counter = 1;
		out->value = malloc(2);
		strcpy(out->value, "T");
		return (lispObject*)out;
	}
}


void loadAllOps(context *ctx){
	addOperator(ctx, "+", add, false);
	addOperator(ctx, "-", sub, false);
	addOperator(ctx, "SETQ", setq, true);
	addOperator(ctx, "DEFUN", defun, true);
	addOperator(ctx, "IF", if_clause, false);
}
