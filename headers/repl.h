#pragma once

#include <stdio.h>

int char_compare(char l, char r);

#define NAME chars_vec
#define TYPE char

#include "decl_vector.h"

void addAutocomplete(char *str);
void deleteAutocomplete(char *str);

void repl(FILE *stream);
