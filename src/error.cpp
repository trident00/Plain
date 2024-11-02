#include <string>
#include <deque>
#include <sstream>
#include "util.h"


void Error_Manager::init()
{

}

void report_error(std::string msg, Error_Type type)
{
	
}

//std::string err_msg_line(Lexer *lexer, Error err)
//{
//	std::ostringstream msg_line;
//	std::string ind = "     ";
//	msg_line << ind << err.line << "\t|" << ind << get_line(lexer, err.line) << '\n';
//
//	std::string und = std::string(err.col - 1, ' ');
//	msg_line << "\t|" << ind << und << '^' << std::endl;
//	err.msg_desc += " at Line " + std::to_string(err.line) + ", Column " + std::to_string(err.col) + ":\n";
//	return msg_line.str();
//}