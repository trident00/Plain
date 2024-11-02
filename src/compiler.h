#pragma once
#include <string>
#include <iostream>
#include <functional>
#include <unordered_map>
#include "util.h"

inline
std::string get_file_name(char *file_dir)
{
	std::string dir = file_dir;
	return dir.substr(dir.find_last_of('\\')+1, dir.length());
}

inline
void report_error(std::string &msg, std::string &line_str, int line, int col)
{

	// Spaces before text
	size_t pos = line_str.find_first_not_of(" \t");
	col -= pos + 1;
	line_str = line_str.substr(pos, line_str.length());

	auto prefix = std::string("  " + std::to_string(line) + " | ");
	auto empty_prefix = std::string(prefix.length() - 3, ' ') + " | ";
	auto carot = empty_prefix + std::string(col, ' ');
	//auto line_prefix = std::string((empty_prefix + std::string(col - 1, '~') + '^').length(), ' ');


	std::cerr << RED << "error: " << RESET << msg << '\n';
	std::cerr << prefix;
	for (int i = 0; i < line_str.size(); i++) {
		if (i == col) {
			std::cerr << RED << line_str[i++] << RESET;
		}
		std::cerr << line_str[i];
	}
	std::cerr << '\n';
	std::cerr << carot << BLUE << '^' << RESET << '\n';

}

struct Source_File
{
	Source_File(char *file_dir, std::string &c) : file_dir(file_dir), contents(c) {
		file_name = get_file_name(file_dir);
		lines = parse_lines(c);
	}
	char *file_dir;
	std::string file_name;
	std::string contents;
	std::vector<std::string> lines;
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
	{"-debug",		[](Compiler_Options *opt) { opt->debug_flag = true; }},
	{"-obj",		[](Compiler_Options *opt) { opt->obj_flag = true; }},
	{"-no-link",	[](Compiler_Options *opt) { opt->link_flag = false; }},
	{"-bench",		[](Compiler_Options *opt) { opt->bench_flag = true; }}
};

