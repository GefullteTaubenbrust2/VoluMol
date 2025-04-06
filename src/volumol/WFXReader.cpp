#include "WFXReader.h"
#include "Orbital.h"
#include "TextUtil.h"
#include "Molecule.h"
#include "MolRenderer.h"
#include "Constants.h"

#include "../logic/MathUtil.h"
#include "../logic/TextReading.h"

#include <vector>

namespace mol {
	extern std::vector<ContractedBasis> basis_set;
	extern std::vector<MolecularOrbital> mos;
}

namespace mol::WFX {
	std::vector<std::string> file;
	uint line = 0;
	uint offset = 0;
	std::string l, cur_keyword;
	Molecule molecule;

	void seekKeyWord(const std::string& keyword) {
		line = 0;
		const std::string search_string = "<" + keyword + '>';
		cur_keyword = "</" + keyword + ">";
		for (; line < file.size(); ++line) {
			uint offset = 0;
			l = file[line];
			skipWhitespace(l, offset);
			if (findKeyword(l, search_string, offset)) break;
		}
	}

	void seekKeyWord(const std::string& keyword, uint from_line) {
		line = from_line;
		const std::string search_string = "<" + keyword + '>';
		cur_keyword = "</" + keyword + ">";
		for (; line < file.size(); ++line) {
			uint offset = 0;
			l = file[line];
			skipWhitespace(l, offset);
			if (findKeyword(l, search_string, offset)) break;
		}
	}

	bool nextTerminates() {
		offset = 0;
		++line;
		l = file[line];
		skipWhitespace(l, offset);
		if (findKeyword(l, cur_keyword, offset)) return true;
		return false;
	}

	constexpr glm::ivec3 gto_exponents[56] = {
		// S and P
		glm::ivec3(0, 0, 0), glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 0, 1),

		// D and F
		glm::ivec3(2, 0, 0), glm::ivec3(0, 2, 0), glm::ivec3(0, 0, 2), glm::ivec3(1, 1, 0),
		glm::ivec3(1, 0, 1), glm::ivec3(0, 1, 1), glm::ivec3(3, 0, 0), glm::ivec3(0, 3, 0),
		glm::ivec3(0, 0, 3), glm::ivec3(2, 1, 0), glm::ivec3(2, 0, 1), glm::ivec3(0, 2, 1),
		glm::ivec3(1, 2, 0), glm::ivec3(1, 0, 2), glm::ivec3(0, 1, 2), glm::ivec3(1, 1, 1),

