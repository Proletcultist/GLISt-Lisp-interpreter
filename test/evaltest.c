#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "parser.h"
#include "lexer.h"
#include "generalFunctions.h"
#include "errorPrinter.h"
#include "lispObject.h"
#include "eval.h"
#include "objectBuilder.h"

lispObject* add(void *global, void *local, lispList *args){
	lispInt *out = malloc(sizeof(lispInt));
	out->type = INT_LISP;
	out->evalable = false;
	out->value = 0;

	for (size_t i = 1; i < args->list.size; i++){
		lispObject *buffer = eval((context*)global, (context*)local, args->list.arr[i]);
		if (buffer->type == ERROR_LISP){
			return &ERROR_OBJECT;
		}
		else if (buffer->type != INT_LISP){
			lispObject_destruct(buffer);
			fprintf(stderr, "[Evaluating] \033[31mError\033[0m expected int\n");
			throughError(args->list.arr[i]->source);
			lispObject_destruct((lispObject*)out);
			return &ERROR_OBJECT;
		}
		out->value += ((lispInt*)buffer)->value;
		lispObject_destruct(buffer);
	}

	return (lispObject*)out;
}

int main(){
	context *global = malloc(sizeof(context));
	*global = CONSTRUCT(context);

// Add -------------------
	lispCFunction *add_func = malloc(sizeof(lispCFunction));
	add_func->type = CFUNC_LISP;
	add_func->evalable = false;
	add_func->body = add;

	char *plus = malloc(2);
	strcpy(plus, "+");
	METHOD(context, *global, set, plus, (lispObject*)add_func);
// Func test -------------
	lispLFunction *func = malloc(sizeof(lispLFunction));
	func->type = LFUNC_LISP;
	func->evalable = false;
	func->args = CONSTRUCT(symb_vec);

	char funcBody[] = "(+ a b)";
	FILE *strstream = fmemopen(funcBody, sizeof(funcBody) - 1, "r");
	lexer l = CONSTRUCT(lexer, strstream, true);
	node *out = parseExprToAST(l);
	func->body = visitExpr(out);
	destruct_node_rec(out);
	fclose(strstream);

	char *first_arg = malloc(2);
	char *second_arg = malloc(2);
	strcpy(first_arg, "A");
	strcpy(second_arg, "B");
	
	lispSymb a1;
	a1.type = SYMB_LISP;
	a1.evalable = false;
	a1.value = first_arg;
	lispSymb a2;
	a2.type = SYMB_LISP;
	a2.evalable = false;
	a2.value = second_arg;

	METHOD(symb_vec, func->args, push, a1);
	METHOD(symb_vec, func->args, push, a2);

	func->ctx = derive_context(NULL);

	char *func_name = malloc(5);
	strcpy(func_name, "FUNC");

	METHOD(context, *global, set, func_name, (lispObject*)func);
// Test ------------------

	char test[] = "(+ (func 2 3) (func 4 5))";

	strstream = fmemopen(test, sizeof(test) - 1, "r");
	l = CONSTRUCT(lexer, strstream, true);
	out = parseExprToAST(l);
	if (iserror(*out)){
		fputc('\n', stderr);
		printParseErrorAST(stderr, out, strstream);
		fputc('\n', stderr);
	}

	lispObject *generated = visitExpr(out);

	lispObject *evaluated = eval(global, NULL, generated);

	if (evaluated->type == ERROR_LISP){
		fputc('\n', stderr);
		printParseErrorAST(stderr, out, strstream);
		fputc('\n', stderr);
	}
	else{
		printObject(stdout, evaluated);
		printf("\n");
	}

	destruct_node_rec(out);
	lispObject_destruct(generated);
	lispObject_destruct(evaluated);
	destructAllObjects(global);
	DESTRUCT(context, *global);
	free(global);
	fclose(strstream);

	return 0;
}
