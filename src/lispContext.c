#include <string.h>
#include <inttypes.h>
#include "lispContext.h"

#define NAME context
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