		// G and H
		glm::ivec3(4, 0, 0), glm::ivec3(0, 4, 0), glm::ivec3(0, 0, 4), glm::ivec3(3, 1, 0),
		glm::ivec3(3, 0, 1), glm::ivec3(0, 3, 1), glm::ivec3(1, 3, 0), glm::ivec3(1, 0, 3),
		glm::ivec3(0, 1, 3), glm::ivec3(2, 2, 0), glm::ivec3(2, 0, 2), glm::ivec3(0, 2, 2),
		glm::ivec3(2, 1, 1), glm::ivec3(1, 2, 1), glm::ivec3(1, 1, 2), glm::ivec3(0, 0, 5),
		glm::ivec3(0, 1, 4), glm::ivec3(0, 2, 4), glm::ivec3(0, 3, 2), glm::ivec3(0, 4, 1),
		glm::ivec3(0, 5, 0), glm::ivec3(1, 0, 4), glm::ivec3(1, 1, 3), glm::ivec3(1, 2, 2),
		glm::ivec3(1, 3, 1), glm::ivec3(1, 4, 0), glm::ivec3(2, 0, 3), glm::ivec3(2, 1, 2),
		glm::ivec3(2, 2, 1), glm::ivec3(2, 3, 0), glm::ivec3(3, 0, 2), glm::ivec3(3, 1, 1),
		glm::ivec3(3, 2, 0), glm::ivec3(4, 0, 1), glm::ivec3(4, 1, 0), glm::ivec3(5, 0, 0)
	};

	void loadMOs() {
		seekKeyWord("Number of Primitives");
		if (line >= file.size()) return;

		while (!nextTerminates()) {
			bool error = false;
			int number = readInt(l, offset, error);
			if (!error) {
				basis_set.resize(number);
				break;
			}
		}
		if (!basis_set.size()) {
			std::cerr << "Molecule has no basis functions\n";
			return;
		}

		seekKeyWord("Primitive Centers");
		if (line >= file.size()) {
			std::cerr << "Could not find primitive centers\n";
			return;
		}
		uint index = 0;
		while (!nextTerminates()) {
			while (offset < l.size()) {
				bool error = false;
				int number = readInt(l, offset, error);
				if (error || number < 1 || number > basis_set.size()) {
					std::cerr << "Invalid entry for primitive center\n";
					return;
				}
				skipWhitespace(l, offset);
				basis_set[index].origin = molecule.atoms[number - 1].position;
				if (index >= basis_set.size()) break;
				++index;
			}
		}

		seekKeyWord("Primitive Types");
		if (line >= file.size()) {
			std::cerr << "Could not find primitive types\n";
			return;
		}
		index = 0;
		while (!nextTerminates()) {
			while (offset < l.size()) {
				bool error = false;
				int number = readInt(l, offset, error);
				if (error || number < 1 || number > 56) {
					std::cerr << "Unsupported entry for primitive type\n";
					return;
				}
				number--;
				skipWhitespace(l, offset);
				basis_set[index].gto_primitives.resize(1);
				basis_set[index].gto_primitives[0] = GTO(1.0, gto_exponents[number].x, gto_exponents[number].y, gto_exponents[number].z, 1.0);
				if (index >= basis_set.size()) break;
				++index;
			}
		}

		seekKeyWord("Primitive Exponents");
		if (line >= file.size()) {
			std::cerr << "Could not find primitive exponents\n";
			return;
		}
		index = 0;
		while (!nextTerminates()) {
			while (offset < l.size()) {
				bool error = false;
				double number = readFloat(l, offset, error);
				if (error) {
					std::cerr << "Invalid entry for exponent\n";
					return;
				}
				skipWhitespace(l, offset);
				basis_set[index].gto_primitives.resize(1);
				GTO& gto = basis_set[index].gto_primitives[0];

				// WFX files save the normalization constant in the MO coefficients.
				// However, because they are stored in atomic units, the conversion causes problems.
				// This is a kind of hacky way of dividing away the atomic units constant and
				// replacing it with one for Angstroms.
				gto = GTO(number, gto.e_x, gto.e_y, gto.e_z, 1.0);
				double norm = gto.coeff;
				gto = GTO(number / (a0_A * a0_A), gto.e_x, gto.e_y, gto.e_z, 1.0 / norm);
				if (index >= basis_set.size()) break;
				++index;
			}
		}
		
		seekKeyWord("Molecular Orbital Energies");
		if (line >= file.size()) {
			std::cerr << "Could not find MO energies\n";
			return;
		}
		index = 0;
		while (!nextTerminates()) {
			while (offset < l.size()) {
				bool error = false;
				double number = readFloat(l, offset, error);
				if (error) {
					std::cerr << "Invalid entry for energy\n";
					return;
				}
				skipWhitespace(l, offset);
				if (index >= mos.size()) mos.resize(index + 1);
				mos[index].energy = number;
				++index;
			}
		}

		seekKeyWord("Molecular Orbital Occupation Numbers");
		if (line >= file.size()) {
			std::cerr << "Could not find MO occupation\n";
			return;
		}
		index = 0;
		while (!nextTerminates()) {
			while (offset < l.size()) {
				bool error = false;
				double number = readFloat(l, offset, error);
				if (error) {
					std::cerr << "Invalid entry for occupation\n";
					return;
				}
				skipWhitespace(l, offset);
				if (index >= mos.size()) mos.resize(index + 1);
				mos[index].occupation = number;
				++index;
			}
		}

		seekKeyWord("Molecular Orbital Spin Types");
		if (line >= file.size()) {
			std::cerr << "Could not find MO spin\n";
			return;
		}
		index = 0;
		while (!nextTerminates()) {
			while (offset < l.size()) {
				Spin spin = Spin::alpha;

				if (safeGetSubstr(l, offset, 14) == "Alpha and Beta") {
					spin = Spin::alpha;
					offset += 14;
				}
				else if (safeGetSubstr(l, offset, 5) == "Alpha") {
					spin = Spin::alpha;
					offset += 5;
				}
				else if (safeGetSubstr(l, offset, 4) == "Beta") {
					spin = Spin::beta;
					offset += 4;
				}
				else {
					std::cerr << "Invalid entry for spin\n";
					return;
				}

				skipWhitespace(l, offset);
				if (index >= mos.size()) mos.resize(index + 1);
				mos[index].spin = spin;
				++index;
			}
		}

		seekKeyWord("Molecular Orbital Primitive Coefficients");
		if (line >= file.size()) {
			std::cerr << "Could not find LCAO coefficients\n";
			return;
		}

		while (true) {
			seekKeyWord("MO Number", line);
			MolecularOrbital* mo = nullptr;
			if (line >= file.size()) {
				break;
			}

			while (!nextTerminates()) {
				bool error = false;
				int number = readInt(l, offset, error);
				if (!error && number > 0 && number <= mos.size()) {
					mo = &mos[number - 1];
					break;
				}
			}
			if (!mo) {
				std::cerr << "MO number not found\n";
				return;
			}
			
			mo->basis = &basis_set;
			mo->lcao_coefficients.resize(basis_set.size());

			++line;

			index = 0;
			while (line < file.size()) {
				++line;
				offset = 0;
				l = file[line];
				skipWhitespace(l, offset);

				while (offset < l.size()) {
					bool error = false;
					double number = readFloat(l, offset, error);
					if (error) break;
					skipWhitespace(l, offset);

					mo->lcao_coefficients[index] = number;
					++index;
					if (index >= basis_set.size()) break;
				}
				if (index >= basis_set.size()) break;
			}
		}
	}

	void loadFile(const std::string& path) {
		file = flo::readFile(path);
		molecule.atoms.clear();
		basis_set.clear();
		mos.clear();

		if (!file.size()) {
			std::cerr << "File not found or empty\n";
			return;
		}

		seekKeyWord("Number of Nuclei");
		if (line >= file.size()) {
			std::cerr << "Could not find number of nuclei\n";
			return;
		}

		while (!nextTerminates()) {
			bool error = false;
			int number = readInt(l, offset, error);
			if (!error) {
				molecule.atoms.resize(number);
				break;
			}
		}
		if (!molecule.atoms.size()) {
			std::cerr << "Molecule has no atoms\n";
			return;
		}

		seekKeyWord("Atomic Numbers");
		if (line >= file.size()) {
			std::cerr << "Could not find atomic numbers\n";
			return;
		}
		uint atom = 0;
		while (!nextTerminates()) {
			while (offset < l.size()) {
				bool error = false;
				int number = readInt(l, offset, error);
				if (error) {
					std::cerr << "Invalid entry for atomic number\n";
					return;
				}
				skipWhitespace(l, offset);
				molecule.atoms[atom].Z = number;
				++atom;
				if (atom >= molecule.atoms.size()) break;
			}
		}

		seekKeyWord("Nuclear Cartesian Coordinates");
		if (line >= file.size()) {
			std::cerr << "Could not find nuclear cartesian coordinates\n";
			return;
		}
		uint index = 0;
		while (!nextTerminates()) {
			while (offset < l.size()) {
				bool error = false;
				double number = readFloat(l, offset, error);
				if (error) {
					std::cerr << "Invalid entry for nuclear coordinates\n";
					return;
				}
				skipWhitespace(l, offset);
				molecule.atoms[index / 3].position[index % 3] = number * a0_A;
				++index;
				if (index / 3 >= molecule.atoms.size()) break;
			}
		}

		loadMOs();

		std::cout << "Successfully read WFX file!\n";
		Renderer::setMolecule(molecule);
	}
}
