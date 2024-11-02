#pragma once

#include <unordered_map>
#include <string>
#include <deque>
#include "compiler.h"
#include "util.h"

enum Error_Severity
{
	_WARNING,
	_ERROR,
};

enum Error_Reporter
{
	NONE = 0,
	LEXER,
	PARSER,
	INTERP,
	TYPER
};

enum Error_Type
{
	ERROR_DEFAULT,
	ERROR_,

};

struct Error
{
	Error_Severity severity = _ERROR;
	Error_Reporter reporter = NONE;
	Error_Type type = ERROR_DEFAULT;
	char *msg;
	int line;
	int col;
	int prec;

	Error(char *ms, int pr) : msg(ms), prec(pr) {}
};

struct Error_Manager
{
	Error_Manager(Source_File &file) : file(file) {}

	Source_File &file;
	Arena arena = Arena(1024*4);
	std::deque<Error> errors;

	void init();

};