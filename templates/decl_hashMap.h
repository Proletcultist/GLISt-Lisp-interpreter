#include <inttypes.h>
#include <stddef.h>

#ifndef _CONCAT
	#define _CONCAT(a,b) a##b
#endif

#ifndef CONCAT
	#define CONCAT(a,b) _CONCAT(a,b)
#endif

#ifndef HASHMAP_NODE_TYPE_42345678983221392132132456956
typedef enum HashMap_node_type{
	EMPTY_NODE = 0,
	VALUE_NODE,
	TOMBSTONE_NODE
}HashMap_node_type;
#define HASHMAP_NODE_TYPE_42345678983221392132132456956
#endif


#if defined(NAME) && defined(KEY_TYPE) && defined(VALUE_TYPE)

typedef struct CONCAT(NAME, _node){
	KEY_TYPE key;
	VALUE_TYPE value;
	HashMap_node_type type;
	size_t probe_start;
}CONCAT(NAME, _node);

typedef struct NAME{
	CONCAT(NAME, _node) *arr;
	size_t size;
	size_t elems; 
}NAME;


NAME CONCAT(NAME, _construct)();

void CONCAT(NAME, _destruct)(NAME self);

VALUE_TYPE* CONCAT(NAME, _get)(NAME *self, KEY_TYPE key);

void CONCAT(NAME, _set)(NAME *self, KEY_TYPE key, VALUE_TYPE value);

void CONCAT(NAME, _remove)(NAME *self, KEY_TYPE key);

#undef NAME
#undef KEY_TYPE
#undef VALUE_TYPE

#endif
