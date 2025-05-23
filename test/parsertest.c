#include <stdio.h>
#include <inttypes.h>
#include "parser.h"
#include "lexer.h"
#include "generalFunctions.h"
#include "errorPrinter.h"

int main(){
	char test[] = "(''fun 2 \"sasas (\n- 9 8\n))";

	FILE *strstream = fmemopen(test, sizeof(test) - 1, "r");

	lexer l = CONSTRUCT(lexer, strstream, true);
	
	node out = parseExprToAST(l);
	if (iserror(out)){
		fputc('\n', stderr);
		printParseErrorAST(stderr, &out, strstream);
		fputc('\n', stderr);
	}

	node *cursor = &out;

	while (1){
		printf("%zu\n", cursor->childs.size);
		for (size_t i = 0; i < cursor->childs.size; i++){
			if (cursor->childs.arr[i].type == TOKEN_NODE){
				switch (cursor->childs.arr[i].value.token_val.type){
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
				switch (cursor->childs.arr[i].value.nonterminal_val.type){
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

		cursor = cursor->childs.arr + num;

	}

	return 0;
}
