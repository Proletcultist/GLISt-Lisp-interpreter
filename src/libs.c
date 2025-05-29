#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include "lispContext.h"
#include "libs.h"
#include "generalFunctions.h"

int dl_compare(dl l, dl r){ return 0; }

#define NAME dl_vec
#define TYPE dl

#include "def_vector.h"

void loadLib(dl_vec *dls, context *global, char *name){
	void *dll = dlopen(name, RTLD_LAZY);
	if (dll == NULL){
		fprintf(stderr, "[Loading lib] \033[31mError\033[0m: %s\n", dlerror());
		return;
	}

	lib_func call = (lib_func)dlsym(dll, "loadAllOps");

	if (call == NULL){
		perror("[Loading lib] \033[31mError\033[0m");
		return;
	}

	call(global);

	METHOD(dl_vec, *dls, push, dll);
}

void unloadLibs(dl_vec *dls){
	for (size_t i = 0; i < dls->size; i++){
		dlclose(dls->arr[i]);
	}
}

