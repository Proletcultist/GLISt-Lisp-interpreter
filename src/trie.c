#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include "trie.h"

#define NAME str_vec
#define TYPE str

#include "def_vector.h"

trie* trie_construct(){
	return calloc(1, sizeof(trie));
}

void trie_destruct(trie *t){
	for (size_t i = 0; i < 256; i++){
		if (t->childs[i] != NULL){
			trie_destruct(t->childs[i]);
		}
	}

	if (t->is_end){
		free(t->value);
	}
	free(t);
}

void trie_add(trie *t, char *s){
	char *start = s;

	while (*s != '\0' && t->childs[toupper(*s)] != NULL){
		t = t->childs[toupper(*s)];
		s++;
	}

	if (*s == '\0' && t->is_end){
		return;
	}
	else if (*s == '\0' && !t->is_end){
		t->is_end = true;
		t->value = malloc(strlen(start) + 1);
		strcpy(t->value, start);
	}
	else{
		while (*s != '\0'){
			t->childs[toupper(*s)] = calloc(1, sizeof(trie));
			t->childs_amount++;
			t = t->childs[toupper(*s)];
			s++;
		}
		t->is_end = true;
		t->value = malloc(strlen(start) + 1);
		strcpy(t->value, start);
	}
}

void trie_delete(trie *t, char *s){
	trie *first_one_childed = NULL;
	char *first_one_next = NULL;

	while (*s != '\0' && t->childs[toupper(*s)] != NULL){
		if (t->childs_amount > 1){
			first_one_childed = t->childs[toupper(*s)];
			first_one_next = s + 1;
		}
		t = t->childs[toupper(*s)];
		s++;
	}

	if (*s != '\0'){
		return;
	}

	if (t->childs_amount != 0){
		t->is_end = false;
		return;
	}

	while (*first_one_next != '\0'){
		trie *buffer = first_one_childed->childs[toupper(*first_one_next)];
		free(first_one_childed);

		first_one_childed = buffer;
		first_one_next++;
	}
	free(first_one_childed);
}

static void printTreeRec(trie_node *node, FILE *suggestions_stream, size_t *suggestions_amount){
	if (node->is_end){
		(*suggestions_amount)++;
		fprintf(suggestions_stream, "%s ", node->value);
	}

	for (size_t i = 0; i < 256; i++){
		if (node->childs[i] != NULL){
			printTreeRec(node->childs[i], suggestions_stream, suggestions_amount);
		}
	}
}

void findByPrefix(trie *t, char *prefix, FILE *outstream, FILE *suggestions_stream, size_t *suggestions_amount){
	char *s = prefix;
	*suggestions_amount = 0;

	while (*s != '\0' && t->childs[toupper(*s)] != NULL){
		t = t->childs[toupper(*s)];
		s++;
	}

	if (*s != '\0'){
		return;
	}

	while (t->childs_amount == 1 && !t->is_end){
		for (size_t i = 0; i < 256; i++){
			if (t->childs[i] != NULL){
				fputc((char)i, outstream);
				t = t->childs[i];
				break;
			}
		}
	}

	if (t->childs_amount == 0){
		fputc(' ', outstream);
		return;
	}

	printTreeRec(t, suggestions_stream, suggestions_amount);
}
