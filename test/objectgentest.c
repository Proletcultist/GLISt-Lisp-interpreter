#include <stdio.h>
#include <inttypes.h>
#include "parser.h"
#include "lexer.h"
#include "generalFunctions.h"
#include "errorPrinter.h"
#include "lispObject.h"
#include "objectBuilder.h"

int main(){
	char test[] = "('fun 2 \"sasas\" (\n- 9 8\n))";

	FILE *strstream = fmemopen(test, sizeof(test) - 1, "r");

	lexer l = CONSTRUCT(lexer, strstream, true);
	
	node *out = parseExprToAST(l);
	if (iserror(*out)){
		fputc('\n', stderr);
		printParseErrorAST(stderr, out, strstream);
		fputc('\n', stderr);
	}

	lispObject *obj = visitExpr(out);

	return 0;
}
