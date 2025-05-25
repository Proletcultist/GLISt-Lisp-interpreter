#include <stdlib.h>
#include <string.h>
#include "objectBuilder.h"
#include "parser.h"
#include "lispObject.h"
#include "generalFunctions.h"

lispObject* visitExpr(node *src){
	// Check for quote
	lispObject *out;
	if (src->childs.arr[0]->type == TOKEN_NODE){
		if (src->childs.arr[1]->value.nonterminal_val.type == VALUE_NT){
			out = visitValue(src->childs.arr[1]);
		}
		else{
			out = visitList(src->childs.arr[1]);
		}
		out->evalable = false;
	}
	else{
		if (src->childs.arr[0]->value.nonterminal_val.type == VALUE_NT){
			out = visitValue(src->childs.arr[0]);
		}
		else{
			out = visitList(src->childs.arr[0]);
		}
		out->evalable = true;
	}
	out->source = src;

	return out;
}

lispObject* visitList(node *src){
	lispList *out = malloc(sizeof(lispList));
	out->type = LIST_LISP;
	// Evalable and source will be set by expr visitor, called this

	out->list = CONSTRUCT(obj_p_vec);

	if (src->childs.arr[1]->type == NONTERMINAL_NODE){
		visitListContent(src->childs.arr[1], out);
	}
	else{
		visitListContent(src->childs.arr[2], out);
	}

	return (lispObject*)out;
}

void visitListContent(node *src, lispList *list){
	if (src->childs.size == 0){
		return;
	}

	METHOD(obj_p_vec, list->list, push, visitExpr(src->childs.arr[0]));

	if (src->childs.size != 1){
		visitListContent(src->childs.arr[2], list);
	}
}

lispObject* visitValue(node *src){
	token t = src->childs.arr[0]->value.token_val;

	if (t.type == INT_TOKEN){
		lispInt *out = malloc(sizeof(lispInt));
		out->type = INT_LISP;
		// Evalable and source will be set by expr visitor, called this
		out->value = t.value.int_val;

		return (lispObject*)out;
	}
	else if (t.type == STR_TOKEN){
		lispStr *out = malloc(sizeof(lispStr));
		out->type = STR_LISP;
		// Evalable and source will be set by expr visitor, called this
		out->value = malloc(strlen(t.value.str_val) + 1);
		strcpy(out->value, t.value.str_val);

		return (lispObject*)out;
	}
	else if (t.type == SYMBOL_TOKEN){
		lispSymb *out = malloc(sizeof(lispSymb));
		out->type = SYMB_LISP;
		// Evalable and source will be set by expr visitor, called this
		out->value = malloc(strlen(t.value.str_val) + 1);
		strcpy(out->value, t.value.str_val);

		return (lispObject*)out;
	}
}
