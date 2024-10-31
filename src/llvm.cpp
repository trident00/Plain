#include "typer.h"
using _Operator = Operator;

#include <filesystem>
#include <map>
#include <cstdlib>
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

namespace LLVM_Emit {

#define u32 unsigned int

	using namespace llvm;

	struct LLVM_Emitter
	{
		LLVMContext c;
		Module *m = new Module("MainModule", c);
		Function *f = nullptr;
		std::map<std::string, Value *> llvm_symbols;
		int block_count = 0;
		IRBuilder<> builder;

		LLVM_Emitter() : builder(c) {}
		~LLVM_Emitter() { delete m; }

		// Gives the builder the entry point to put in instructions
		inline
		void create_block() {
			BasicBlock *entry = BasicBlock::Create(c, f->getName() + Twine(block_count++), f);
			builder.SetInsertPoint(entry);
		}

		inline
		void create_function(const char *name) {
			f = Function::Create(FunctionType::get(Type::getInt32Ty(c), false), Function::ExternalLinkage, name, m);
		}

		inline
		BasicBlock *peek_stack() {
			return builder.GetInsertBlock();
		}

		inline
		void pop_stack() {
			builder.SetInsertPoint(this->peek_stack());
		}

		inline
		Value *var_alloc(int val, const Twine ident = "") {
			Value *ptr = builder.CreateAlloca(Type::getInt32Ty(c), nullptr, ident);
			builder.CreateStore(builder.getInt32(val), ptr);
			//return ptr;
			return builder.CreateLoad(Type::getInt32Ty(c), ptr);
		}

		inline
		Value *var_alloc(Value *val, const Twine ident = "") {
			Value *ptr = builder.CreateAlloca(Type::getInt32Ty(c), nullptr, ident);
			builder.CreateStore(val, ptr);
			//return ptr;
			return builder.CreateLoad(Type::getInt32Ty(c), ptr);
		}

	};

	Value *binary_op(LLVM_Emitter *llvm, Value *left, Operator_Type op, Value *right)
	{
		// '=' OEPRATOR CAUSING THE BREAKPOINT
		Value *result = nullptr;
		switch (op) {
		case (Operator_Type)'=': result = llvm->builder.CreateStore(left, right); break;
		case (Operator_Type)'+': result = llvm->builder.CreateAdd(left, right); break;
		case (Operator_Type)'-': result = llvm->builder.CreateSub(left, right); break;
		case (Operator_Type)'*': result = llvm->builder.CreateMul(left, right); break;
		case (Operator_Type)'/': result = llvm->builder.CreateSDiv(left, right); break;

		case (Operator_Type)'<': result = llvm->builder.CreateICmpSLT(left, right); break;

		default: break;
		}
		return result;
	}

	Value *ast_literal_alloc(LLVM_Emitter *llvm, Ast_Literal *node)
	{
		if (node->literal_type != LITERAL_INT) {
			exit(-1);
		}

		return llvm->builder.getInt32(node->integer_value);
	}

	Value *recurse_expression(LLVM_Emitter *llvm, Ast *ast)
	{
		switch (ast->type) {

		case AST_LITERAL: {
			return ast_literal_alloc(llvm, static_cast<Ast_Literal *>(ast));
		}

		case AST_BINARY_EXPRESSION: {
			auto *node = static_cast<Ast_Binary_Expression *>(ast);

			Value *left = recurse_expression(llvm, node->left);
			Value *right = recurse_expression(llvm, node->right);

			return binary_op(llvm, left, node->op.type, right);
		}

		case AST_IDENT: {
			auto *node = static_cast<Ast_Ident *>(ast);
			Value *ptr = llvm->llvm_symbols.find(node->string_value)->second;
			return llvm->builder.CreateLoad(Type::getInt32Ty(llvm->c), ptr);
		}

		default: assert(0);
		}
	}

