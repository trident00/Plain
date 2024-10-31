#include "interp.h"
#include "typer.h"
#include "compiler.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <ctime>
#endif

const size_t MAIN_ARENA_SIZE = 1024 * 1024;  // 1 MB arena size


struct Compiler
{
	Compiler(Arena &arena, Compiler_Options *o) : arena(arena), ops(o) {}

	Arena &arena;
	Compiler_Options *ops;
	Lexer *lexer = nullptr;
	Parser *parser = nullptr;
	Interp *interp = nullptr;
	Typer *typer = nullptr;
	Error_Manager *err_mgr = nullptr;

};

void init_compiler(Compiler *compiler, Compiler_Options *ops)
{
	std::cout << "Compiling " << ops->source_file.file_name << " ...\n";
	// Time

	ULONGLONG start;
	std::vector<ULONGLONG> times;

	Arena &arena = compiler->arena;

	Error_Manager *err = new Error_Manager(ops->source_file);
	
	std::string source_code = ops->source_file.contents;
	//
	//	Lexer
	//
	start = GetTickCount64();
	Lexer lexer = Lexer(source_code, arena);
	init_lexer(lexer);
	compiler->lexer = &lexer;

	Array<Token *> tokens = Array<Token *>(lexer.arena, lexer.num_tokens);
	for (auto *t : lexer.tokens) {
		tokens.push(t);
	}
	lexer.tokens.clear();

	if (ops->bench_flag) times.push_back(GetTickCount64() - start);
	

#ifdef _DEBUG
	std::vector<Token *> tok_debug;
	for (int i = 0; i < tokens.size; i++) {
		tok_debug.push_back(tokens[i]);
	}
#endif
	//
	//	Parser
	//
	if (ops->bench_flag) start = GetTickCount64();

	Parser parser = Parser(tokens, err, arena);
	auto *root = init_parser(parser);
	compiler->parser = &parser;

	if (ops->bench_flag) times.push_back(GetTickCount64() - start);


#ifndef _DEBUG
	lexer.arena.free();
#endif

#ifdef _DEBUG

	std::vector<Ast *> ast_debug;
	for (int i = 0; i < root->nodes.size; i++) {
		ast_debug.push_back(root->nodes[i]);
	}
#endif
	//
	//	Interp
	//
	if (ops->bench_flag) start = GetTickCount64();

	Interp interp = Interp(arena, root);
	init_interp(interp);
	compiler->interp = &interp;

	if (ops->bench_flag) times.push_back(GetTickCount64() - start);

#ifdef _DEBUG
	std::vector<Symbol *> symbols;
	for (int i = 0; i < interp.all_symbols.size; i++) {
		symbols.push_back(interp.all_symbols[i]);
	}
#endif
	//
	// Typer
	//
	Typer typer = Typer(arena, ops, root, times);
	init_typer(&typer);
	compiler->typer = &typer;


}

Compiler_Options *parse_compiler_options(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Missing input file argument." << '\n';
		exit(1);
	}
	
	if (!has_extension(argv[1], ".fk")) { std::cerr << "Invalid file extension: " << argv[1] << '\n'; exit(-1); }
	std::ifstream ifs(argv[1]);
	if (!ifs) { std::cerr << "No input file." << std::endl; exit(-1); }

	std::ostringstream oss;
	oss << ifs.rdbuf();
	const std::string contents = oss.str();
	Source_File s = Source_File(argv[1], contents);

	auto *options = new Compiler_Options(s);

	for (int i = 2; i < argc; i++) {
		auto it = flag_map.find(argv[i]);
		if (it != flag_map.end()) it->second(options);
		else std::cout << "> Unknown flag '" << argv[i] << "', ignoring.\n";
	}

	return options;
}

int main(int argc, char *argv[])
{
	std::cout << '\n';
	auto *ops = parse_compiler_options(argc, argv);

	Arena arena = Arena(MAIN_ARENA_SIZE);
	Compiler compiler(arena, ops);
	
	init_compiler(&compiler, ops);

	return 0;
}
