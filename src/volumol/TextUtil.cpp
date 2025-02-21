#include "TextUtil.h"

namespace mol {
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

	int readInt(const std::string& str, uint& offset, bool& error) {
		char c = safeGetChar(str, offset);
		int start_offset = offset;
		if (c != '-' && c != '+' && !isDigit(c)) {
			error = true;
			return 0;
		}
		++offset;
		for (; offset < str.size(); ++offset) {
			if (!isDigit(str[offset])) break;
		}
		return std::stoi(safeGetSubstr(str, start_offset, offset - start_offset));
	}

	double readFloat(const std::string& str, uint& offset, bool& error) {
		char c = safeGetChar(str, offset);
		int start_offset = offset;
		if (c != '-' && c != '+' && !isDigit(c)) {
			error = true;
			return 0.0;
		}
		++offset;
		bool period = false;
		bool exponential = false;
		for (; offset < str.size(); ++offset) {
			c = str[offset];
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
				c = str[offset];
				if (c != '-' && c != '+' && !isDigit(c)) {
					error = true;
					return 0.0;
				}
				continue;
			}
			if (!isDigit(c)) break;
		}
		return std::stod(safeGetSubstr(str, start_offset, offset - start_offset));
	}

	std::string readText(const std::string& str, uint& offset) {
		uint start = offset;
		for (; offset < str.size(); ++offset) {
			if (!isText(str[offset]) && !isDigit(str[offset])) break;
		}
		return safeGetSubstr(str, start, offset - start);
	}

	bool findKeyword(const std::string& str, const std::string& keyword, uint offset) {
		uint start = offset;
		if (str.length() < offset + keyword.length()) return false;
		for (; offset < str.size(); ++offset) {
			if (offset >= start + keyword.size()) return true;
			if (str[offset] != keyword[offset - start]) return false;
		}
		return true;
	}

	void skipWhitespace(const std::string& str, uint& offset) {
		for (; offset < str.size(); ++offset) {
			if (!isWhiteSpace(str[offset])) break;
		}
	}
}