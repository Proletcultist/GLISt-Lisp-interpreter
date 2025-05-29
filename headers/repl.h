#pragma once

#include <stdio.h>
#include "lispContext.h"
#include "libs.h"
#include "main.h"

int char_compare(char l, char r);

#define NAME chars_vec
#define TYPE char

#include "decl_vector.h"

void addAutocomplete(char *str);
void deleteAutocomplete(char *str);

void repl(FILE *stream, context *global, dl_vec *dls, str_vec libs, str_vec files);
