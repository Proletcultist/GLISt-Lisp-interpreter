#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "generalFunctions.h"

typedef char *str;

int str_compare(const char *l, const char *r){
	return strcmp(l,r);
}

size_t jenkins_hash(const uint8_t* key, size_t len) {
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

#define NAME strint_map
#define KEY_TYPE str
#define VALUE_TYPE int

#include "decl_hashMap.h"
#include "def_hashMap.h"

int main(){
	strint_map m = CONSTRUCT(strint_map);

	METHOD(strint_map, m, set, "aboba", 123);
	
	if (METHOD(strint_map, m, get, "nenen") == NULL){
		printf("NULL\n");
	}
	printf("%d\n", *METHOD(strint_map, m, get, "aboba"));
	METHOD(strint_map, m, set, "aboba", 228);
	printf("%d\n", *METHOD(strint_map, m, get, "aboba"));

	DESTRUCT(strint_map, m);

	return 0;
}
