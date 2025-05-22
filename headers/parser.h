#include "lexer.h"

#define iserror(n) (n.type == TOKEN_NODE && n.value.token_val.type == ERROR_TOKEN || n.type == NONTERMINAL_NODE && n.value.nonterminal_val.type == ERROR_NT)
#define isunfinished(n) (n.type == NONTERMINAL_NODE && n.value.nonterminal_val.type == UNFINISHED_NT)

typedef enum nonterminal_type{
	PROGRAMM_NT,
	EXPR_NT,
	LIST_NT,
	LIST_CONTENT_NT,
	VALUE_NT,
	ERROR_NT,
	UNFINISHED_NT
}nonterminal_type;

typedef struct nonterminal{
	nonterminal_type type;
}nonterminal;

typedef enum node_type{
	TOKEN_NODE,
	NONTERMINAL_NODE
}node_type;

typedef union node_value{
	token token_val;
	nonterminal nonterminal_val;
}node_value;

typedef struct node{
	node_type type;
	node_value value;

	struct node_vec{
		struct node *arr;
		size_t cap, size;
	} childs;
}node;

int node_compare(node l, node r);

void node_destruct(node n);
void destruct_node_rec(node n);

#define NAME node_vec
#define TYPE node

#include "decl_vector.h"

node parseExprToAST(lexer l);
