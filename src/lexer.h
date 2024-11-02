#pragma once

#include <iostream>
#include <string>
#include "util.h"

#define For(container) for (auto& item : container)

const unsigned MAX_STRING_LENGTH = 256;
const size_t STRING_ARENA_SIZE = 16 * 1024;  // 16 KB arena size

enum Token_Type
{
	TOKEN_EOF = -1,
	TOKEN_ERROR = 0,
	// ACSII chars here (1-128)

	// Double char EQ things, first char + 120.
	TOKEN_NEQ 			= 153,
	TOKEN_STAR_EQ		= 162,
	TOKEN_PLUS_EQ		= 163,
	TOKEN_MINUS_EQ		= 165,
	TOKEN_SLASH_EQ		= 167,
	TOKEN_LESS_THAN_EQ 	= 180,
	TOKEN_EQ 			= 181,
	TOKEN_MORE_THAN_EQ 	= 182,

	// other operators
	TOKEN_PLUS_PLUS = 256,
	TOKEN_MINUS_MINUS,

	TOKEN_SLASH_COMMENT,
	TOKEN_BLOCK_COMMENT_START,
	TOKEN_BLOCK_COMMENT_END,
	TOKEN_ARROW,
	TOKEN_DOUBLE_COLON,
	TOKEN_DOUBLE_DOT,
	TOKEN_DOUBLE_AND,
	TOKEN_DOUBLE_OR,

	// IDENTIFIERS
	TOKEN_IDENT,
	TOKEN_NUMBER,
	TOKEN_NUMBER_FLOAT,
	TOKEN_LITERAL,

	// KEYWORDS
	// Identifier-like
	TOKEN_KEYWORD_TRUE,
	TOKEN_KEYWORD_FALSE,
	// Control
	TOKEN_KEYWORD_IF = 320,
	TOKEN_KEYWORD_FOR,
	TOKEN_KEYWORD_ELSE,
	TOKEN_KEYWORD_WHILE,

	// Unary keywords
	TOKEN_KEYWORD_RETURN,

};

struct Token
{
	Token_Type type = TOKEN_ERROR;

	int line = 0;
	int column = 0;

	union {
		char *string_value;
		long long integer_value;
		double double_value;
	};

	int word_length;
};



struct Lexer
{
	Lexer(std::string &input, Arena &perm_arena) : input(input), arena(perm_arena) {}

	std::vector<Token *> tokens;
	std::vector<std::string> lines;
	std::string line_buffer;

	std::string &input;
	int current_line_number = 1;
	int current_character_index = 1;


	Arena &arena;

	char string_buffer[256];
	int num_tokens = 0;

	int input_cursor = 0;

	bool error_reported = false;
};

Token *compose_new_token(Lexer *lexer);

inline void init_lexer(Lexer &lexer)
{
	lexer.lines.push_back(""); // Line 0
	while (1) {
		Token *t = compose_new_token(&lexer);
		if (t->type == TOKEN_EOF) break;
		if (t->type == TOKEN_ERROR) { std::cerr << "Token_Error in Lexer\n"; exit(1); }
	}

	if (!lexer.line_buffer.empty()) lexer.lines.push_back(lexer.line_buffer);
	lexer.line_buffer.clear();
}

inline Token *token_alloc(Lexer *lexer)
{
	Token *t = arena_alloc<Token>(lexer->arena);
	lexer->tokens.push_back(t);
	t->line = lexer->current_line_number;
	t->column = lexer->current_character_index;
	lexer->num_tokens++;

	return t;
}
