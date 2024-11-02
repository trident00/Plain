#include "interp.h"
#include "parser.h"
#include "util.h"

void Interp::report_error(std::string msg, Ast *ast, Error err)
{
	error_reported = true;
	int line = ast->line;
	int col = ast->col;

	// Spaces before text
	auto line_str = file.lines[line];
	::report_error(msg, line_str, line, col);

	std::cerr << '\n';
	if (err == EXIT) {
		std::cerr << "Compilation failed at " << CYAN << file.file_name << RESET << ":" << line << ", " << "exiting ...\n";
		exit(-1);
	}
}

bool check_symbol(Interp *interp, Ast_Ident *ident, Ast_Block *block)
{
	char *key = ident->string_value;
	Ast_Block *current_block = block;

	while (current_block) {
		Foreach(current_block->symbols) {
			if (!strcmp(key, current_block->symbols[i].ident)) return true;
		}
		current_block = current_block->parent_block;
	}
	return false;
}

inline
Symbol &add_symbol(Interp *interp, Ast_Ident *ident, Ast_Block *block)
{
	if (check_symbol(interp, ident, block)) interp->report_error("Re-declaration of variable", ident);

	auto s = Symbol(ident->string_value, false, block);
	block->symbols.push(s);
	return block->symbols.peek();
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
				new_block->symbols.allocate(new_block->num_of_decls);
			}

			Foreach(new_block->nodes) {
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
			if (!check_symbol(interp, id, block)) {
				interp->report_error("Undeclared variable", ast);
			}
			return;
		}

		case AST_DECLARATION: {
			auto *decl = static_cast<Ast_Declaration *>(ast);
			auto *id = decl->ident;
			assert(strlen(id->string_value) != 0);

			auto &symbol = add_symbol(interp, id, block);

			if (decl->init) {
				check_node(interp, decl->init, block);
				symbol.is_initialized = true;
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
	if (interp.parser_error_line) {
		std::cerr << "Compilation failed at " << CYAN << interp.file.file_name << RESET << ":" << interp.parser_error_line << ", " << "exiting ...\n";
		exit(-1);
	}
	Ast_Block *root = interp.root;
	check_node(&interp, root, root);
}
