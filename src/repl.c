#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include "repl.h"
#include "generalFunctions.h"
#include "objectBuilder.h"
#include "fileExecutor.h"
#include "main.h"
#include "iotools.h"
#include "lexer.h"
#include "errorPrinter.h"
#include "libs.h"
#include "interpreterAPI.h"
#include "trie.h"

#define STD_LIB_NAME "GLIStSTD.so"
#define MAX_SUGGESTIONS 2

int char_compare(char l, char r){ return 0; }

#define NAME chars_vec
#define TYPE char

#include "def_vector.h"

// AUTOCOMPLETITION ------------------------------------

static trie *autocomplete = NULL;

void addAutocomplete(char *str);
void deleteAutocomplete(char *str);

char* getAutoPrefix(chars_vec input);

// TERMINAL --------------------------------------------

#define END_OF_FILE 4
#define TAB_SIZE 4
#define TAB_CHAR "    "
#define BACKSPACE_TAB "\b\b\b\b    \b\b\b\b"

static struct termios saved_settings;
static int BACKSPACE;

static void setupTerminal();
static void restoreTerminalSettings();

// -----------------------------------------------------

void addAutocomplete(char *str){
	if (autocomplete == NULL){
		return;
	}
	trie_add(autocomplete, str);
}

void deleteAutocomplete(char *str){
	if (autocomplete == NULL){
		return;
	}
	trie_delete(autocomplete, str);
}

// NULL if no prefix
char* getAutoPrefix(chars_vec input){
	char last = input.arr[input.size - 1];

	size_t index = input.size;

	while (index != 0 && input.arr[index - 1] != '(' && input.arr[index - 1] != ')' && input.arr[index - 1] != ' ' && input.arr[index - 1] != '\n' && input.arr[index - 1] != '\t'){
		index--;
	}

	while (index != input.size && (isdigit(input.arr[index]) || input.arr[index] == '\'' || input.arr[index] == '"')){
		index++;
	}
			
	if (index == input.size){
		return NULL;
	}
	else{
		return input.arr + index;
	}

}

static void setupTerminal(){
	// Saving initial settings
	tcgetattr(STDIN_FILENO, &saved_settings);

	struct termios tty;

	tcgetattr(STDIN_FILENO, &tty);

	// Disabling canonical mode
	tty.c_lflag &= ~ICANON;
	// Disabling echoeing
	tty.c_lflag &= ~ECHO;
	// Read minimum 1 character
	tty.c_cc[VMIN] = 1;

	tcsetattr(STDIN_FILENO, TCSANOW, &tty);

	BACKSPACE = tty.c_cc[VERASE];
}

static void restoreTerminalSettings(){
	tcsetattr(STDIN_FILENO, TCSANOW, &saved_settings);
}

