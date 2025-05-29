#pragma once

#include <stdio.h>
#include <stdbool.h>

typedef char *str;

int str_compare(const char *l, const char *r);

#define NAME str_vec
#define TYPE str

#include "decl_vector.h"

typedef struct trie_node{
	struct trie_node *childs[256];
	bool is_end;
	char *value;
	size_t childs_amount;
}trie_node;

typedef trie_node trie;

trie* trie_construct();
void trie_destruct(trie *t);
void trie_add(trie *t, char *s);
void trie_delete(trie *t, char *s);

void findByPrefix(trie *t, char *prefix, FILE *outstream, FILE *suggestions_stream, size_t *suggestions_amount);
