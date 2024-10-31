#include "interp.h"
#include "error.h"
#include "parser.h"
#include "util.h"

/*
	@TODO
	Optimizations
---------------------
	Ast_Binary_Expression:
		Calculate if both left and right are Literals.

*/

void print_line(int line, int col)
{

}



void check_symbol(Interp *interp, Ast_Ident *ident, Ast_Block *block, bool add=false, bool init=false)
{
	if (!block->symbols) return;
	char *key = ident->string_value;

	Ast_Block *current_block = block;
	while (1) {
		for (int i = 0; i < current_block->symbols->size; i++) {
			if (strcmp(key, (*current_block->symbols)[i].ident) == 0) {
				if (add) {
					std::cerr << "Re-declaration of " << ident->string_value << '\n'; exit(-1);
				}

				return;
			}

		}

		if (current_block->parent_block) {
			current_block = current_block->parent_block;

		} else break;
	}

	if (add) {
		Symbol s = {key, init, block};
		(*block->symbols).push(s);
#ifdef _DEBUG
		interp->all_symbols.push(&block->symbols->peek());
#endif
		return;
	}

	std::cerr<<"Not in symbol table\n";
	exit(-1);
}

void check_node(Interp *interp, Ast *ast, Ast_Block *block)
{
#ifdef _DEBUG
	ast->interp_checked = true;
#endif
	switch (ast->type) {
		case AST_BLOCK: {
			Ast_Block *new_block = static_cast<Ast_Block *>(ast);
			if (new_block->num_of_decls > 0) {
				new_block->symbols = new_array<Symbol>(interp->arena, new_block->num_of_decls);
			}

			for (int i = 0; i < new_block->nodes.size; i++) {
				check_node(interp, new_block->nodes[i], new_block);
			}
			return;
		}

		case AST_BINARY_EXPRESSION: {
			auto *node = static_cast<Ast_Binary_Expression *>(ast);
			check_node(interp, node->left, block);
			check_node(interp, node->right, block);
			return;
		}

		case AST_IDENT: {
			auto *id = static_cast<Ast_Ident *>(ast);

			assert(strlen(id->string_value) != 0);
			check_symbol(interp, id, block);
			return;
		}

		case AST_DECLARATION: {
			auto *decl = static_cast<Ast_Declaration *>(ast);
			auto *id = decl->ident;
			assert(strlen(id->string_value) != 0);
			check_symbol(interp, id, block, true, decl->init);
			if (decl->init) {
				check_node(interp, decl->init, block);
			}
			return;
		}

		case AST_IF: {
			auto *node = static_cast<Ast_If *>(ast);
			auto *cond = static_cast<Ast_Binary_Expression *>(node->condition);
			if (!(cond->op.category == OP_LOGIC || cond->op.category == OP_COMPARE)) exit(-1);
			check_node(interp, node->condition, block);

			check_node(interp, node->if_block, block);
			return;
		}

		default: return;
	}
}

void init_interp(Interp &interp)
{
	Ast_Block *root = interp.root;

	root->symbols = new_array<Symbol>(interp.arena, root->num_of_decls);
	for (int i = 0; i < root->nodes.size; i++) {
		check_node(&interp, root->nodes[i], root);
	}
}
