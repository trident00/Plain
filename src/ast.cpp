#include "ast.h"
#include "lexer.h"


std::unordered_map<Token_Type, Operator> operator_table = {
	// used as delimiters
	{(Token_Type)';',		{(Operator_Type)';', OP_GENERAL, -3, UNARY}},
	{(Token_Type)')',		{(Operator_Type)')', OP_BRACKET, -3, UNARY}},

	// Assignment
	{TOKEN_STAR_EQ,	 		{OPERATOR_STAR_EQ, OP_ASSIGN, -2, BINARY}},
	{TOKEN_PLUS_EQ,	 		{OPERATOR_PLUS_EQ, OP_ASSIGN, -2, BINARY}},
	{TOKEN_MINUS_EQ,	 	{OPERATOR_MINUS_EQ, OP_ASSIGN, -2, BINARY}},
	{TOKEN_SLASH_EQ,	 	{OPERATOR_SLASH_EQ, OP_ASSIGN, -2, BINARY}},
	{(Token_Type)'=',		{(Operator_Type)'=', OP_ASSIGN, -2, BINARY}},

	// Function call: 			Left: name, Op. ')' Right: args.
	// Parens in Expression:	Operators not created.

	{(Token_Type)'(',		{(Operator_Type)'(', OP_BRACKET, -1, BINARY}},
	{(Token_Type)',',		{(Operator_Type)',', OP_GENERAL, 0, BINARY}},

	// Comparison
	{(Token_Type)'<',		{(Operator_Type)'<', 	OP_COMPARE, 1, BINARY}},
	{(Token_Type)'>',		{(Operator_Type)'>', 	OP_COMPARE, 1, BINARY}},
	{TOKEN_NEQ,				{OPERATOR_NEQ,			OP_COMPARE, 1, BINARY}},
	{TOKEN_LESS_THAN_EQ, 	{OPERATOR_LESS_THAN_EQ, OP_COMPARE, 1, BINARY}},
	{TOKEN_EQ, 		  		{OPERATOR_EQ,			OP_COMPARE, 1, BINARY}},
	{TOKEN_MORE_THAN_EQ,	{OPERATOR_MORE_THAN_EQ, OP_COMPARE, 1, BINARY}},


	// Arith
	{(Token_Type)'+',		{(Operator_Type)'+', 	OP_ARITH, 2, BINARY}},
	{(Token_Type)'-',		{(Operator_Type)'-', 	OP_ARITH, 2, BINARY}},
	{(Token_Type)'/',		{(Operator_Type)'/', 	OP_ARITH, 3, BINARY}},
	{(Token_Type)'*',		{(Operator_Type)'*', 	OP_ARITH, 3, BINARY}},


	{(Token_Type)':',		{(Operator_Type)':', 	OP_DECL, 4, BINARY}},
};

Operator to_operator(Token* t)
{
	auto it = operator_table.find(t->type);
	if (it == operator_table.end()) return Operator();
	return it->second;
}
