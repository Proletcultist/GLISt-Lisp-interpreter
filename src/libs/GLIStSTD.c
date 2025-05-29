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
		printf("MEMO ADD!\n");
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
		printf("MEMO SUB!\n");
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
static body_info checkBody(void *global, void *local, lispObject *obj, char *origin_name, obj_p_vec args){

	if (obj->type == LIST_LISP && obj->evalable){
		body_info out = {false, false};

		if (((lispList*)obj)->list.size == 0){
			return (body_info){false, false};
		}

		if (((lispList*)obj)->list.arr[0]->type == SYMB_LISP){
			if (strcmp(((lispSymb*)((lispList*)obj)->list.arr[0])->value, origin_name) == 0){
				return (body_info){false, false};
			}
			for (size_t i = 0; i < args.size; i++){
				if (strcmp(((lispSymb*)((lispList*)obj)->list.arr[0])->value, ((lispSymb*)args.arr[i])->value) == 0){
					return (body_info){false, false};
				}
			}
		}

		lispObject *first = eval(global, local, ((lispList*)obj)->list.arr[0]);
		if (first->type == ERROR_LISP){
			return (body_info){false, true};
		}

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
			body_info buffer = checkBody(global, local, ((lispList*)obj)->list.arr[i], origin_name, args);
			if (buffer.error){
				return (body_info){false, true};
			}
			if (buffer.dirty){
				out.dirty = true;
			}
		}

		return out;
	}
	else if (obj->type == SYMB_LISP){
		for (size_t i = 0; i < args.size; i++){
			if (strcmp(((lispSymb*)obj)->value, ((lispSymb*)args.arr[i])->value) == 0){
				return (body_info){false, false};
			}
		}
		return (body_info){true, false};
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

	body_info info = checkBody(global, local, body, ((lispSymb*)symb)->value, args_list);
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
		lispObject_destruct(clause);
		return eval(global, local, args->list.arr[2]);
	}
	else{
		lispObject_destruct(clause);
		return eval(global, local, args->list.arr[3]);
	}
}

lispObject* eq(void *global, void *local, lispCFunction *func, lispList *args){
	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
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
		return &ERROR_OBJECT;
	}

	lispObject *lookup = lookupToMemo(func, evaluated);
	if (lookup != NULL){
		printf("MEMO =!\n");
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	if (evaluated.size <= 1){
		lispSymb *out = malloc(sizeof(lispSymb));
		out->type = SYMB_LISP;
		out->evalable = false;
		out->ref_counter = 1;
		out->value = malloc(2);
		strcpy(out->value, "T");
		
		addToMemo(func, evaluated, (lispObject*)out);

		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return (lispObject*)out;
	}

	lispSymb *out = malloc(sizeof(lispSymb));
	out->type = SYMB_LISP;
	out->evalable = false;
	out->ref_counter = 1;

	for (size_t i = 1; i < evaluated.size; i++){
		if (lispObject_p_compare(evaluated.arr[i - 1], evaluated.arr[i]) != 0){
			out->value = malloc(4);
			strcpy(out->value, "NIL");
			terminate = true;
			break;
		}
	}

	if (!terminate){
		out->value = malloc(2);
		strcpy(out->value, "T");
	}

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);
	return (lispObject*)out;
}

