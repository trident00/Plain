#include "interp.h"
#include "typer.h"
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
	Compiler(Arena &arena, Compiler_Options *o) : arena(arena), ops(o), source_code(ops->source_file.contents) {
		std::cout << "Compiling " << CYAN << ops->source_file.file_name << RESET << " ...\n";
		if (ops->bench_flag) start = GetTickCount64();
		tokenize();
		parse();
		interpret();
		//emit();
	}

	Arena &arena;
	Compiler_Options *ops;
	std::string &source_code;

	volatile ULONGLONG start;
	std::vector<ULONGLONG> times;

#ifdef _DEBUG
	std::vector<Token *> tok_debug;
	std::vector<Ast *> ast_debug;
	std::vector<Symbol *> symbols;
#endif

	// Lexer
	Array<Token *> tokens = Array<Token *>(arena, false);
	// Parser
	Ast_Block *root = nullptr;
	int error_line = 0;


	void tokenize() {
		Lexer lexer = Lexer(source_code, arena);
		init_lexer(lexer);

		tokens.allocate(lexer.num_tokens);
		for (auto *t : lexer.tokens) {
			tokens.push(t);
		}
		lexer.tokens.clear();

		if (ops->bench_flag) times.push_back(GetTickCount64() - start);

#ifdef _DEBUG
		for (int i = 0; i < tokens.size; i++) {
			tok_debug.push_back(tokens[i]);
		}
#endif
	}

	void parse() {
		if (ops->bench_flag) start = GetTickCount64();

		Parser parser = Parser(tokens, ops->source_file, arena);
		root = init_parser(parser);
		error_line = parser.parser_error_line;

		if (ops->bench_flag) times.push_back(GetTickCount64() - start);

#ifdef _DEBUG
		for (int i = 0; i < root->nodes.size; i++) {
			ast_debug.push_back(root->nodes[i]);
		}
#else	
		//lexer.arena.free();
#endif

	}

	void interpret() {
		if (ops->bench_flag) start = GetTickCount64();

		Interp interp = Interp(arena, ops->source_file, root, error_line);
		init_interp(interp);

		if (ops->bench_flag) times.push_back(GetTickCount64() - start);
#ifdef _DEBUG
		for (int i = 0; i < interp.all_symbols.size; i++) {
			symbols.push_back(interp.all_symbols[i]);
		}
#endif
	}

	void emit() {
		Typer typer = Typer(arena, ops, root, times);
		init_typer(&typer);
	}

};

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
	std::string contents = oss.str();
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
	test_array();
	auto *ops = parse_compiler_options(argc, argv);

	Arena arena = Arena(MAIN_ARENA_SIZE);
	Compiler compiler(arena, ops);

	return 0;
}