void repl(FILE *stream, context *global, dl_vec *dls, str_vec libs, str_vec files){

	srand(time(NULL));
	char *st[] = {"\033[4mSt\033[0mable", "\033[4mSt\033[0mrong", "\033[4mSt\033[0mepan", "\033[4mSt\033[0mandart", "\033[4mSt\033[0myled", "\033[4mSt\033[0mressed"};

	// Print hello message
	if ((unsigned)rand() % 50 == 1){
		fprintf(stdout, "GLISt v.1.0\nI'd just like to interject for a moment. What you're referring to as GLISt, is in fact, GNU/GLISt - Great Lisp Interpreter (\033[4mSt\033[0mallman)\n\n");
	}
	else{
		char *random = st[(unsigned)rand() % (sizeof(st) / sizeof(char*))];
		fprintf(stdout, "GLISt v1.0\nGreat Lisp Interpreter (");
		fprintf(stdout, random);
		fprintf(stdout, ")\n\n");
	}

	// Init autocompletition trie
	autocomplete = CONSTRUCT(trie);

	// Load std lib
	loadLib(dls, global, STD_LIB_NAME);

	for (size_t i = 0; i < libs.size; i++){
		loadLib(dls, global, libs.arr[i]);
	}
	for (size_t i = 0; i < files.size; i++){
		executeFile(global, dls, files.arr[i]);
	}

	// Init buffer for input
	chars_vec input = CONSTRUCT(chars_vec);

	setupTerminal();
	fprintf(stdout, "\n>>> ");

	while (1){
		int c = fgetc(stdin);

revisit:

		if (c == '\t'){
			// Autocompletition logic
			METHOD(chars_vec, input, push, '\0');

			input.size--;
			
			char *prefix = getAutoPrefix(input);

			input.size++;

			if (prefix == NULL){
				METHOD(chars_vec, input, pop);
				METHOD(chars_vec, input, push, '\t');
				fprintf(stdout, TAB_CHAR);
				continue;
			}


			char *outb = NULL;
			size_t outs = 0;
			char *suggb = NULL;
			size_t suggs = 0;

			FILE *outstream = open_memstream(&outb, &outs);
			FILE *suggestions = open_memstream(&suggb, &suggs);
			size_t sugg_amount;

			findByPrefix(autocomplete, prefix, outstream, suggestions, &sugg_amount);
			fclose(outstream);
			fclose(suggestions);

			METHOD(chars_vec, input, pop);

			// No matching
			if (sugg_amount == 0 && outs == 0){
				free(outb);
				free(suggb);
				METHOD(chars_vec, input, push, '\t');
				fprintf(stdout, TAB_CHAR);
				continue;
			}

			// Output out
			for (size_t i = 0; i < outs; i++){
				METHOD(chars_vec, input, push, outb[i]);
				fputc(outb[i], stdout);
			}

			// Check if there are multiple suggestions
			if (sugg_amount == 0){
				free(outb);
				free(suggb);
				continue;
			}

			c = fgetc(stdin);
			if (c == '\t'){
				if (sugg_amount > MAX_SUGGESTIONS){
					fprintf(stdout, "\nDisplay all %zu possibilities? (y or n)", sugg_amount);
					while (1){
						c = fgetc(stdin);
						if (c == 'y'){
							fprintf(stdout, "\n");
							fprintf(stdout, suggb);
							fprintf(stdout, "\n>>> ");
							for (size_t i = 0; i < input.size; i++){
								fputc(input.arr[i], stdout);
							}
							break;
						}
						else if (c == 'n'){
							fprintf(stdout, "\n>>> ");
							for (size_t i = 0; i < input.size; i++){
								fputc(input.arr[i], stdout);
							}
							break;
						}
					}
					free(suggb);
					free(outb);
					continue;
				}
				else{
					fprintf(stdout, "\n");
					fprintf(stdout, suggb);
					fprintf(stdout, "\n>>> ");
					for (size_t i = 0; i < input.size; i++){
						fputc(input.arr[i], stdout);
					}
					free(suggb);
					free(outb);
					continue;
				}
			}
			else{
				free(outb);
				free(suggb);
				goto revisit;
			}
			
		}
		else if (c == BACKSPACE){
			if (input.size != 0){
				char deleted = METHOD(chars_vec, input, pop);
				if (deleted == '\t'){
					fprintf(stdout, BACKSPACE_TAB);
				}
				else if (deleted == '\n'){
					METHOD(chars_vec, input, push, '\n');
				}
				else{
					fprintf(stdout, "\b \b");
				}
			}
		}
		else if (c == END_OF_FILE){
			break;
		}
		else if (c == '\n'){
			fputc(c, stdout);

			if (input.size == 0){
				fprintf(stdout, ">>> ");
				continue;
			}

			// Process input logic
			FILE *read_stream = fmemopen(input.arr, input.size, "r");
			lexer l = CONSTRUCT(lexer, read_stream, true);

			bool unf = false;

			// Skip leading seps
			while (METHOD(lexer, l, peekToken) == SEP_TOKEN){
				METHOD(lexer, l, getToken, SEP_TOKEN);
			}

			while (fpeek(read_stream) != EOF){
				node *out = parseExprToAST(l);

				if (isunfinished(*out)){
					// \n not just end of line, but separator, so push it to input
					METHOD(chars_vec, input, push, '\n');

					unf = true;
					destruct_node_rec(out);
					break;
				}

				fputc('\n', stdout);

				if (iserror(*out)){
					fputc('\n', stderr);
					printParseErrorAST(stderr, out, read_stream);
					fputc('\n', stderr);

					destruct_node_rec(out);

					break;
				}

				lispObject *generated = visitExpr(out);
				lispObject *evaluated = eval(global, NULL, generated);

				if (evaluated->type == ERROR_LISP){
					fputc('\n', stderr);
					printParseErrorAST(stderr, out, read_stream);
					fputc('\n', stderr);
				}
				else{
					printObject(stdout, evaluated);
					fputc('\n', stdout);
					lispObject_destruct(evaluated);
				}

				lispObject_destruct(generated);
				destruct_node_rec(out);

				// Skip seps after expr
				while (METHOD(lexer, l, peekToken) == SEP_TOKEN){
					METHOD(lexer, l, getToken, SEP_TOKEN);
				}
			}

			fclose(read_stream);
			if (unf){
				fprintf(stdout, "... ");
			}
			else{
				fprintf(stdout, "\n>>> ");
				input.size = 0;
			}

		}
		else if (isprint(c)){
			METHOD(chars_vec, input, push, (char)c);
			fputc(c, stdout);
		}
	}

	fputc('\n', stdout);
	restoreTerminalSettings();
	trie_destruct(autocomplete);
	DESTRUCT(chars_vec, input);
}
