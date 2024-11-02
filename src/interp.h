#pragma once

#include "compiler.h"
#include "ast.h"

enum Error
{
	EXIT,
	PASS,
	WARNING
};

struct Interp
{
	Interp(Arena &arena, Source_File &file, Ast_Block *root, int err_line) : arena(arena), file(file), root(root), parser_error_line(err_line) {}

	Arena &arena;
	Source_File &file;
	Ast_Block *root;

	bool error_reported;
	int parser_error_line;

	void report_error(std::string msg, Ast *ast, Error err=EXIT);


#ifdef _DEBUG
	Arena debug_arena = Arena(1024 * 1024);
	Array<Symbol *> all_symbols = Array<Symbol *>(debug_arena);
#endif
};

void init_interp(Interp &interp);
