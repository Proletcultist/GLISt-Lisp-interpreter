#include <stdio.h>
#include "fileExecutor.h"
#include "generalFunctions.h"
#include "objectBuilder.h"
#include "iotools.h"
#include "lexer.h"
#include "errorPrinter.h"
#include "libs.h"

void executeFile(context *global, dl_vec *dls, char *filename){
	FILE *read_stream = fopen(filename, "r");
	if (read_stream == NULL){
		perror("[File executing] \033[31mError\033[0m");
		return;
	}

	lexer l = CONSTRUCT(lexer, read_stream, true);

	// Skip leading seps
	while (METHOD(lexer, l, peekToken) == SEP_TOKEN){
		METHOD(lexer, l, getToken, SEP_TOKEN);
	}

	while (fpeek(read_stream) != EOF){
		node *out = parseExprToAST(l);

		if (isunfinished(*out)){
			// Print unfinished error
			break;
		}

		fputc('\n', stdout);

		if (iserror(*out)){
			fputc('\n', stderr);
			printParseErrorAST(stderr, out, read_stream);
			fputc('\n', stderr);

			destruct_node_rec(out);

			break;
		}

		lispObject *generated = visitExpr(out);
		lispObject *evaluated = eval(global, NULL, generated);

		if (evaluated->type == ERROR_LISP){
			fputc('\n', stderr);
			printParseErrorAST(stderr, out, read_stream);
			fputc('\n', stderr);
		}
		else{
			lispObject_destruct(evaluated);
		}

		lispObject_destruct(generated);
		destruct_node_rec(out);

		// Skip seps after expr
		while (METHOD(lexer, l, peekToken) == SEP_TOKEN){
			METHOD(lexer, l, getToken, SEP_TOKEN);
		}
	}

	fputc('\n', stdout);
	fclose(read_stream);
}
