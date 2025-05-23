#pragma once

#include "lexer.h"
#include "parser.h"

void printParseErrorAST(FILE *stream, node *AST, FILE *src_stream);

void printUnfAST(FILE *stream, node *AST);
