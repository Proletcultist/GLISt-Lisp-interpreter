#include "ASTtools.h"
#include "parser.h"

size_t_pair getNonterminalBounds(node *nt){
	size_t_pair out;

	// Go to the leftmost leaf
	node *cursor = nt;
	while (cursor->childs.size != 0){
		cursor = cursor->childs.arr[0];
	}
	out.first = cursor->value.token_val.start;

	// Go to the rigtmost leaf
	cursor = nt;
	while (cursor->childs.size != 0){
		cursor = cursor->childs.arr[cursor->childs.size - 1];
	}
	out.second = cursor->value.token_val.end;

	return out;
}

void spreadError(node *error_src){
	while (error_src != NULL){
		if (error_src->type == NONTERMINAL_NODE){
			error_src->value.nonterminal_val.type = ERROR_NT;
		}
		else{
			error_src->value.token_val.type = ERROR_TOKEN;
		}
		error_src = error_src->parent;
	}
}
