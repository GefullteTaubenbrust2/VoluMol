#pragma once
#include <fstream>
#include <vector>

namespace flo {
	std::istream& safeGetline(std::istream& is, std::string& t);

	std::vector<std::string> readFile(const std::string& path);
}