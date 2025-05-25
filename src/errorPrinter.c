#include <stdio.h>
#include <inttypes.h>
#include "lexer.h"
#include "parser.h"
#include "ASTtools.h"

#define ERROR_MESSAGE_LIMIT 75

// Assumes, what tree is an error node
static size_t_pair getErrorBounds(node *tree){

	while (1){
		bool cont = false;

		for (size_t i = 0; i < tree->childs.size; i++){
			if (iserror(*tree->childs.arr[i])){
				tree = tree->childs.arr[i];
				cont = true;
				break;
			}
		}

		if (!cont){
			break;
		}
	}

	if (istoken(*tree)){
		size_t_pair out;
		out.first = tree->value.token_val.start;
		out.second = tree->value.token_val.end;
		return out;
	}
	else{
		return getNonterminalBounds(tree);
	}
}

// Tries to print char from stream until one of following:
// 1. All opened brackets are closed
// 2. Stream ended
// 3. Printed chars more than ERROR_MESSAGE_LIMIT
static void printTail(FILE *stream, FILE *src_stream, int32_t brackets_count){

	size_t chars_after_error = 0;
	while (chars_after_error < ERROR_MESSAGE_LIMIT && brackets_count != 0){
		int buffer = fgetc(src_stream);
		if (buffer == EOF){
			fputc('\n', stream);
			return;
		}
		else if (buffer == '('){
			brackets_count++;
		}
		else if (buffer == ')'){
			brackets_count--;
		}

		fputc(buffer, stream);
		chars_after_error++;
	}

	if (chars_after_error >= ERROR_MESSAGE_LIMIT){
		fprintf(stream, "\n\033[90m...\033[0m");
	}

}

// Checks Leftmost token in the tree for start of print
// Finds error token, its end is end of print and chars in this token printed red
void printParseErrorAST(FILE *stream, node *AST, FILE *src_stream){
	size_t_pair whole = getNonterminalBounds(AST);
	size_t_pair error = getErrorBounds(AST);

	size_t curr_pos = ftell(src_stream);
	fseek(src_stream, whole.first, SEEK_SET);

	int32_t brackets_count = 0;

	while (whole.first < error.first){
		int buffer = fgetc(src_stream);
		if (buffer == EOF){
			fputc('\n', stream);
			return;
		}
		else if (buffer == '('){
			brackets_count++;
		}
		else if (buffer == ')'){
			brackets_count--;
		}

		fputc(buffer, stream);
		whole.first++;
	}

	fprintf(stream, "\033[31;4;1m");

	while (whole.first < error.second){
		int buffer = fgetc(src_stream);
		if (buffer == EOF){
			fputc('\n', stream);
			return;
		}
		else if (buffer == '('){
			brackets_count++;
		}
		else if (buffer == ')'){
			brackets_count--;
		}

		fputc(buffer, stream);
		whole.first++;
	}

	fprintf(stream, "\033[0m");

	while (whole.first < whole.second){
		int buffer = fgetc(src_stream);
		if (buffer == EOF){
			fputc('\n', stream);
			return;
		}
		else if (buffer == '('){
			brackets_count++;
		}
		else if (buffer == ')'){
			brackets_count--;
		}

		fputc(buffer, stream);
		whole.first++;
	}

	printTail(stream, src_stream, brackets_count);

	fseek(src_stream, curr_pos, SEEK_SET);
}

void printUnfAST(FILE *stream, node AST);
