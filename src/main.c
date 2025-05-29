#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "repl.h"
#include "generalFunctions.h"
#include "fileExecutor.h"
#include "main.h"

#define STD_LIB_NAME "GLIStSTD.so"

typedef char *str;

static dl_vec dls;
static context *global;

void loadLibAPI(char *name){
	loadLib(&dls, global, name);
}

static void printHelp(){
	printf("\n");
	printf("Usage: glist [OPTIONS] [FILE]\n\n");
	printf("Options:\n");
	printf("  -h --help\t\t\tshow this text end exit\n");
	printf("  -l --load [LIBRARY FILE]\tload library\n");
	printf("  -p --prerun [LISP FILE] \trun LISP FILE before runningmain file or entering repl\n");
	printf("\n");
}

int main(int argc, char **argv){
	bool run_repl = true;
	char *filename;
	bool terminate = false;

	str_vec libs_to_load = CONSTRUCT(str_vec);
	str_vec files_to_execute = CONSTRUCT(str_vec);

	for (int arg = 1; arg < argc; arg++){
		if (strcmp(argv[arg], "-h") == 0 || strcmp(argv[arg], "--help") == 0){
			printHelp();
			terminate = true;
			break;
		}
		else if (strcmp(argv[arg], "-l") == 0 || strcmp(argv[arg], "--load") == 0){
			if (arg + 1 >= argc){
				fprintf(stderr,"\nMissing argument for %s flag\n\n", argv[arg]);
			}
			METHOD(str_vec, libs_to_load, push, argv[++arg]);
		}
		else if (strcmp(argv[arg], "-p") == 0 || strcmp(argv[arg], "--prerun") == 0){
			if (arg + 1 >= argc){
				fprintf(stderr,"\nMissing argument for %s flag\n\n", argv[arg]);
			}
			METHOD(str_vec, files_to_execute, push, argv[++arg]);
		}
		else if (argv[arg][0] != '-'){
			run_repl = false;
			filename = argv[arg];
			break;
		}
		else{
			fprintf(stderr, "\nWrong option: '%s'! Type 'glist -h' to see help\n\n", argv[arg]);
			terminate = true;
			break;
		}
	}

	if (terminate){
		DESTRUCT(str_vec, libs_to_load);
		DESTRUCT(str_vec, files_to_execute);
		return -1;
	}

	global = derive_context(NULL);
	dls = CONSTRUCT(dl_vec);

	if (run_repl){
		repl(stdin, global, &dls, libs_to_load, files_to_execute);
	}
	else{
		loadLib(&dls, global, STD_LIB_NAME);
		for (size_t i = 0; i < libs_to_load.size; i++){
			loadLib(&dls, global, libs_to_load.arr[i]);
		}
		for (size_t i = 0; i < files_to_execute.size; i++){
			executeFile(global, &dls, files_to_execute.arr[i]);
		}
		executeFile(global, &dls, filename);
	}

	DESTRUCT(str_vec, libs_to_load);
	DESTRUCT(str_vec, files_to_execute);
	putContext(global);
	unloadLibs(&dls);
	DESTRUCT(dl_vec, dls);

	return 0;
}
