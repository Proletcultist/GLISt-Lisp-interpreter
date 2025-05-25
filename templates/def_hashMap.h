#include <stdlib.h>
#include <memory.h>
#include <inttypes.h>
#include "hashmap_utils.h"
#include "generalFunctions.h"

#define INITIAL_HM_SIZE 113 // Prime number
#define HM_EXTENSION 2
#define MAX_HM_LOAD 90 // % 

#ifndef _CONCAT
	#define _CONCAT(a,b) a##b
#endif

#ifndef CONCAT
	#define CONCAT(a,b) _CONCAT(a,b)
#endif

#ifndef CALLMACRO
	#define CALLMACRO(m, ...) m(__VA_ARGS__)
#endif

#if defined(NAME) && defined(KEY_TYPE) && defined(VALUE_TYPE)

static void CONCAT(NAME, _swap_value)(VALUE_TYPE *l, VALUE_TYPE *r){
	VALUE_TYPE buffer = *l;
	*l = *r;
	*r = buffer;
}

static void CONCAT(NAME, _swap_key)(KEY_TYPE *l, KEY_TYPE *r){
	KEY_TYPE buffer = *l;
	*l = *r;
	*r = buffer;
}


void CONCAT(NAME, _extend)(NAME *self){
	CONCAT(NAME, _node) *old_arr = self->arr;
	size_t old_size = self->size;
	self->size = nextPrime(self->size * HM_EXTENSION);
	self->arr = calloc(self->size, sizeof(CONCAT(NAME, _node)));

	size_t elems = self->elems;
	self->elems = 0;

	for (size_t i = 0; i < old_size && elems;i++){
		// Not empty nor tombstone
		if (old_arr[i].type == VALUE_NODE){
			elems--;
			CONCAT(NAME, _set)(self, old_arr[i].key, old_arr[i].value);
		}
	}

	free(old_arr);
}

void CONCAT(NAME, _check_load)(NAME *self){
	if (self->size == 0){
		self->arr = calloc(INITIAL_HM_SIZE, sizeof(CONCAT(NAME, _node)));
		self->size = INITIAL_HM_SIZE;
	}
	else if (self->elems >= self->size / 100 * MAX_HM_LOAD){
		CONCAT(NAME, _extend)(self);
	}
}

NAME CONCAT(NAME, _construct)(){
	return (NAME){NULL, 0, 0};
}

NAME CONCAT(NAME, _copy_construct)(NAME map){
	NAME out;
	out.arr = malloc(map.size * sizeof(CONCAT(NAME, _node)));
	memcpy(out.arr, map.arr, map.size * sizeof(CONCAT(NAME, _node)));
	out.size = map.size;
	out.elems = map.elems;

	return out;
}

void CONCAT(NAME, _destruct)(NAME self){
	// TODO: Destruct all objects in map
	if (self.size != 0){	
		free(self.arr);
		self.size = 0;
		self.elems = 0;
	}
}

VALUE_TYPE* CONCAT(NAME, _get)(NAME *self, KEY_TYPE key){
	if (self->size == 0 || self->elems == 0){
		return NULL;
	}

	size_t hash = CALLMACRO(HASH, KEY_TYPE, key) % self->size;
	size_t probe_dist = 0;
	
	for (size_t pos = hash;;pos = (pos + 1) % self->size){
		// Empty place
		if (self->arr[pos].type == EMPTY_NODE){
			return NULL;
		}
		// Current elem has prob sequence shorter than new elem
		else if (calcProbDist(pos, self->arr[pos].probe_start, self->size) < probe_dist){
			return NULL;	
		}
		// Element with such key already in table
		else if (self->arr[pos].type != TOMBSTONE_NODE && self->arr[pos].probe_start == hash && CALLMACRO(COMPARE, KEY_TYPE, self->arr[pos].key, key) == 0){
			return &self->arr[pos].value;
		}
		probe_dist++;
	}
}

void CONCAT(NAME, _set)(NAME *self, KEY_TYPE key, VALUE_TYPE value){
	CONCAT(NAME, _check_load)(self);
	self->elems++;

	size_t hash = CALLMACRO(HASH, KEY_TYPE, key) % self->size;
	size_t probe_dist = 0;
	for (size_t pos = hash;;pos = (pos + 1) % self->size){

		// Empty place
		if (self->arr[pos].type == EMPTY_NODE){
			self->arr[pos] = (CONCAT(NAME, _node)){key, value, VALUE_NODE, hash};
			return;
		}

		// Current elem has prob sequence shorter than new elem
		if (calcProbDist(pos, self->arr[pos].probe_start, self->size) < probe_dist){
			// Is tombstone
			if (self->arr[pos].type == TOMBSTONE_NODE){
				self->arr[pos] = (CONCAT(NAME, _node)){key, value, VALUE_NODE, hash};
				return;
			}
			
			// Swaping current and new and continue to try find new place for current
			probe_dist = calcProbDist(pos, self->arr[pos].probe_start, self->size);
			CONCAT(NAME, _swap_key)(&self->arr[pos].key, &key);
			CONCAT(NAME, _swap_value)(&self->arr[pos].value, &value);
			swapSize_t(&self->arr[pos].probe_start, &hash);

			probe_dist++;
			continue;
		}
		
		// Element with such key already in table
		if (self->arr[pos].type != TOMBSTONE_NODE && self->arr[pos].probe_start == hash && CALLMACRO(COMPARE, KEY_TYPE, self->arr[pos].key, key) == 0){
			self->arr[pos].value = value;
			self->elems--;
			return; 
		}

		probe_dist++;
	}
}


void CONCAT(NAME, _remove)(NAME *self, KEY_TYPE key){
	if (self->size == 0 || self->elems == 0){
		return;
	}

	size_t hash = CALLMACRO(HASH, KEY_TYPE, key) % self->size;
	size_t probe_dist = 0;
	
	for (size_t pos = hash;;pos = (pos + 1) % self->size){
		// Empty place
		if (self->arr[pos].type == EMPTY_NODE){
			return;
		}
		// Current elem has prob sequence shorter than new elem
		else if (calcProbDist(pos, self->arr[pos].probe_start, self->size) < probe_dist){
			return;	
		}
		// Element with such key already in table
		else if (self->arr[pos].type != TOMBSTONE_NODE && self->arr[pos].probe_start == hash && CALLMACRO(COMPARE, KEY_TYPE, self->arr[pos].key, key) == 0){
			// Make tombstone
			self->arr[pos].type = TOMBSTONE_NODE;
			self->elems--;
			return;
		}
		probe_dist++;
	}
}

#undef NAME
#undef KEY_TYPE
#undef VALUE_TYPE

#endif
