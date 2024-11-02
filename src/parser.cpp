#include "parser.h"
#include <unordered_map>

void Parser::report_error(std::string msg, Token *t, Token_Type panic)
{
	int line = t->line;
	int col = t->column;
	if (panic != (Token_Type)0) {
		col++;
	}

	// Spaces before text
	auto line_str = file.lines[line];
	::report_error(msg, line_str, line, col);

	if (panic != (Token_Type)0) {
		while (1) {
			Token *t = peek_token(this);
			if (t->type == panic) break;
			if (t->type == TOKEN_EOF) {
				std::cerr << "Compilation failed at " << CYAN << file.file_name << RESET << ":" << line << ", " << "exiting ...\n";
				exit(-1);
			}
			eat_token(this);
		}
	}
	parser_error_line = line;
}

Ast_Binary_Expression *make_binary(Parser *parser, Ast *left, Operator op, Ast *right)
{
	Ast_Binary_Expression *result = ast_alloc<Ast_Binary_Expression>(parser, right->line, right->col);
	result->left = left;
	result->op = op;
	result->right = right;
	result->expr_type = op.category;
	return result;
}

Ast_Declaration *make_decl(Parser *parser, Ast_Ident *ident, Ast_Ident *data_type, Ast_Binary_Expression *init=nullptr)
{
	Ast_Declaration *result = ast_alloc<Ast_Declaration>(parser, ident->line, ident->col);
	result->ident = ident;
	result->type = data_type;
	result->init = init;
	parser->current_block->num_of_decls++;
	return result;
}

Ast *parse_leaf(Parser *parser)
{
	if (peek_token(parser)->type == '(') {
		eat_token(parser);
		auto *result = parse_expression(parser);
		expect_character(parser, ')');
		return result;
	}

	Token* t = peek_token(parser);
	if (t->type == TOKEN_IDENT) {
		Ast_Ident *ident = ast_alloc<Ast_Ident>(parser, t->line, t->column);
		ident->string_value = t->string_value;
		eat_token(parser);
		return (Ast *)ident;

	} else if (t->type == TOKEN_NUMBER) {
		Ast_Literal *literal = ast_alloc<Ast_Literal>(parser, t->line, t->column);
		literal->literal_type = LITERAL_INT;
		literal->integer_value = t->integer_value;
		eat_token(parser);
		return (Ast *)literal;

	} else return nullptr;
}

Ast *parse_increasing_prec(Parser *parser, Ast *left, int min_prec=-10)
{
	Operator next = to_operator(peek_token(parser));
	// Returns the operand in an empty Ast_Binary_Expression at left if min_prec is higher. Which is why min_prec must default to be -INF.
	if (next.type == (Operator_Type)0 || next.arity != BINARY || next.prec <= min_prec) return left;
	eat_token(parser);

	// Else, recursive call with the last operator's precedence.
	Ast *right = parse_expression(parser, next.prec);
	return make_binary(parser, left, next, right);

}

Ast *parse_expression(Parser *parser, int min_prec)
{
	Ast *left = parse_leaf(parser);
	// Right leaning tree in order of increasing precedence.
	while (1) {

		Ast *node = parse_increasing_prec(parser, left, min_prec);
		if (node == left) break;
		left = node;
	}
	return left;
}

void parse_block(Parser *parser, Ast_Block *block, Token_Type until_type=TOKEN_EOF)
{
	while (peek_token(parser)->type != until_type) {
		Ast *ast = parse_statement(parser);
		if (!ast) break;
		block->nodes.push(ast);

		if (ast->type == AST_IF) {
			auto *ctrl = (Ast_If *)ast;
			ctrl->if_block = create_block(parser, block);

			parse_block(parser, ctrl->if_block, (Token_Type)'}');
		}
	}
	eat_token(parser);
}

Ast *parse_statement(Parser *parser)
{
	Token *symbol = peek_token(parser);

	if (symbol->type == TOKEN_IDENT) {
		if (peek_next_token(parser)->type == ':') {
			// Declaration
			Ast_Ident *ident = static_cast<Ast_Ident *>(parse_leaf(parser));

			eat_token(parser); // ':'
			Ast *data_type = (parse_leaf(parser));

			if (peek_token(parser)->type == '=') {
				// Assignment: "p : int = 2"; / "p := 2";
				eat_token(parser);

				auto *init = static_cast<Ast_Binary_Expression *>(parse_expression(parser));
				if (peek_token(parser)->type != ';') parser->report_error("Expected ';' after statement", last_token(parser), (Token_Type)';');
				else eat_token(parser);

				return make_decl(parser, ident, (Ast_Ident *)data_type, init);
			}

			if (peek_token(parser)->type == ';' && data_type) {
				// Decl only

				eat_token(parser);
				return make_decl(parser, ident, (Ast_Ident *)data_type);
			}
		}

		Ast *exp = parse_expression(parser);
		expect_character(parser, ';');
		return exp;

	}

	if (symbol->type == TOKEN_KEYWORD_IF) {
		eat_token(parser);
		Ast_If *result = ast_alloc<Ast_If>(parser, symbol->line, symbol->column);

		result->condition = (Ast_Binary_Expression *)parse_expression(parser);

		expect_character(parser, '{');
		return result;

	}

	if (symbol->type == (Token_Type)'{') {
		eat_token(parser);
		return parse_statement(parser);

	}

	if (symbol->type == (Token_Type)'}') {
		eat_token(parser);
	}

	return nullptr;
}

Ast_Block *init_parser(Parser &parser)
{
	parser.root =  new (parser.arena.allocate<Ast_Block>(sizeof(Ast_Block))) Ast_Block(parser.arena, SCOPE_GLOBAL);
	parser.current_block = parser.root;
	parse_block(&parser, parser.root);
	return parser.root;
}
