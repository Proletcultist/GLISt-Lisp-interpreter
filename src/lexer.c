#include <ctype.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lexer.h"
#include "iotools.h"

lexer lexer_construct(FILE *stream, bool setposes){
	// Test if stream is seekable
	if (setposes && ftell(stream) < 0){
		fprintf(stderr, "[Lexing] \033[93mWarning\033[0m Input stream isn't seekable. Where will be no info about error location in lines\n");
		setposes = false;
	}
	
	return (lexer){stream, setposes};
}

token_type lexer_peekToken(lexer *l){
	int input = fpeek(l->stream);

	if (isdigit(input)){
		return INT_TOKEN;
	}

	switch (input){
		case '"':
			return STR_TOKEN;
		case '\n':
		case '\t':
		case ' ':
			return SEP_TOKEN;
		case '\'':
			return QUOTE_TOKEN;
		case '(':
			return OPEN_PARENTHESIS_TOKEN;
		case ')':
			return CLOSE_PARENTHESIS_TOKEN;
		case EOF:
			return END_OF_INPUT;
		default:
	}

	if (isprint(input)){
		return SYMBOL_TOKEN;
	}

	return ERROR_TOKEN;
}

static token readIntToken(FILE *stream, bool setposes){
	token out;
	out.type = INT_TOKEN;
	out.value.int_val = 0;
	out.start = 0;
	out.end = 0;

	if (setposes){
		out.start = ftell(stream);
	}

	while (isdigit(fpeek(stream))){
		out.value.int_val *= 10;
		out.value.int_val += fgetc(stream) - '0';
	}

	if (setposes){
		out.end = ftell(stream);
	}

	return out;
}
static token readSymbolToken(FILE *stream, bool setposes){
	token out;
	out.type = SYMBOL_TOKEN;
	out.value.str_val = NULL;
	out.start = 0;
	out.end = 0;

	size_t str_len = 0;

	if (setposes){
		out.start = ftell(stream);
	}

	FILE *strstream = open_memstream(&out.value.str_val, &str_len);
	if (strstream == NULL){
		perror("[Lexing] \033[31mError\033[0m Can't open memstream");
		return (token){ERROR_TOKEN, {.int_val = -1}, 0, 0};
	}

	int buffer;
	while ((buffer = fpeek(stream)) != ' ' && buffer != '\n' && buffer != '\t' && buffer != EOF && buffer != '(' && buffer != ')'){
		fputc(toupper(fgetc(stream)), strstream);
	}
	fclose(strstream);

	if (setposes){
		out.end = ftell(stream);
	}

	return out;
}
static token readStrToken(FILE *stream, bool setposes){
	token out;
	out.type = STR_TOKEN;
	out.value.str_val = NULL;
	out.start = 0;
	out.end = 0;

	size_t str_len = 0;

	if (setposes){
		out.start = ftell(stream);
	}

	FILE *strstream = open_memstream(&out.value.str_val, &str_len);
	if (strstream == NULL){
		perror("[Lexing] \033[31mError\033[0m Can't open memstream");
		return (token){ERROR_TOKEN, {.int_val = -1}, 0, 0};
	}

	int buffer;

	if ((buffer = fgetc(stream)) != '"'){
		fprintf(stderr, "[Lexing] \033[31mError\033[0m Expected \", found '%c'\n", (char)buffer); 
		return (token){ERROR_TOKEN, {.int_val = -1}, 0, 0};
	}
	while ((buffer = fpeek(stream)) != '"'){
		if (buffer == EOF || buffer == '\n'){
			fprintf(stderr, "[Lexing] \033[31mError\033[0m Mismatching quote\n");
			if (setposes){
				return (token){ERROR_TOKEN, {.int_val = -1}, out.start, out.start + 1};
			}
			else{
				return (token){ERROR_TOKEN, {.int_val = -1}, 0, 0};
			}
		}

		fputc(fgetc(stream), strstream);
	}
	fgetc(stream);		// Get " after string
	fclose(strstream);

	if (setposes){
		out.end = ftell(stream);
	}

	return out;
}

static token readSepToken(FILE *stream, bool setposes){
	token out;
	out.type = SEP_TOKEN;
	out.start = 0;
	out.end = 0;

	if (setposes){
		out.start = ftell(stream);
		out.end = out.start;
	}

	int buffer;
	if ((buffer = fpeek(stream)) != '\n' && buffer != '\t' && buffer != ' '){
		fprintf(stderr, "[Lexer] \033[31mError\033[0m Expected separator\n");
		return (token){ERROR_TOKEN, {.int_val = -1}, out.start, out.start + 1};
	}

	while ((buffer = fpeek(stream)) == '\n' || buffer == '\t' || buffer == ' '){
		fgetc(stream);
		out.end++;
	}

	return out;
}

token lexer_getToken(lexer *l, token_type type){
	switch (type){
		case INT_TOKEN:
			return readIntToken(l->stream, l->setposes);
		case STR_TOKEN:
			return readStrToken(l->stream, l->setposes);
		case SYMBOL_TOKEN:
			return readSymbolToken(l->stream, l->setposes);
		case SEP_TOKEN:
			return readSepToken(l->stream, l->setposes);
		case QUOTE_TOKEN:
		case OPEN_PARENTHESIS_TOKEN:
		case CLOSE_PARENTHESIS_TOKEN:
			fgetc(l->stream);		// Uncought error
			return (token){type, {.int_val = -1}, (l->setposes ? ftell(l->stream) - 1 : 0), (l->setposes ? ftell(l->stream) : 0)};
		case END_OF_INPUT:
			return (token){type, {.error_char = fgetc(l->stream)}, 0, 0};
		case ERROR_TOKEN:
			return (token){type, {.int_val = -1}, 0, 0};
	}
}

void lexer_destruct(lexer l){}