lispObject* progn(void *global, void *local, lispCFunction *func, lispList *args){
	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
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
		return &ERROR_OBJECT;
	}

	lispObject *lookup = lookupToMemo(func, evaluated);
	if (lookup != NULL){
		printf("MEMO progn!\n");
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	if (evaluated.size == 0){
		lispSymb *out = malloc(sizeof(lispSymb));
		out->type = SYMB_LISP;
		out->evalable = false;
		out->ref_counter = 1;
		out->value = malloc(4);
		strcpy(out->value, "NIL");
		
		addToMemo(func, evaluated, (lispObject*)out);

		DESTRUCT(obj_p_vec, evaluated);
		return (lispObject*)out;
	}

	lispObject *out = lispObject_borrow(evaluated.arr[evaluated.size - 1]);

		addToMemo(func, evaluated, out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return out;
}

lispObject* let(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 3){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong amount of arguments, expected: 2, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	lispObject *bindings = args->list.arr[1];
	if (bindings->type != LIST_LISP){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Expected list with bindings\n");
		throughError(bindings->source);
		return &ERROR_OBJECT;
	}

	obj_p_vec symbols = CONSTRUCT(obj_p_vec);
	obj_p_vec values = CONSTRUCT(obj_p_vec);

	bool terminate = false;

	for (size_t i = 0; i < ((lispList*)bindings)->list.size; i++){
		if (((lispList*)bindings)->list.arr[i]->type == SYMB_LISP){
			METHOD(obj_p_vec, symbols, push, ((lispList*)bindings)->list.arr[i]);

			lispSymb *out = malloc(sizeof(lispSymb));
			out->type = SYMB_LISP;
			out->evalable = false;
			out->ref_counter = 1;
			out->value = malloc(4);
			strcpy(out->value, "NIL");

			METHOD(obj_p_vec, values, push, (lispObject*)out);
		}
		else if (((lispList*)bindings)->list.arr[i]->type == LIST_LISP){
			lispList *pair = ((lispList*)((lispList*)bindings)->list.arr[i]);
			if (pair->list.size != 2){
				fprintf(stderr, "[Evaluating] \033[31mError\033[0m Expected symbol or pair of symbol and value\n");
				throughError(((lispList*)bindings)->list.arr[i]->source);
				terminate = true;
				break;
			}

			if (pair->list.arr[0]->type != SYMB_LISP){
				fprintf(stderr, "[Evaluating] \033[31mError\033[0m Expected symbol\n");
				throughError(pair->list.arr[0]->source);
				terminate = true;
				break;
			}

			METHOD(obj_p_vec, symbols, push, pair->list.arr[0]);

			lispObject *value = eval((context*)global, (context*)local, pair->list.arr[1]);
			if (value->type == ERROR_LISP){
				terminate = true;
				break;
			}

			METHOD(obj_p_vec, values, push, value);
		}
		else{
			fprintf(stderr, "[Evaluating] \033[31mError\033[0m Expected symbol or pair of symbol and value\n");
			throughError(((lispList*)bindings)->list.arr[i]->source);
			terminate = true;
			break;
		}
	}

	if (terminate){
		for (size_t i = 0; i < values.size; i++){
			lispObject_destruct(values.arr[i]);
		}
		DESTRUCT(obj_p_vec, symbols);
		DESTRUCT(obj_p_vec, values);

		return &ERROR_OBJECT;
	}

	context *new_local = derive_context((context*)local);

	for (size_t i = 0; i < symbols.size; i++){
		bind(new_local, ((lispSymb*)symbols.arr[i])->value, values.arr[i]);
	}

	lispObject *out = eval((context*)global, new_local, args->list.arr[2]);

	putContext(new_local);
	DESTRUCT(obj_p_vec, symbols);
	DESTRUCT(obj_p_vec, values);

	return out;
}

lispObject* print(void *global, void *local, lispCFunction *func, lispList *args){
	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
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
		return &ERROR_OBJECT;
	}

	for (size_t i = 0; i < evaluated.size; i++){
		printObject(stdout, evaluated.arr[i]);
		fprintf(stdout, "\n");
	}

	lispObject *out;
	if (evaluated.size == 0){
			out = malloc(sizeof(lispSymb));
			((lispSymb*)out)->type = SYMB_LISP;
			((lispSymb*)out)->evalable = false;
			((lispSymb*)out)->ref_counter = 1;
			((lispSymb*)out)->value = malloc(4);
			strcpy(((lispSymb*)out)->value, "NIL");
	}
	else{
		out = lispObject_borrow(evaluated.arr[evaluated.size - 1]);
	}

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return out;
}

