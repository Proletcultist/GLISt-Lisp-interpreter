#pragma once
#include <dlfcn.h>
#include "lispContext.h"

typedef void *dl;
typedef void (*lib_func)(context *global);

int dl_compare(dl l, dl r);

#define NAME dl_vec
#define TYPE dl

#include "decl_vector.h"

void loadLib(dl_vec *dls, context *global, char *name);
void unloadLibs(dl_vec *dls);
