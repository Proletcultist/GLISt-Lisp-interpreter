#pragma once

#include "parser.h"

typedef struct size_t_pair{
	size_t first;
	size_t second;
}size_t_pair;

size_t_pair getNonterminalBounds(node *nt);
void spreadError(node *error_src);
