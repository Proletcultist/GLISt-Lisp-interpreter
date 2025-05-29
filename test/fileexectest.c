#include "fileExecutor.h"

#define STD_LIB_NAME "GLIStSTD.so"

int main(){
	// Init global context
	context *global = derive_context(NULL);

	// Init dls vector
	dl_vec dls = CONSTRUCT(dl_vec);
	// Load std lib
	loadLib(&dls, global, STD_LIB_NAME);

	executeFile(global, &dls, "test.lisp");

	putContext(global);
	unloadLibs(&dls);
	DESTRUCT(dl_vec, dls);

	return 0;
}
