#pragma once
#include "Molecule.h"

namespace mol {
	Molecule readXYZ(const std::vector<std::string>& file);
}