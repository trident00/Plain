#pragma once
#include <string>
#include <functional>
#include <unordered_map>

struct Source_File
{
	Source_File(char *file_name, const std::string &c) : file_name(file_name), contents(c) {
		
	}
	char *file_name;
	const std::string contents;
};

struct Compiler_Options
{
	Compiler_Options(Source_File source_file) : source_file(source_file) {}

	Source_File source_file;
	bool debug_flag = false;
	bool obj_flag = true;
	bool link_flag = true;
	bool bench_flag = false;
};

inline std::unordered_map<std::string, std::function<void(Compiler_Options*)>> flag_map = {
	{"-debug", [](Compiler_Options *opt) { opt->debug_flag = true; }},
	{"-obj", [](Compiler_Options *opt) { opt->obj_flag = true; }},
	{"-no-link", [](Compiler_Options *opt) { opt->link_flag = false; }},
	{"-bench", [](Compiler_Options *opt) { opt->bench_flag = true; }}
};

