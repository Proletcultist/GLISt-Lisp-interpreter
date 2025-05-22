#include <stdbool.h>
#include "parser.h"
#include "lexer.h"
#include "generalFunctions.h"

#define NAME node_vec
#define TYPE node

#include "def_vector.h"

static node tokenToNode(token t);
static node parseList(lexer l);
static node parseListContent(lexer l);
static node parseValue(lexer l);
node parseExprToAST(lexer l);

int node_compare(node l, node r){ return 0; }

static node tokenToNode(token t){
	return (node){TOKEN_NODE, {.token_val = t}, CONSTRUCT(node_vec)};
}

static node parseList(lexer l){
	node out;
	out.type = NONTERMINAL_NODE;
	out.value.nonterminal_val.type = LIST_NT;
	out.childs = CONSTRUCT(node_vec);

	token_type tt = METHOD(lexer, l, peekToken);

	if (tt == END_OF_INPUT){
		out.value.nonterminal_val.type = UNFINISHED_NT;
		return out;
	}
	else if (tt == OPEN_PARENTHESIS_TOKEN){
		METHOD(node_vec, out.childs, push, tokenToNode(METHOD(lexer, l, getToken, tt)));
		if (iserror(METHOD(node_vec, out.childs, peek))){
			out.value.nonterminal_val.type = ERROR_NT;
			return out;
		}

		if (METHOD(lexer, l, peekToken) == SEP_TOKEN){
			METHOD(node_vec, out.childs, push, tokenToNode(METHOD(lexer, l, getToken, SEP_TOKEN)));
			if (iserror(METHOD(node_vec, out.childs, peek))){
				out.value.nonterminal_val.type = ERROR_NT;
				return out;
			}
		}

		node next = parseListContent(l);
		METHOD(node_vec, out.childs, push, next);

		if (iserror(next)){
			out.value.nonterminal_val.type = ERROR_NT;
			return out;
		}
		else if (isunfinished(next) == UNFINISHED_NT){
			out.value.nonterminal_val.type = UNFINISHED_NT;
			return out;
		}
		
	}
	else{
		METHOD(node_vec, out.childs, push, tokenToNode(METHOD(lexer, l, getToken, tt)));
		out.childs.arr[out.childs.size - 1].value.token_val.type = ERROR_TOKEN;
		out.value.nonterminal_val.type = ERROR_NT;
		fprintf(stderr, "[Parsing] \033[31mError\033[0m Expected '('\n");

		return out;
	}

	if (METHOD(lexer, l, peekToken) == SEP_TOKEN){
		METHOD(node_vec, out.childs, push, tokenToNode(METHOD(lexer, l, getToken, SEP_TOKEN)));
		if (iserror(METHOD(node_vec, out.childs, peek))){
			out.value.nonterminal_val.type = ERROR_NT;
			return out;
		}
	}

	tt = METHOD(lexer, l, peekToken);
	if (tt == END_OF_INPUT){
		out.value.nonterminal_val.type = UNFINISHED_NT;
		return out;
	}
	else if (tt == CLOSE_PARENTHESIS_TOKEN){
		METHOD(node_vec, out.childs, push, tokenToNode(METHOD(lexer, l, getToken, tt)));
		if (iserror(METHOD(node_vec, out.childs, peek))){
			out.value.nonterminal_val.type = ERROR_NT;
			return out;
		}
	}
	else{
		METHOD(node_vec, out.childs, push, tokenToNode(METHOD(lexer, l, getToken, tt)));
		out.childs.arr[out.childs.size - 1].value.token_val.type = ERROR_TOKEN;
		out.value.nonterminal_val.type = ERROR_NT;
		fprintf(stderr, "[Parsing] \033[31mError\033[0m Expected ')'\n");

		return out;
	}

	return out;

}

static node parseListContent(lexer l){
	node out;
	out.type = NONTERMINAL_NODE;
	out.value.nonterminal_val.type = LIST_CONTENT_NT;
	out.childs = CONSTRUCT(node_vec);

	token_type tt;

	if ((tt = METHOD(lexer, l, peekToken)) == QUOTE_TOKEN || tt == OPEN_PARENTHESIS_TOKEN || isvaluetype(tt)){
		node next = parseExprToAST(l);
		METHOD(node_vec, out.childs, push, next);

		if (iserror(next)){
			out.value.nonterminal_val.type = ERROR_NT;
			return out;
		}
		else if (isunfinished(next)){
			out.value.nonterminal_val.type = UNFINISHED_NT;
			return out;
		}

		if (METHOD(lexer, l, peekToken) == SEP_TOKEN){
			METHOD(node_vec, out.childs, push, tokenToNode(METHOD(lexer, l, getToken, SEP_TOKEN)));
			if (iserror(METHOD(node_vec, out.childs, peek))){
				out.value.nonterminal_val.type = ERROR_NT;
				return out;
			}


			node next = parseListContent(l);
			METHOD(node_vec, out.childs, push, next);

			if (iserror(next)){
				out.value.nonterminal_val.type = ERROR_NT;
				return out;
			}
			else if (isunfinished(next)){
				out.value.nonterminal_val.type = UNFINISHED_NT;
				return out;
			}
		}
	}

	return out;

}

