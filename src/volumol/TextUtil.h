#pragma once
#include <string>
#include <vector>
#include "../logic/Types.h"

namespace mol::FileReader {
	extern uint offset;

	bool readFile(const std::string& path);

	void setFile(const std::vector<std::string>& file);

	void throwError(const std::string& message);

	std::string& getLine();

	uint getLineNumber();

	void setLineNumber(uint line);

	void nextLine();

	void previousLine();

	uint getLineCount();

	bool endOfFile();

	bool endOfLine();

	void ignoreLine();

	bool isWhiteSpace(char c);

	bool isText(char c);

	bool isDigit(char c);

	char safeGetChar(const std::string& str, uint index);

	std::string safeGetSubstr(const std::string& str, uint start, uint size);

	int readInt(bool& error);

	double readFloat(bool& error);

	std::string readText();

	bool findKeyword(const std::string& keyword);

	void skipWhitespace();
}