#pragma once

#include "ast.h"



struct Interp
{
	Interp(Arena &arena, Ast_Block *root) : arena(arena), root(root) {}

	Arena &arena;

#ifdef _DEBUG
	Arena debug_arena = Arena(1024 * 1024);
	MutableArray<Symbol *> all_symbols = MutableArray<Symbol *>(debug_arena);
#endif

	Ast_Block *root;
	bool error_reported = false;

};

void init_interp(Interp &interp);