lispObject* mult(void *global, void *local, lispCFunction *func, lispList *args){
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
		printf("MEMO MULT!\n");
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
	out->value = 1;

	for (size_t i = 0; i < evaluated.size; i++){
		out->value *= ((lispInt*)evaluated.arr[i])->value;
	}

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

lispObject* divide(void *global, void *local, lispCFunction *func, lispList *args){
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
		printf("MEMO DIV!\n");
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
	out->value = 1;

	if (evaluated.size == 0){
		addToMemo(func, evaluated, (lispObject*)out);
		DESTRUCT(obj_p_vec, evaluated);
		return (lispObject*)out;
	}

	out->value = ((lispInt*)evaluated.arr[0])->value;
	for (size_t i = 1; i < evaluated.size; i++){
		out->value /= ((lispInt*)evaluated.arr[i])->value;
	}

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

lispObject* listcr(void *global, void *local, lispCFunction *func, lispList *args){
	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
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
		return &ERROR_OBJECT;
	}

	lispObject *lookup = lookupToMemo(func, evaluated);
	if (lookup != NULL){
		printf("MEMO LISTCR!\n");
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	lispList *out = malloc(sizeof(lispList));
	out->type = LIST_LISP;
	out->evalable = true;
	out->ref_counter = 1;
	out->list = evaluated;

	addToMemo(func, evaluated, (lispObject*)out);

	return (lispObject*)out;

}

lispObject* lambda(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 3){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong amount of arguments, expected: 2, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	if (args->list.arr[1]->type != LIST_LISP){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m expected list with symbols\n");
		throughError(args->list.arr[1]->source);
		return &ERROR_OBJECT;
	}

	obj_p_vec args_list = CONSTRUCT(obj_p_vec);
	for (size_t i = 0; i < ((lispList*)args->list.arr[1])->list.size; i++){
		if (((lispList*)args->list.arr[1])->list.arr[i]->type != SYMB_LISP){
			fprintf(stderr, "[Evaluating] \033[31mError\033[0m expected symbol\n");
			throughError(((lispList*)args->list.arr[1])->list.arr[i]->source);
			DESTRUCT(obj_p_vec, args_list);
			return &ERROR_OBJECT;
		}
		METHOD(obj_p_vec, args_list, push, ((lispList*)args->list.arr[1])->list.arr[i]);
	}

	lispObject *body = args->list.arr[2];

	lispAnonFunction *out = malloc(sizeof(lispAnonFunction));
	out->type = ANON_FUNC_LISP;
	out->evalable = false;
	out->ref_counter = 1;
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

	return (lispObject*)out;
}

lispObject* le(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 3){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong amount of arguments, expected: 2, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
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
		return &ERROR_OBJECT;
	}

	lispObject *lookup = lookupToMemo(func, evaluated);
	if (lookup != NULL){
		printf("MEMO =!\n");
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	if (evaluated.arr[0]->type != evaluated.arr[1]->type){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m There is no order on objects with different type\n");
		throughError(evaluated.arr[0]->source);
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return &ERROR_OBJECT;
	}

	if (evaluated.arr[0]->type != STR_LISP && evaluated.arr[0]->type != INT_LISP){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Expected ints or strings\n");
		throughError(evaluated.arr[0]->source);
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return &ERROR_OBJECT;
	}

	lispSymb *out = malloc(sizeof(lispSymb));
	out->type = SYMB_LISP;
	out->evalable = false;
	out->ref_counter = 1;

	if (lispObject_p_compare(evaluated.arr[0], evaluated.arr[1]) == -1){
		out->value = malloc(2);
		strcpy(out->value, "T");
	}
	else{
		out->value = malloc(4);
		strcpy(out->value, "NIL");
	}

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

lispObject* hi(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 3){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong amount of arguments, expected: 2, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
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
		return &ERROR_OBJECT;
	}

	lispObject *lookup = lookupToMemo(func, evaluated);
	if (lookup != NULL){
		printf("MEMO =!\n");
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	if (evaluated.arr[0]->type != evaluated.arr[1]->type){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m There is no order on objects with different type\n");
		throughError(evaluated.arr[0]->source);
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return &ERROR_OBJECT;
	}

	if (evaluated.arr[0]->type != STR_LISP && evaluated.arr[0]->type != INT_LISP){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Expected ints or strings\n");
		throughError(evaluated.arr[0]->source);
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return &ERROR_OBJECT;
	}

	lispSymb *out = malloc(sizeof(lispSymb));
	out->type = SYMB_LISP;
	out->evalable = false;
	out->ref_counter = 1;

	if (lispObject_p_compare(evaluated.arr[0], evaluated.arr[1]) == 1){
		out->value = malloc(2);
		strcpy(out->value, "T");
	}
	else{
		out->value = malloc(4);
		strcpy(out->value, "NIL");
	}

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

lispObject* leq(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 3){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong amount of arguments, expected: 2, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
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
		return &ERROR_OBJECT;
	}

	lispObject *lookup = lookupToMemo(func, evaluated);
	if (lookup != NULL){
		printf("MEMO =!\n");
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	if (evaluated.arr[0]->type != evaluated.arr[1]->type){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m There is no order on objects with different type\n");
		throughError(evaluated.arr[0]->source);
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return &ERROR_OBJECT;
	}

	if (evaluated.arr[0]->type != STR_LISP && evaluated.arr[0]->type != INT_LISP){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Expected ints or strings\n");
		throughError(evaluated.arr[0]->source);
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return &ERROR_OBJECT;
	}

	lispSymb *out = malloc(sizeof(lispSymb));
	out->type = SYMB_LISP;
	out->evalable = false;
	out->ref_counter = 1;

	if (lispObject_p_compare(evaluated.arr[0], evaluated.arr[1]) == 0 || lispObject_p_compare(evaluated.arr[0], evaluated.arr[1]) == -1){
		out->value = malloc(2);
		strcpy(out->value, "T");
	}
	else{
		out->value = malloc(4);
		strcpy(out->value, "NIL");
	}

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

lispObject* hiq(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 3){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong amount of arguments, expected: 2, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
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
		return &ERROR_OBJECT;
	}

	lispObject *lookup = lookupToMemo(func, evaluated);
	if (lookup != NULL){
		printf("MEMO =!\n");
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	if (evaluated.arr[0]->type != evaluated.arr[1]->type){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m There is no order on objects with different type\n");
		throughError(evaluated.arr[0]->source);
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return &ERROR_OBJECT;
	}

	if (evaluated.arr[0]->type != STR_LISP && evaluated.arr[0]->type != INT_LISP){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Expected ints or strings\n");
		throughError(evaluated.arr[0]->source);
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return &ERROR_OBJECT;
	}

	lispSymb *out = malloc(sizeof(lispSymb));
	out->type = SYMB_LISP;
	out->evalable = false;
	out->ref_counter = 1;

	if (lispObject_p_compare(evaluated.arr[0], evaluated.arr[1]) == 0 || lispObject_p_compare(evaluated.arr[0], evaluated.arr[1]) == 1){
		out->value = malloc(2);
		strcpy(out->value, "T");
	}
	else{
		out->value = malloc(4);
		strcpy(out->value, "NIL");
	}

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

lispObject* car(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 2){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong amount of arguments, expected: 1, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	lispObject *arg = eval((context*)global, (context*)local, args->list.arr[1]);
	if (arg->type == ERROR_LISP){
		return &ERROR_OBJECT;
	}

	if (arg->type != LIST_LISP){
		fprintf(stderr, "[Evaluating] \033[31Error\033[0m Expected list\n");
		throughError(arg->source);
		lispObject_destruct(arg);
		return &ERROR_OBJECT;
	}

	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);
	METHOD(obj_p_vec, evaluated, push, arg);

	lispObject *lookup = lookupToMemo(func, evaluated);
	if (lookup != NULL){
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	if (((lispList*)arg)->list.size == 0){
		lispSymb *out = malloc(sizeof(lispSymb));
		out->type = SYMB_LISP;
		out->evalable = false;
		out->ref_counter = 1;
		out->value = malloc(4);
		strcpy(out->value, "NIL");

		addToMemo(func, evaluated, (lispObject*)out);

		lispObject_destruct(arg);
		DESTRUCT(obj_p_vec, evaluated);
		return (lispObject*)out;
	}

	lispObject *out = lispObject_borrow(((lispList*)arg)->list.arr[0]);

	addToMemo(func, evaluated, out);

	lispObject_destruct(arg);
	DESTRUCT(obj_p_vec, evaluated);

	return out;
}

lispObject* cdr(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 2){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong amount of arguments, expected: 1, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	lispObject *arg = eval((context*)global, (context*)local, args->list.arr[1]);
	if (arg->type == ERROR_LISP){
		return &ERROR_OBJECT;
	}

	if (arg->type != LIST_LISP){
		fprintf(stderr, "[Evaluating] \033[31Error\033[0m Expected list\n");
		throughError(arg->source);
		lispObject_destruct(arg);
		return &ERROR_OBJECT;
	}

	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);
	METHOD(obj_p_vec, evaluated, push, arg);

	lispObject *lookup = lookupToMemo(func, evaluated);
	if (lookup != NULL){
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	if (((lispList*)arg)->list.size <= 1){
		lispSymb *out = malloc(sizeof(lispSymb));
		out->type = SYMB_LISP;
		out->evalable = false;
		out->ref_counter = 1;
		out->value = malloc(4);
		strcpy(out->value, "NIL");

		addToMemo(func, evaluated, (lispObject*)out);

		lispObject_destruct(arg);
		DESTRUCT(obj_p_vec, evaluated);
		return (lispObject*)out;
	}

	lispList *out = malloc(sizeof(lispList));
	out->type = LIST_LISP;
	out->evalable = false;
	out->ref_counter = 1;
	out->list = CONSTRUCT(obj_p_vec);

	for (size_t i = 1; i < ((lispList*)arg)->list.size; i++){
		METHOD(obj_p_vec, out->list, push, lispObject_borrow(((lispList*)arg)->list.arr[i]));
	}

	addToMemo(func, evaluated, (lispObject*)out);

	lispObject_destruct(arg);
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

lispObject* cons(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 3){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong amount of arguments, expected: 2, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
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
		return &ERROR_OBJECT;
	}

	if (evaluated.arr[1]->type != LIST_LISP){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Expected list\n");
		throughError(evaluated.arr[1]->source);
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return &ERROR_OBJECT;
	}

	lispObject *lookup = lookupToMemo(func, evaluated);
	if (lookup != NULL){
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	lispList *out = malloc(sizeof(lispList));
	out->type = LIST_LISP;
	out->evalable = false;
	out->ref_counter = 1;
	out->list = CONSTRUCT(obj_p_vec);

	METHOD(obj_p_vec, out->list, push, lispObject_borrow(evaluated.arr[0]));
	for (size_t i = 0; i < ((lispList*)evaluated.arr[1])->list.size; i++){
		METHOD(obj_p_vec, out->list, push, lispObject_borrow(((lispList*)evaluated.arr[1])->list.arr[i]));
	}

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

lispObject* append(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 3){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong amount of arguments, expected: 2, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
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
		return &ERROR_OBJECT;
	}

	if (evaluated.arr[1]->type != LIST_LISP){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Expected list\n");
		throughError(evaluated.arr[1]->source);
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return &ERROR_OBJECT;
	}

	lispObject *lookup = lookupToMemo(func, evaluated);
	if (lookup != NULL){
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	lispList *out = malloc(sizeof(lispList));
	out->type = LIST_LISP;
	out->evalable = false;
	out->ref_counter = 1;
	out->list = CONSTRUCT(obj_p_vec);

	for (size_t i = 0; i < ((lispList*)evaluated.arr[1])->list.size; i++){
		METHOD(obj_p_vec, out->list, push, lispObject_borrow(((lispList*)evaluated.arr[1])->list.arr[i]));
	}
	METHOD(obj_p_vec, out->list, push, lispObject_borrow(evaluated.arr[0]));

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

lispObject* and(void *global, void *local, lispCFunction *func, lispList *args){
	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
		if (buffer->type == ERROR_LISP){
			terminate = true;
			break;
		}
		else if (buffer->type != SYMB_LISP || (strcmp(((lispSymb*)buffer)->value, "NIL") != 0 && strcmp(((lispSymb*)buffer)->value, "T") != 0)){
			fprintf(stderr, "[Evaluating] \033[31mError\033[0m expected T or NIL symbols\n");
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
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	lispSymb *out = malloc(sizeof(lispSymb));
	out->type = SYMB_LISP;
	out->evalable = false;
	out->ref_counter = 1;

	for (size_t i = 0; i < evaluated.size; i++){
		if (strcmp(((lispSymb*)evaluated.arr[i])->value, "NIL") == 0){
			out->value = malloc(4);
			strcpy(out->value, "NIL");
			terminate = true;
			break;
		}
	}

	if (!terminate){
		out->value = malloc(2);
		strcpy(out->value, "T");
	}

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

lispObject* or(void *global, void *local, lispCFunction *func, lispList *args){
	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
		if (buffer->type == ERROR_LISP){
			terminate = true;
			break;
		}
		else if (buffer->type != SYMB_LISP || (strcmp(((lispSymb*)buffer)->value, "NIL") != 0 && strcmp(((lispSymb*)buffer)->value, "T") != 0)){
			fprintf(stderr, "[Evaluating] \033[31mError\033[0m expected T or NIL symbols\n");
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
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	lispSymb *out = malloc(sizeof(lispSymb));
	out->type = SYMB_LISP;
	out->evalable = false;
	out->ref_counter = 1;

	for (size_t i = 0; i < evaluated.size; i++){
		if (strcmp(((lispSymb*)evaluated.arr[i])->value, "T") == 0){
			out->value = malloc(2);
			strcpy(out->value, "T");
			terminate = true;
			break;
		}
	}

	if (!terminate){
		out->value = malloc(4);
		strcpy(out->value, "NIL");
	}

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

lispObject* not(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 2){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong amount of arguments, expected: 1, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	obj_p_vec evaluated = CONSTRUCT(obj_p_vec);

	bool terminate = false;
	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
		if (buffer->type == ERROR_LISP){
			terminate = true;
			break;
		}
		else if (buffer->type != SYMB_LISP || (strcmp(((lispSymb*)buffer)->value, "NIL") != 0 && strcmp(((lispSymb*)buffer)->value, "T") != 0)){
			fprintf(stderr, "[Evaluating] \033[31mError\033[0m expected T or NIL symbols\n");
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
		for (size_t i = 0; i < evaluated.size; i++){
			lispObject_destruct(evaluated.arr[i]);
		}
		DESTRUCT(obj_p_vec, evaluated);
		return lispObject_borrow(lookup);
	}

	lispSymb *out = malloc(sizeof(lispSymb));
	out->type = SYMB_LISP;
	out->evalable = false;
	out->ref_counter = 1;

	if (strcmp(((lispSymb*)evaluated.arr[0])->value, "T") == 0){
		out->value = malloc(4);
		strcpy(out->value, "NIL");
	}
	else{
		out->value = malloc(2);
		strcpy(out->value, "T");
	}

	addToMemo(func, evaluated, (lispObject*)out);

	for (size_t i = 0; i < evaluated.size; i++){
		lispObject_destruct(evaluated.arr[i]);
	}
	DESTRUCT(obj_p_vec, evaluated);

	return (lispObject*)out;
}

void loadAllOps(context *ctx){
	addOperator(ctx, "+", add, false);
	addOperator(ctx, "-", sub, false);
	addOperator(ctx, "SETQ", setq, true);
	addOperator(ctx, "DEFUN", defun, true);
	addOperator(ctx, "IF", if_clause, false);
	addOperator(ctx, "=", eq, false);
	addOperator(ctx, "PROGN", progn, false);
	addOperator(ctx, "LET", let, true);
	addOperator(ctx, "PRINT", print, true);
	addOperator(ctx, "*", mult, false);
	addOperator(ctx, "/", divide, false);
	addOperator(ctx, "LIST", listcr, false);
	addOperator(ctx, "LAMBDA", lambda, false);
	addOperator(ctx, "<", le, false);
	addOperator(ctx, ">", hi, false);
	addOperator(ctx, "<=", leq, false);
	addOperator(ctx, ">=", hiq, false);
	addOperator(ctx, "CAR", car, false);
	addOperator(ctx, "CDR", cdr, false);
	addOperator(ctx, "CONS", cons, false);
	addOperator(ctx, "APPEND", append, false);
	addOperator(ctx, "AND", and, false);
	addOperator(ctx, "OR", or, false);
	addOperator(ctx, "NOT", not, false);
}
