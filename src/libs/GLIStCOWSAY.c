#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreterAPI.h"

#define EYES 'o'
#define UPPERBUBBLE '\\'
#define LOWERBUBBLE '\\'

static void printcow(){
	printf("        %c   ^__^\n\
         %c  (%c%c)\\_______\n\
            (__)\\       )\\/\\\n\
                ||----w |\n\
                ||     ||\n", UPPERBUBBLE, LOWERBUBBLE, EYES, EYES);
}

static void printbubble(char *str){
	printf(" ");

	size_t argscharcount = strlen(str);
	size_t padding = 2;

	for(size_t counter = 0; counter < argscharcount + padding * 2; counter++) {
		printf("_");
	}

	printf("\n<");

	for (size_t i = 0; i < padding; i++){
		printf(" ");
	}

	printf(str);

	for (size_t i = 0; i < padding; i++){
		printf(" ");
	}

	printf(">\n ");

	for(int counter = 0; counter < argscharcount + padding * 2; counter++) {
		printf("-");
	}

	printf("\n");
}

lispObject* cowsay(void *global, void *local, lispCFunction *func, lispList *args){
	if (args->list.size != 2){
		fprintf(stderr, "[Evaluating] \033[31mError\033[0m Wrong anount of arguments, expected: 1, got: %zu\n", args->list.size - 1);
		throughError(args->source);
		return &ERROR_OBJECT;
	}

	lispObject *theOnlyOneArg = eval((context*)global, (context*)local, args->list.arr[1]);
	if (theOnlyOneArg->type == ERROR_LISP){
		return &ERROR_OBJECT;
	}

	char *str = NULL;
	size_t str_s;
	
	FILE *str_stream = open_memstream(&str, &str_s);

	printObject(str_stream, theOnlyOneArg);

	fclose(str_stream);

	printbubble(str);
	printcow();

	free(str);

	return theOnlyOneArg;
}

void loadAllOps(context *ctx){
	addOperator(ctx, "COWSAY", cowsay, true);
}