static node parseValue(lexer l){
	node out;
	out.type = NONTERMINAL_NODE;
	out.value.nonterminal_val.type = VALUE_NT;
	out.childs = CONSTRUCT(node_vec);

	token_type tt = METHOD(lexer, l, peekToken);

	if (tt == END_OF_INPUT){
		out.value.nonterminal_val.type = UNFINISHED_NT;
		return out;
	}
	else if (isvaluetype(tt)){
		METHOD(node_vec, out.childs, push, tokenToNode(METHOD(lexer, l, getToken, tt)));
		if (iserror(METHOD(node_vec, out.childs, peek))){
			out.value.nonterminal_val.type = ERROR_NT;
			return out;
		}
	}
	else{
		METHOD(node_vec, out.childs, push, tokenToNode(METHOD(lexer, l, getToken, tt)));
		out.childs.arr[out.childs.size - 1].value.token_val.type = ERROR_TOKEN;
		out.value.nonterminal_val.type = ERROR_NT;
		fprintf(stderr, "[Parsing] \033[31mError\033[0m Expected value\n");

		return out;
	}

	return out;
}

node parseExprToAST(lexer l){

	// Skip any leading seps
	while (METHOD(lexer, l, peekToken) == SEP_TOKEN){
		METHOD(lexer, l, getToken, SEP_TOKEN);
	}

	node out;
	out.type = NONTERMINAL_NODE;
	out.value.nonterminal_val.type = EXPR_NT;
	out.childs = CONSTRUCT(node_vec);

	token_type tt = METHOD(lexer, l, peekToken);

	if (tt == QUOTE_TOKEN){
		METHOD(node_vec, out.childs, push, tokenToNode(METHOD(lexer, l, getToken, QUOTE_TOKEN)));
		if (iserror(METHOD(node_vec, out.childs, peek))){
			out.value.nonterminal_val.type = ERROR_NT;
			return out;
		}
	}
	else if (tt == END_OF_INPUT){
		out.value.nonterminal_val.type = UNFINISHED_NT;
		return out;
	}
	else if (tt == ERROR_TOKEN){
		METHOD(node_vec, out.childs, push, tokenToNode(METHOD(lexer, l, getToken, tt)));
		out.value.nonterminal_val.type = ERROR_NT;
		fprintf(stderr, "[Parsing] \033[31mError\033[0m Expected quote, list or value\n");

		return out;
	}


	tt = METHOD(lexer, l, peekToken);
	if (tt == OPEN_PARENTHESIS_TOKEN){
		METHOD(node_vec, out.childs, push, parseList(l));
	}
	else if (tt == ERROR_TOKEN){
		METHOD(node_vec, out.childs, push, tokenToNode(METHOD(lexer, l, getToken, tt)));
		out.value.nonterminal_val.type = ERROR_NT;
		fprintf(stderr, "[Parsing] \033[31mError\033[0m Expected list or value\n");

		return out;
	}
	else{
		METHOD(node_vec, out.childs, push, parseValue(l));
	}


	node last = METHOD(node_vec, out.childs, peek);
	if (iserror(last)){
		out.value.nonterminal_val.type = ERROR_NT;
	}
	else if (isunfinished(last)){
		out.value.nonterminal_val.type = UNFINISHED_NT;
	}

	return out;
}

void node_destruct(node n){
	if (n.type == TOKEN_NODE && (n.value.token_val.type == STR_TOKEN || n.value.token_val.type == SYMBOL_TOKEN)){
		free(n.value.token_val.value.str_val);
	}

	DESTRUCT(node_vec, n.childs);
}

void destruct_node_rec(node n){
	for (size_t i = 0; i < n.childs.size; i++){
		destruct_node_rec(n.childs.arr[i]);
	}

	DESTRUCT(node, n);
}
