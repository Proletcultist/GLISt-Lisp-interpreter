#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include "lexer.h"
#include "generalFunctions.h"
#include "errorPrinter.h"

#define NAME int_vec
#define TYPE int

#include "decl_vector.h"

#define NAME int_vec
#define TYPE int

#include "def_vector.h"

int main(){
	char test[] = "(+ 124 \"abod d df true)";

	FILE *strstream = fmemopen(test, sizeof(test) - 1, "r");

	lexer l = CONSTRUCT(lexer, strstream, true);

	while (METHOD(lexer, l, peekToken) != END_OF_INPUT){
		token t = METHOD(lexer, l, getToken, METHOD(lexer, l, peekToken));
		switch (t.type){
			case INT_TOKEN:
				printf("<[%zu,%zu] %"PRId32">", t.start, t.end, t.value.int_val);
				break;
			case STR_TOKEN:
			case SYMBOL_TOKEN:	
				printf("<[%zu,%zu] %s>", t.start, t.end, t.value.str_val);
				break;
			case SEP_TOKEN:
				printf("<[%zu,%zu] sep>", t.start, t.end);
				break;
			case NEW_LINE_TOKEN:
				printf("<[%zu,%zu] NL>", t.start, t.end);
				break;
			case QUOTE_TOKEN:
				printf("<[%zu,%zu] quote>", t.start, t.end);
				break;
			case OPEN_PARENTHESIS_TOKEN:
				printf("<[%zu,%zu] op>", t.start, t.end);
				break;
			case CLOSE_PARENTHESIS_TOKEN:
				printf("<[%zu,%zu] cp>", t.start, t.end);
				break;
			case ERROR_TOKEN:
				fprintf(stderr, "\n");
				printLineWithError(stderr, test, t);
				fprintf(stderr, "\n");
				goto aboba;
		}
	}

	DESTRUCT(lexer, l);

aboba:
	fclose(strstream);
	printf("\n");

	return 0;
}
