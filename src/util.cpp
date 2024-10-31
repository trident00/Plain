#include "util.h"

char *string_alloc(Arena &arena, char value[256])
{
	size_t len = strlen(value);
	char *str = (char *)arena.allocate<char>(len + 1);
	memcpy(str, value, len + 1);
	return str;
}

char *string_alloc(Arena &arena, char value)
{
	char *str = (char *)arena.allocate<char>(2);
	str[0] = value;
	str[1] = '\0';
	return str;
}

std::vector<std::string> parse_lines(std::string source)
{
	std::vector<std::string> result;
	result.push_back("");

	while (1) {
		size_t pos = source.find('\n');
		if (pos == std::string::npos || source.empty()) break;
		result.push_back(source.substr(0, pos));
		source.erase(0, pos + 1);
	}
	return result;
}