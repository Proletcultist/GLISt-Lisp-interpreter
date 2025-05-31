#include <stdio.h>
#include "fileExecutor.h"
#include "generalFunctions.h"
#include "objectBuilder.h"
#include "iotools.h"
#include "lexer.h"
#include "errorPrinter.h"
#include "libs.h"
#include "ASTtools.h"

static void printPosInFile(FILE *file, char *filename, size_t_pair pos){
	fprintf(stderr, filename);
	fprintf(stderr, ":");

	size_t init_pos = ftell(file);

	fseek(file, 0, SEEK_SET);

	size_t lineno = 1;

	for (size_t curs_pos = 0; curs_pos < pos.first; curs_pos++){
		int c = fgetc(file);
		if (c == '\n'){
			lineno++;
		}
	}

	fprintf(stderr, "%zu", lineno);

	fseek(file, init_pos, SEEK_SET);
}

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
			fprintf(stderr, "\n[Parsing] \033[31mError\033[0m Mismatching paranthesis\n");

			fprintf(stderr, "on ");
			printPosInFile(read_stream, filename, getNonterminalBounds(out));
			fprintf(stderr, "\n");

			fputc('\n', stderr);
			printUnfAST(stderr, out, read_stream);
			fputc('\n', stderr);

			destruct_node_rec(out);

			break;
		}

		fputc('\n', stdout);

		if (iserror(*out)){
			fprintf(stderr, "on ");
			printPosInFile(read_stream, filename, getNonterminalBounds(out));
			fprintf(stderr, "\n");

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
