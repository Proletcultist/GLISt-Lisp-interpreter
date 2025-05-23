#include <stdio.h>
#include "parser.h"
#include "lexer.h"
#include "generalFunctions.h"
#include "errorPrinter.h"
#include "lispContext.h"
#include "macroProcessor.h"

node macrotest_func(void *ctx, node args){
	node out;
	out.type = NONTERMINAL_NODE;
	out.value.nonterminal_val.type = EXPR_NT;
	out.childs = CONSTRUCT(node_vec);

	node out_value;
	out_value.type = NONTERMINAL_NODE;
	out_value.value.nonterminal_val.type = VALUE_NT;
	out_value.childs = CONSTRUCT(node_vec);

	node value_token;
	value_token.type = TOKEN_NODE;
	value_token.childs = CONSTRUCT(node_vec);
	value_token.value.token_val.type = INT_TOKEN;
	value_token.value.token_val.value.int_val = 128;
	value_token.value.token_val.start = 0;
	value_token.value.token_val.end = 0;
	METHOD(node_vec, out_value.childs, push, value_token);

	METHOD(node_vec, out.childs, push, out_value);

	return out;
}
static lispCMacro macrotest = {CMACRO_LISP, false, macrotest_func};

node errormacro_func(void *ctx, node args){
	node out;
	out.type = NONTERMINAL_NODE;
	out.value.nonterminal_val.type = ERROR_NT;
	out.childs = CONSTRUCT(node_vec);

	return out;
}
static lispCMacro errormacro = {CMACRO_LISP, false, errormacro_func};

int main(){


	context global = CONSTRUCT(context);

	METHOD(context, global, set, "MACROTEST", (lispObject*)&macrotest);
	METHOD(context, global, set, "ERRORMACRO", (lispObject*)&errormacro);

	char test[] = "(func (errormacro 24 ))";

	FILE *strstream = fmemopen(test, sizeof(test) - 1, "r");

	lexer l = CONSTRUCT(lexer, strstream, true);
	
	node out = parseExprToAST(l);
	if (iserror(out)){
		fputc('\n', stderr);
		printParseErrorAST(stderr, &out, strstream);
		fputc('\n', stderr);
	}

	macroProcess(&global, &out);
	if (iserror(out)){
		fputc('\n', stderr);
		printParseErrorAST(stderr, &out, strstream);
		fputc('\n', stderr);
	}

	node cursor = out;

	while (1){
		printf("%zu\n", cursor.childs.size);
		for (size_t i = 0; i < cursor.childs.size; i++){
			if (cursor.childs.arr[i].type == TOKEN_NODE){
				switch (cursor.childs.arr[i].value.token_val.type){
					case INT_TOKEN:
						printf("<int>");
						break;
					case STR_TOKEN:
						printf("<str>");
						break;
					case SYMBOL_TOKEN:
						printf("<symbol>");
						break;
					case SEP_TOKEN:
						printf("<sep>");
						break;
					case QUOTE_TOKEN:
						printf("<quote>");
						break;
					case OPEN_PARENTHESIS_TOKEN:
						printf("<op>");
						break;
					case CLOSE_PARENTHESIS_TOKEN:
						printf("<cp>");
						break;
					case END_OF_INPUT:
						printf("<EOF>");
						break;
					case ERROR_TOKEN:
						printf("<ERROR>");
				}
			}
			else{
				switch (cursor.childs.arr[i].value.nonterminal_val.type){
					case EXPR_NT:
						printf("[expr]");
						break;
					case LIST_NT:
						printf("[list]");
						break;
					case LIST_CONTENT_NT:
						printf("[list_c]");
						break;
					case VALUE_NT:
						printf("[value]");
						break;
					case ERROR_NT:
						printf("[ERROR]");
						break;
					case UNFINISHED_NT:
						printf("[UNF]");
				}
			}
		}
		printf("\n");

	
		size_t num;
		scanf("%zu", &num);

		cursor = cursor.childs.arr[num];
	}

	return 0;
}
