#include <stdio.h>
#include <inttypes.h>
#include "lexer.h"
#include "parser.h"

#define ERROR_MESSAGE_LIMIT 75

// Assumes, what leafs are tokens
static size_t findStartOfTree(node AST){
	while (AST.childs.size != 0){
		AST = AST.childs.arr[0];
	}

	return AST.value.token_val.start;
}

typedef struct size_t_pair{
	size_t first;
	size_t second;
}size_t_pair;
// Assumes, what AST is error token/nonterminal
static size_t_pair findErrorBounds(node AST){
	while (AST.childs.size != 0){
		for (size_t i = 0; i < AST.childs.size; i++){
			if (AST.childs.arr[i].type == TOKEN_NODE && AST.childs.arr[i].value.token_val.type == ERROR_TOKEN || AST.childs.arr[i].type == NONTERMINAL_NODE && AST.childs.arr[i].value.nonterminal_val.type == ERROR_NT){
				AST = AST.childs.arr[i];
				break;
			}
		}
	}

	return (size_t_pair){AST.value.token_val.start, AST.value.token_val.end};
}

// Checks Leftmost token in the tree for start of print
// Finds error token, its end is end of print and chars in this token printed red
void printParseErrorAST(FILE *stream, node AST, FILE *src_stream){
	size_t curr_pos = ftell(src_stream);

	size_t start = findStartOfTree(AST);
	size_t_pair error = findErrorBounds(AST);

	int32_t brackets_count = 0;

	fseek(src_stream, start, SEEK_SET);

	while (start != error.first){
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
		start++;
	}

	fprintf(stream, "\033[31;4;1m");

	while (start != error.second){
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
		start++;
	}

	fprintf(stream, "\033[0m");

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

	fputc('\n', stream);
}

void printUnfAST(FILE *stream, node AST);
