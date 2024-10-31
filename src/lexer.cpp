#include <iostream>
#include <string>
#include <cstring>
#include <cassert>
#include "lexer.h"

// TODO
// BLock comments or slash  comments, make a skip_line() function? @SPEED
//

void eat_character(Lexer *lexer)
{
	char c = lexer->input[lexer->input_cursor];
	if (c == '\n') {
		lexer->lines.push_back(lexer->line_buffer);
		lexer->line_buffer.clear();

		lexer->current_line_number++;
		lexer->current_character_index = 0;

	} else if (c > 32) lexer->line_buffer.push_back(lexer->input[lexer->input_cursor]);
	lexer->input_cursor++;
	lexer->current_character_index++;
}

int peek_next_character(Lexer *lexer)
{
	if (lexer->input_cursor > lexer->input.length()) return -1;

	int next_char = lexer->input[lexer->input_cursor];
	return next_char;
}

bool starts_word(int c)
{
	if (isalpha(c)) return true;
	if (c == '_') return true;
	return false;
}

bool continues_word(int c)
{
	if (isalnum(c) || c == '_') return true;
	return false;
}

char *parse_word(Lexer *lexer, Token *result)
{
	char *buffer = lexer->string_buffer;
	auto pos = buffer;

	while (1) {
		int c = peek_next_character(lexer);
		if (!continues_word(c)) {
			break;
		}
		*pos++ = c;
		eat_character(lexer);
	}
	result->word_length = (pos - buffer);
	buffer[result->word_length] = '\0';
	return buffer;
}

Token_Type check_for_keyword(Token *token)
{
	auto *s = token->string_value;
	switch (token->word_length) {
		case 2:
			if (!strcmp(s, "if"))			return TOKEN_KEYWORD_IF;
			break;
		case 3:
			if (!strcmp(s, "for"))			return TOKEN_KEYWORD_FOR;
			break;
		case 4:
			if (!strcmp(s, "else"))			return TOKEN_KEYWORD_ELSE;
			if (!strcmp(s, "true"))			return TOKEN_KEYWORD_TRUE;
			break;
		case 5:
			if (!strcmp(s, "while"))		return TOKEN_KEYWORD_WHILE;
			if (!strcmp(s, "false"))		return TOKEN_KEYWORD_FALSE;
			break;
		case 6:
			if (!strcmp(s, "return"))		return TOKEN_KEYWORD_RETURN;
			break;
	}
	return (Token_Type)0;
}

Token *make_one_character_token(Lexer *lexer, char type = 0)
{
	Token *result = token_alloc(lexer);
	result->type = (Token_Type)type;
	result->string_value = string_alloc(lexer->string_arena, type);
	return result;
}


Token *make_ident_or_keyword(Lexer *lexer)
{
	Token *result = token_alloc(lexer);
	result->type = TOKEN_IDENT;

	auto *word = parse_word(lexer, result);
	result->string_value = string_alloc(lexer->string_arena, word);

	Token_Type keyword_type = check_for_keyword(result);
	if (keyword_type) result->type = keyword_type;

	return result;
}

Token *make_literal_string_token(Lexer *lexer, Token_Type type=TOKEN_LITERAL)
{
	Token *result = make_ident_or_keyword(lexer);
	result->type = type;
	return result;
}

Token *make_number(Lexer *lexer)
{
	Token *result = token_alloc(lexer);
	result->type = TOKEN_NUMBER;

	char digital_buffer[256];
	auto *pos = digital_buffer;

	bool after_decimal_cursor = false;
	while (1) {
		int c = peek_next_character(lexer);
		if (after_decimal_cursor) {
			if (isdigit(c)) {
				*pos++ = c;

			} else if (c == '.') {
				lexer->input_cursor--;
				result->integer_value = std::atoll(digital_buffer);
				return result;
			} else {
				*pos = '\0';
				result->type = TOKEN_NUMBER_FLOAT;
				result->double_value = std::atof(digital_buffer);
				break;
			}
		} else {
			if (isdigit(c)) {
				*pos++ = c;
			} else if (c == '.') {
				after_decimal_cursor = true;
				*pos++ = c;
			} else {
				*pos = '\0';
				result->integer_value = std::atoll(digital_buffer);
				break;
			}
		}
		eat_character(lexer);
	}
	return result;
}

bool character_lookahead(Lexer *lexer, Token *token, char c, Token_Type special_type)
{
	if (peek_next_character(lexer) == c) {
		token->type = special_type;
		token->string_value[1] = c;
		eat_character(lexer);
		return true;
	}
	return false;
}

bool character_lookahead(Lexer *lexer, char c)
{
	if (peek_next_character(lexer) == c) {
		eat_character(lexer);
		return true;
	}
	return false;
}

bool skip_comment(Lexer *lexer)
{
	if (peek_next_character(lexer) != '/') return false;
	eat_character(lexer);
	if (character_lookahead(lexer, '/')) {
		while (peek_next_character(lexer) != '\n') eat_character(lexer);
		eat_character(lexer);
	} else if (character_lookahead(lexer, '*')) {
		while (1) {
			eat_character(lexer);
			if (peek_next_character(lexer) == '*') {
				eat_character(lexer);
				if (peek_next_character(lexer) == '/'){eat_character(lexer); break;}
			}
		}
	} else return false;
	return true;
}

Token *compose_new_token(Lexer *lexer)
{
	int c = peek_next_character(lexer);
	while (c == ' ' || c == '\n') {
		eat_character(lexer);
		c = peek_next_character(lexer);
	}

	if (starts_word(c)) {
		return make_ident_or_keyword(lexer);
	}

	if (isdigit(c)) {
		return make_number(lexer);
	}

	Token *result = NULL;
	switch(c) {
		case '\0':		return make_one_character_token(lexer, -1);
		case '(':
		case ')':
		case '{':
		case '}':
		case '[':
		case ']':
		case ';':
		case ':':
		case '&':
		case '|':
		case '#':
		case ',':
		case '.':
		default:
			result = make_one_character_token(lexer, c);
			eat_character(lexer);
			break;
		case '"':
			eat_character(lexer);
			result = make_literal_string_token(lexer);
			assert(peek_next_character(lexer) == '"'); // @ERROR
			eat_character(lexer);
			break;
		case '=':
		case '+':
		case '-':
		case '*':
		case '/':
			if (skip_comment(lexer)) {
				return compose_new_token(lexer);
			}
		case '!':
		case '<':
		case '>':
			result = make_one_character_token(lexer, c);
			eat_character(lexer);
			character_lookahead(lexer, result, '=', (Token_Type)(c+120));
			break;
	}
	// Special operators
	switch ((int)result->type) {

		case '+':	character_lookahead(lexer, result, '+', TOKEN_PLUS_PLUS); break;
		case '-':	if(!character_lookahead(lexer, result, '-', TOKEN_MINUS_MINUS))
						character_lookahead(lexer, result, '>', TOKEN_ARROW);
					break;
		case ':':	character_lookahead(lexer, result, ':', TOKEN_DOUBLE_COLON); break;
		case '.':	character_lookahead(lexer, result, '.', TOKEN_DOUBLE_DOT); break;
		case '&':	character_lookahead(lexer, result, '&', TOKEN_DOUBLE_AND); break;
		case '|':	character_lookahead(lexer, result, '|', TOKEN_DOUBLE_OR); break;
		default: break;
	}

	return result;
}