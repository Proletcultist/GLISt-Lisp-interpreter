#include <stdio.h>
#include <stdlib.h>
#include "trie.h"
#include "generalFunctions.h"

int main(){
	trie *t = CONSTRUCT(trie);

	trie_add(t, "aboba");
	trie_add(t, "abvgd");
	trie_add(t, "ab");

	char *outb = NULL;
	size_t outb_s = 0;
	char *suggb = NULL;
	size_t sugb_s = 0;

	FILE *outstream = open_memstream(&outb, &outb_s);
	FILE *suggestions = open_memstream(&suggb, &sugb_s);
	size_t sugg_amount;

	findByPrefix(t, "abo", outstream, suggestions, &sugg_amount);
	fclose(outstream);
	fclose(suggestions);

	printf("out: '%s'\n", outb);
	printf("suggestions: '%s'\n", suggb);

	free(outb);
	free(suggb);
	trie_destruct(t);

	return 0;
}