	void emit_llvm(LLVM_Emitter *llvm, Ast_Block *block)
	{
		auto *casted = static_cast<Ast_Block *>(block);
		Array <Symbol> &interp_table = *(casted->symbols);

		llvm->create_block();
		if (casted->symbols) {
			for (int i = 0; i < interp_table.size; i++) {
				Value *ptr = llvm->builder.CreateAlloca(Type::getInt32Ty(llvm->c), nullptr, interp_table[i].ident);
				llvm->llvm_symbols[interp_table[i].ident] = ptr;
			}
		}

		for (int i = 0; i < casted->nodes.size; i++) {
			Ast *ast = casted->nodes[i];

			switch (ast->type) {

			case AST_BLOCK: {
				emit_llvm(llvm, static_cast<Ast_Block *>(ast));
				break;
			}

			case AST_DECLARATION: {
				auto *node = static_cast<Ast_Declaration *>(ast);
				if (!node->init) break;
				Value *init = recurse_expression(llvm, node->init);

				Value *ptr = llvm->llvm_symbols.find(node->ident->string_value)->second;
				llvm->builder.CreateStore(init, ptr);
				break;
			}

			case AST_LITERAL:
			case AST_BINARY_EXPRESSION: {
				Value *result = recurse_expression(llvm, ast);
				llvm->var_alloc(result);
				break;
			}

			case AST_IF: {
				auto *node = static_cast<Ast_If *>(ast);
				Value *condition = recurse_expression(llvm, node->condition);
				llvm->var_alloc(condition);
				// New block
				emit_llvm(llvm, static_cast<Ast_Block *>(node->if_block));
				break;
			}

			default: assert(0);
			}

		}
		llvm->builder.CreateRet(llvm->builder.getInt32(0));
	}
}

#include <Windows.h>

std::string get_current_executable_directory()
{
	char buffer[260];
	GetModuleFileNameA(NULL, buffer, sizeof(buffer));
	std::string full_path(buffer);
	return full_path;
}

void output_llvm(LLVM_Emit::LLVM_Emitter *llvm, const std::string &source_file, Compiler_Options *ops)
{
	// To LLVM IR .ll
	std::error_code IR_error;

	std::string bin_path = get_current_executable_directory() + "\\..\\..\\..\\bin\\";

	std::string ll_file = source_file + "ll";

	llvm::raw_fd_ostream output(ll_file.c_str(), IR_error);
	if (IR_error) {
		llvm::errs() << "Could not open file: " << IR_error.message() << "\n";
		return;
	}
	llvm->m->print(output, nullptr);
	output.flush();

	if (!ops->obj_flag) return;

	std::string to_obj = bin_path + "llc.exe -filetype=obj " + ll_file;
	llvm::outs() << to_obj << "\n";
	std::system(to_obj.c_str());

	if (!ops->link_flag) return;

	std::string link = bin_path + "lld-link.exe -entry:main " + source_file + "obj";
	llvm::outs() << link << "\n";
	std::system(link.c_str());

#ifndef _DEBUG
	std::remove((source_file + "obj").c_str());
	std::remove(ll_file.c_str());
#endif

	if (!ops->debug_flag) return;

	std::string objdump = bin_path + "llvm-objdump.exe -d " + source_file + "exe";
	llvm::outs() << objdump << "\n";
	std::system(objdump.c_str());
}


void init_typer(Typer *typer)
{
	std::string source_file = std::string(typer->ops->source_file.file_name);
	if (source_file.back() != 'k') exit(-1);
	source_file.pop_back();
	if (source_file.back() != 'f') exit(-1);
	source_file.pop_back();

	auto *emitter = new LLVM_Emit::LLVM_Emitter();
	emitter->create_function("main");
	LLVM_Emit::emit_llvm(emitter, typer->root);
#ifdef _DEBUG
	emitter->m->print(LLVM_Emit::outs(), nullptr);
#endif

	output_llvm(emitter, source_file, typer->ops);
}
