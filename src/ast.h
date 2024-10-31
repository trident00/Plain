#pragma once


#include <string>
#include <unordered_map>
#include "util.h"

struct Token;

enum Operator_Category
{
	OP_GENERAL,
	OP_DECL,	    // Declaration (:, ::)
	OP_ARITH,	// Arithmetic (+, -, *, /, %)
	OP_COMPARE, // Comparison (<, >, ==, <=, !=, etc. in Token_Type)
	OP_ASSIGN,	// Assignment (=, +=, -=. etc. in Token_Type)
	OP_LOGIC,	// Logical (&&,||,!)
	OP_INC_DEC, // Inc/Decrement (++,--)
	//OP_TERNARY,	// Ternary ( ? : )
	OP_CAST,	// Type cast ( (int) )
	OP_ACCESS,	// Member access (. or ->)
	OP_COMMA,	// Comma (,)
	OP_INVOKE, 	// Function call ( ident() )
	OP_BRACKET  // Array subscript or grouping ( [], () )
};

enum Operator_Type
{
	//ASCII here

	OPERATOR_NEQ = 129,
	OPERATOR_STAR_EQ,
	OPERATOR_PLUS_EQ,
	OPERATOR_MINUS_EQ,
	OPERATOR_SLASH_EQ,
	OPERATOR_LESS_THAN_EQ,
	OPERATOR_EQ,
	OPERATOR_MORE_THAN_EQ,

	OPERATOR_PLUS_PLUS,
	OPERATOR_MINUS_MINUS,

	OPERATOR_SLASH_COMMENT,
	OPERATOR_BLOCK_COMMENT_START,
	OPERATOR_BLOCK_COMMENT_END,
	OPERATOR_ARROW,
	OPERATOR_DOUBLE_COLON,
	OPERATOR_DOUBLE_DOT,
	OPERATOR_DOUBLE_AND,
	OPERATOR_DOUBLE_OR,
};

enum Operator_Arity
{
	UNARY,
	BINARY,
	TERNARY
};

struct Operator
{
	Operator_Type		type = (Operator_Type)0;
	Operator_Category	category = OP_GENERAL;
	int					prec;
	Operator_Arity		arity;
};

Operator to_operator(Token* t);

struct Ast;
struct Ast_Block;
struct Ast_Binary_Expression;
struct Ast_Ident;
struct Ast_Literal;
struct Ast_Procedure_Definition;
struct Ast_Procedure_Call;
struct Ast_Type_Definition;
struct Ast_Type_Instantiation;
struct Ast_Comma_Seperated_Args;
struct Ast_For;
struct Ast_If;
struct Ast_Return;
struct Ast_Declaration;


enum Ast_Type
{
	AST_ERROR,
	AST_BLOCK,
	AST_IDENT,
	AST_BINARY_EXPRESSION,
	AST_LITERAL,
	AST_DECLARATION,
	AST_IF,
};

struct Ast
{
	Ast(Ast_Type t = AST_ERROR, int line=-1, int col=-1) : type(t), line(line), col(col) {
		line_string = 
	}
	Ast_Type type;
	int line;
	std::string &line_string;
	int col;

#ifdef _DEBUG
	bool interp_checked = false;
	virtual std::string ast_type() const { return typeid(*this).name(); }
#endif
};


struct Ast_Binary_Expression : Ast
{
	Ast_Binary_Expression(int line, int col) : Ast(AST_BINARY_EXPRESSION, line, col) {}

	Ast *left;
	Operator op;
	Ast *right;

	Operator_Category expr_type;
};

struct Ast_Ident : Ast
{
	Ast_Ident(int line, int col) : Ast(AST_IDENT, line, col) {}
	char *string_value;
};

enum Literal_Type
{
	LITERAL_ERROR,
	LITERAL_STRING,
	LITERAL_FLOAT,
	LITERAL_INT
};

struct Ast_Literal : Ast
{
	Ast_Literal(int line, int col) : Ast(AST_LITERAL, line, col) {}

	Literal_Type literal_type = LITERAL_ERROR;
	union {
		char *string_value;
		double float_value;
		int integer_value;
	};
};

enum Scope
{
	SCOPE_GLOBAL,
	SCOPE_BLOCK,
};

struct Symbol
{
	char *ident = nullptr;
	bool is_initialized = false;
	Ast_Block *block = nullptr; // Uses?
	// Data type later, just int for now.
};

struct Ast_Block : Ast
{
	Ast_Block(Arena &arena, Scope scope = SCOPE_BLOCK) : Ast(AST_BLOCK), scope(scope), nodes(arena), symbols(nullptr), parent_block(nullptr) {}

	Scope scope;
	MutableArray<Ast *> nodes;

	int num_of_decls = 0;
	Array<Symbol> *symbols; // In second pass. Interp my have its own symbol table Array with pointers to the Ast_Block?

	Ast_Block *parent_block;
};

struct Ast_Declaration : Ast
{
	Ast_Declaration(int line, int col) : Ast(AST_DECLARATION, line, col) {}
	Ast_Ident *ident;
	Ast_Ident *type;
	Ast_Binary_Expression *init;
};

struct Ast_If : Ast
{
	Ast_If(int line, int col) : Ast(AST_IF, line, col) {}

	Ast_Binary_Expression *condition;
	Ast_Block *if_block;
};
