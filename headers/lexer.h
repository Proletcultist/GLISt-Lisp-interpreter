#pragma once

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

typedef enum token_type{
	INT_TOKEN,
	STR_TOKEN,
	SYMBOL_TOKEN,
	SEP_TOKEN,
	QUOTE_TOKEN,
	OPEN_PARENTHESIS_TOKEN,
	CLOSE_PARENTHESIS_TOKEN,
	END_OF_INPUT,
	ERROR_TOKEN
}token_type;

typedef union token_value{
	int32_t int_val;
	char *str_val;
	char error_char;
}token_value;

// [start;end) - part of source string, where token is located
typedef struct token{
	token_type type;
	token_value value;

	size_t start;
	size_t end;
}token;

typedef struct lexer{
	FILE *stream;
	bool setposes;
}lexer;

lexer lexer_construct(FILE *stream, bool setposes);
// Only checks which token type can start with char under cursor
// Doesn't check for correctness of this token
token_type lexer_peekToken(lexer *l);
// Tries to create token of specified type, starting from cursor
// Returns ERROR_TOKEN, if unsuccessfull, prints to stderr info about error
token lexer_getToken(lexer *l, token_type type);
void lexer_destruct(lexer l);


void freeToken(token t);
