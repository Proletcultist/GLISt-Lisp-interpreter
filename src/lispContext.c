#include <string.h>
#include <inttypes.h>
#include "lispContext.h"
#include "generalFunctions.h"

#define NAME str_obj_p_map
#define KEY_TYPE str
#define VALUE_TYPE lispObject_p

#include "def_hashMap.h"

int str_compare(const char *l, const char *r){
	return strcmp(l,r);
}

static size_t jenkins_hash(const uint8_t* key, size_t len) {
    size_t hash = 0;
    for (size_t i = 0; i < len; i++) {
        hash += key[i];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

size_t str_hash(char *s){
	return jenkins_hash((const uint8_t*)s, strlen(s));
}

context *context_construct(){
	context *out = malloc(sizeof(context));
	out->ref_count = 1;
	out->map = CONSTRUCT(str_obj_p_map);

	return out;
}

context* derive_context(context *ctx){
	if (ctx == NULL){
		return context_construct();
	}


	context *derived = malloc(sizeof(context));
	derived->ref_count = 1;
	derived->map = COPY_CONSTRUCT(str_obj_p_map, ctx->map);

	for (size_t i = 0; i < derived->map.size; i++){
		if (derived->map.arr[i].type == VALUE_NODE){
			derived->map.arr[i].value = lispObject_borrow(derived->map.arr[i].value);
			derived->map.arr[i].key = malloc(strlen(derived->map.arr[i].key) + 1);
			strcpy(derived->map.arr[i].key, ctx->map.arr[i].key);
		}
	}

	return derived;
}

context* borrow_context(context *ctx){
	ctx->ref_count++;
	return ctx;
}

static void destructAllObjects(context *ctx){
	for (size_t i = 0; i < ctx->map.size; i++){
		if (ctx->map.arr[i].type == VALUE_NODE){
			lispObject_destruct(ctx->map.arr[i].value);
			free(ctx->map.arr[i].key);
		}
	}
}

void putContext(context *ctx){
	ctx->ref_count--;
	if (ctx->ref_count == 0){
		destructAllObjects(ctx);
		DESTRUCT(str_obj_p_map, ctx->map);
		free(ctx);
	}
}
