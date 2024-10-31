#include "typer.h"

inline Operand get_register(Register reg) { return Operand(reg); }

inline Operand create_address(int offset) { return Operand(RBP, offset); }

inline Operand create_constant(int v) { return Operand(v); }

void traverse_ast(Concatenator *c, Ast_Block *block)
{
	for (size_t i = 0; i < block->nodes.size; i++) {
		Ast *ast = block->nodes[i];
		switch (ast->type) {
			case AST_DECLARATION:
				auto *decl = (Ast_Declaration *)ast;
				
				break;

		}
	}
}

#include "Windows.h"
std::string get_current_executable_directory()
{
	char buffer[260];
	GetModuleFileNameA(NULL, buffer, sizeof(buffer));
	std::string full_path(buffer);
	return full_path;
}

void link(Typer *typer, std::string asm_file)
{
	ULONGLONG start = GetTickCount64();

	auto *ops = typer->ops;
	if (!ops->obj_flag) return;
	auto obj_file = remove_extension(asm_file) + ".obj";
	std::string bin_dir = get_current_executable_directory() + "\\..\\..\\..\\bin\\";

	std::string nasm = bin_dir + "nasm.exe -f win64 " + asm_file;
	std::system(nasm.c_str());

	typer->times.push_back(GetTickCount64() - start);
	start = GetTickCount64();

	if (!ops->link_flag) return;
	std::string link = bin_dir + "ld.exe " + obj_file + " -o " + remove_extension(asm_file)+".exe";

	std::system(link.c_str());


	if (ops->bench_flag) {
		typer->times.push_back(GetTickCount64() - start);
		std::cout << "  Lexer..." << typer->times[0]    << "ms\n";
		std::cout << "  Parser..." << typer->times[1]   << "ms\n";
		std::cout << "  Interp..." << typer->times[2]   << "ms\n";
		std::cout << "  " << nasm << "..." << typer->times[3] << "ms\n";
		std::cout << "  " << link << "..." << typer->times[4] << "ms\n";
		std::cout << '\n';
	}

	std::cout << "  ";

}

void init_typer(Typer *typer)
{
	std::string to_compile = typer->ops->source_file.file_name;
	link(typer, "C:\\FK\\test\\asm_test.asm");
}
