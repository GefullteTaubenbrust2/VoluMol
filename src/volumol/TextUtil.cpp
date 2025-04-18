#include "TextUtil.h"

#include <iostream>

#include "../logic/TextReading.h"

namespace mol::FileReader {
	std::vector<std::string> file;
	std::string l;
	uint line, offset;

	bool readFile(const std::string& path) {
		file = flo::readFile(path);
		if (!file.size()) {
			std::cerr << "File is empty or does not exist\n";
			return false;
		}
		line = 0;
		offset = 0;
		if (file.size()) l = file[0];
		else l = "";
		return true;
	}

	void setFile(const std::vector<std::string>& _file) {
		file = _file;
		line = 0;
		offset = 0;
		if (file.size()) l = file[0];
		else l = "";
	}

	void throwError(const std::string& message) {
		std::cerr << "Error occured reading file: \n" << message << "\nIn line: " << line << "\nOffending content:\n";
		for (int i = line - 2; i <= line + 2; ++i) {
			if (i < 0) i = 0;
			if (i >= file.size()) break;
			std::cerr << file[i];
			if (i == line) std::cerr << " <<< ERROR";
			std::cerr << '\n';
		}
	}

	std::string& getLine() {
		return l;
	}

	uint getLineNumber() {
		return line;
	}

	void setLineNumber(uint _line) {
		line = _line;
		offset = 0;
		if (line < file.size()) l = file[line];
		else l = "";
	}

	void nextLine() {
		++line;
		offset = 0;
		if (line < file.size()) l = file[line];
		else l = "";
	}

	void previousLine() {
		if (!line) return;
		--line;
		offset = 0;
		if (line < file.size()) l = file[line];
		else l = "";
	}

	uint getLineCount() {
		return file.size();
	}

	bool endOfFile() {
		return line >= file.size();
	}

	bool endOfLine() {
		return offset >= l.size();
	}

	void ignoreLine() {
		if (line >= file.size()) return;
		file.erase(file.begin() + line);
	}

	bool isWhiteSpace(char c) {
		return c == ' ' || c == '\n' || c == '\r' || c == '\t';
	}

	bool isText(char c) {
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
	}

	bool isDigit(char c) {
		return c >= '0' && c <= '9';
	}

	int matchBracket(const std::string& str, char bracket_left, char bracket_right) {
		int bracket_level = 0;
		bool bracket_located = false;
		for (int i = 0; i < str.size(); ++i) {
			char c = str[i];
			if (c == bracket_left) {
				++bracket_level;
				bracket_located = true;
			}
			else if (c == bracket_right) {
				--bracket_level;
				if (!bracket_level) return i;
			}
		}
		return 0;
	}

	char safeGetChar(const std::string& str, uint index) {
		if (index >= str.size()) return 0;
		return str[index];
	}

	std::string safeGetSubstr(const std::string& str, uint start, uint size) {
		if (!str.size()) return "";
		if (start >= str.length()) {
			start = str.length() - 1;
			size = 0;
		}
		if (start + size > str.length()) {
			size = str.length() - 1 - start;
		}
		return str.substr(start, size);
	}

	int readInt(bool& error) {
		char c = safeGetChar(l, offset);
		int start_offset = offset;
		if (c != '-' && c != '+' && !isDigit(c)) {
			error = true;
			return 0;
		}
		++offset;
		for (; offset < l.size(); ++offset) {
			if (!isDigit(l[offset])) break;
		}
		return std::stoi(safeGetSubstr(l, start_offset, offset - start_offset));
	}

	double readFloat(bool& error) {
		char c = safeGetChar(l, offset);
		int start_offset = offset;
		if (c != '-' && c != '+' && !isDigit(c)) {
			error = true;
			return 0.0;
		}
		++offset;
		bool period = false;
		bool exponential = false;
		for (; offset < l.size(); ++offset) {
			c = l[offset];
			if (c == '.') {
				if (period) break;
				period = true;
				continue;
			}
			if (c == 'e' || c == 'E') {
				if (exponential) break;
				period = true;
				exponential = true;
				++offset;
				c = safeGetChar(l, offset);
				if (c != '-' && c != '+' && !isDigit(c)) {
					error = true;
					return 0.0;
				}
				continue;
			}
			if (!isDigit(c)) break;
		}
		return std::stod(safeGetSubstr(l, start_offset, offset - start_offset));
	}

	std::string readText() {
		uint start = offset;
		for (; offset < l.size(); ++offset) {
			if (!isText(l[offset]) && !isDigit(l[offset])) break;
		}
		return safeGetSubstr(l, start, offset - start);
	}

	bool findKeyword(const std::string& keyword) {
		if (l.size() < offset + keyword.size()) return false;
		for (uint i = offset; i < l.size(); ++i) {
			if (i >= offset + keyword.size()) break;
			if (l[i] != keyword[i - offset]) return false;
		}
		offset += keyword.size();
		return true;
	}

	void skipWhitespace() {
		for (; offset < l.size(); ++offset) {
			if (!isWhiteSpace(l[offset])) break;
		}
	}
}