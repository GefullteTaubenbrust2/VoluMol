#include "TextReading.h"

namespace flo {
	std::istream& safeGetline(std::istream& is, std::string& t)
	{
		t.clear();

		// The characters in the stream are read one-by-one using a std::streambuf.
		// That is faster than reading them one-by-one using the std::istream.
		// Code that uses streambuf this way must be guarded by a sentry object.
		// The sentry object performs various tasks,
		// such as thread synchronization and updating the stream state.

		std::istream::sentry se(is, true);
		std::streambuf* sb = is.rdbuf();

		for (;;) {
			int c = sb->sbumpc();
			switch (c) {
			case '\n':
				return is;
			case '\r':
				if (sb->sgetc() == '\n')
					sb->sbumpc();
				return is;
			case std::streambuf::traits_type::eof():
				// Also handle the case when the last line has no line ending
				if (t.empty())
					is.setstate(std::ios::eofbit);
				return is;
			default:
				t += (char)c;
			}
		}
	}
	
	std::vector<std::string> readFile(const std::string& path) {
		std::ifstream file;
		std::vector<std::string> result;
		file.open(path);
		if (!file.is_open()) return result;

		while (file) {
			std::string str;
			safeGetline(file, str);
			result.push_back(str);
		}

		return result;
	}

	std::string readFullFile(const std::string& path) {
		std::ifstream file;
		std::string result;
		file.open(path);
		if (!file.is_open()) return result;

		while (file) {
			std::string str;
			safeGetline(file, str);
			result += str + '\n';
		}

		return result;
	}
}