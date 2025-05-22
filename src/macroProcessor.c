#include "macroProcessor.h"
#include "lexer.h"
#include "parser.h"

void macroProcess(context *ctx, node *tree){
	for (size_t i = 0; i < tree->childs.size; i++){
		macroProcess(ctx, tree->childs.arr + i);
	}

	// Check if this node isnt expression with calling something (func or macro)
	
	if (tree->type != NONTERMINAL_NODE || tree->value.nonterminal_val.type != EXPR_NT){
		return;
	}
	if (tree->childs.size != 1 || tree->childs.arr[0].value.nonterminal_val.type != LIST_NT){
		return;
	}

	node this_list_c = tree->childs.arr[0].childs.arr[1].type == NONTERMINAL_NODE ? tree->childs.arr[0].childs.arr[1] : tree->childs.arr[0].childs.arr[2];

	if (this_list_c.childs.size == 0){
		return;
	}
	if (this_list_c.childs.arr[0].childs.size != 1 || this_list_c.childs.arr[0].childs.arr[0].value.nonterminal_val.type != VALUE_NT){
		return;
	}
	
	token this_symbol = this_list_c.childs.arr[0].childs.arr[0].childs.arr[0].value.token_val;

	if (this_symbol.type != SYMBOL_TOKEN){
		return;
	}

	cMacro *this_cMacro = METHOD(str_cMacro_map, ctx->cMacros, get, this_symbol.value.str_val);
	if (this_cMacro == NULL){
		return;
	}

	//TODO: Add lookup to lispMacro table

	node args;
	if (this_list_c.childs.size != 3){
		args.type = NONTERMINAL_NODE;
		args.value.nonterminal_val.type = LIST_CONTENT_NT;
		args.childs = CONSTRUCT(node_vec);
	}
	else{
		args = this_list_c.childs.arr[2];
	}




	node processed = (*this_cMacro)(ctx, args);

	destruct_node_rec(*tree);
	*tree = processed;
}
