#pragma once

#include <cassert>
#include <cstring>
#include <string>
#include <deque>
#include "ast.h"
#include "compiler.h"
#include "lexer.h"


//-Parser-------------------------
//
//  Oct 21 2024
//	Use object pooling? Later. Get it up and running first, deal with your intense memory control obsession and optimizations later. ie. Profiling
//
//--------------------------------


template<typename T>
Ast_Block *ast_block_alloc(Arena &arena, Scope scope)
{
	return new (arena.allocate<T>(sizeof(T))) T(arena, scope);
}

inline bool is_ident(Token *t){ return (t->type >= TOKEN_IDENT && t->type <= TOKEN_LITERAL); }
inline bool is_ident_inclusive(Token *t){ return (t->type >= TOKEN_IDENT && t->type <= TOKEN_KEYWORD_FALSE); }
inline bool is_keyword(Token *t){ return t->type >= TOKEN_KEYWORD_IF; }
inline bool is_control_type(Token *t){ return t->type <= TOKEN_KEYWORD_WHILE; }
inline bool is_unary_keyword(Token *t){ return t->type == TOKEN_KEYWORD_RETURN; }

// @FIX
inline bool is_operator(Token* t) { return t->type <= TOKEN_MINUS_MINUS && t->type > TOKEN_ERROR; }


struct Parser
{
	Parser(const Array<Token *> &tokens, Source_File &file, Arena &arena) : tokens(tokens), file(file), arena(arena) {}

	const Array<Token *> &tokens;
	Source_File &file;
	Arena &arena;
	

	Ast_Block *root; // Root AST tree
	Ast_Block *current_block;

	int token_array_cursor = 0;
	int parser_error_line = 0;
#ifdef _DEBUG
	std::vector<Ast *> nodes;
	int ast_block_count = 1; // +global
	int ast_count = 0;
#endif

	void report_error(std::string msg, Token *t, Token_Type panic=(Token_Type)0);

};


//
// Token iterators
//

inline Token *peek_token(Parser *parser)
{
	Token *t = parser->tokens[parser->token_array_cursor];
	return t;
}

inline Token *peek_next_token(Parser *parser)
{
	assert(peek_token(parser)->type != TOKEN_EOF);
	Token *t = parser->tokens[parser->token_array_cursor + 1];
	return t;
}

// for error reporting only
inline Token *last_token(Parser *parser)
{
	assert(parser->token_array_cursor > 0);
	Token *t = parser->tokens[parser->token_array_cursor - 1];
	return t;
}

inline void eat_token(Parser *parser)
{
	parser->token_array_cursor++;
}

// Error reporting
inline
void expect_character(Parser *parser, char type)
{
	if (peek_token(parser)->type != (Token_Type)type) {
		std::string msg = "Expected '" + std::string(1, type) + '\'';
		parser->report_error(msg, last_token(parser), (Token_Type)type);

	} else eat_token(parser);
}

//
// Allocations
//
template<typename T>
T *ast_alloc(Parser *parser, int line, int col)
{
	T *ast = new (parser->arena.allocate<T>(sizeof(T))) T(line, col);
#ifdef _DEBUG
	parser->nodes.push_back(ast);
	parser->ast_count++;
#endif

	return ast;
}

inline Ast_Block *create_block(Parser *parser, Ast_Block *parent, Scope scope = SCOPE_BLOCK)
{
#ifdef _DEBUG
	parser->ast_block_count++;
#endif

	Ast_Block *result = new (parser->arena.allocate<Ast_Block>(sizeof(Ast_Block))) Ast_Block(parser->arena, scope);
	result->parent_block = parent;
	parser->current_block = result;
	return result;
}

//
// Forward decls
//
Ast_Block *init_parser(Parser &parser);

Ast *parse_expression(Parser* parser, int min_prec = -100);
Ast *parse_statement(Parser *parser);
