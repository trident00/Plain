#include "parser.h"
#include "compiler.h"
#include <sstream>
#include "Windows.h"

#define s32 int;
#define u32 unsigned int;

const size_t STACK_BUFFER_SIZE = 8;

// Process the Ast_Nodes and send instructions. Interprets the symbol tables created by Interp.
struct Typer
{
	Typer(Arena &arena, Compiler_Options *op, Ast_Block *root, std::vector<ULONGLONG> times) : arena(arena), ops(op), root(root), times(times) {}

	Arena &arena;
	Compiler_Options *ops;
	Ast_Block *root;
	std::vector<ULONGLONG> times;
};

enum Operand_Type { REGISTER, MEMORY, IMMEDIATE };

enum Register
{
	RSP,	// Stack pointer
	RBP,	// Base pointer
};

struct Operand
{
	Operand_Type type;
	union {
		Register reg;
		int immediate;
	};
	int offset = 0;
	// Register
	Operand(Register r) : type(REGISTER), reg(r) {}
	// Memory
	Operand(Register base, int off) : type(MEMORY), reg(base), offset(off) {}
	// Immediate
	Operand(int imm) : type(IMMEDIATE), immediate(imm) {}
};

enum Instruction_Type
{
	MOV,
};

struct Instruction
{
	Instruction_Type type;
	Operand src;
	Operand dest;
};

struct Concatenator
{
	Arena arena = Arena(1024 * 1024);
	Instruction buffer[STACK_BUFFER_SIZE];
	size_t buffer_cursor = 0;

	inline
	void add(const Instruction& i)
	{
		buffer[++buffer_cursor % STACK_BUFFER_SIZE] = i;
	}
};

void init_typer(Typer* typer);
