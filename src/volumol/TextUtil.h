#pragma once
#include <string>
#include <vector>
#include "../logic/Types.h"

namespace mol {
	bool isWhiteSpace(char c);

	bool isText(char c);

	bool isDigit(char c);

	int matchBracket(const std::string& str, char bracket_left, char bracket_right);

	char safeGetChar(const std::string& str, uint index);

	std::string safeGetSubstr(const std::string& str, uint start, uint size);

	int readInt(const std::string& str, uint& offset, bool& error);

	double readFloat(const std::string& str, uint& offset, bool& error);

	std::string readText(const std::string& str, uint& offset);

	bool findKeyword(const std::string& str, const std::string& keyword, uint offset);
}