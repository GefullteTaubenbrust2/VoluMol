#include "WFXReader.h"
#include "Orbital.h"
#include "TextUtil.h"
#include "Molecule.h"
#include "MolRenderer.h"
#include "Constants.h"

#include <vector>

namespace mol {
	extern std::vector<ContractedBasis> basis_set;
	extern std::vector<MolecularOrbital> mos;
}

namespace mol::WFX {
	using namespace FileReader;

	std::string cur_keyword;
	Molecule molecule;

	void seekKeyWord(const std::string& keyword, uint from_line = 0) {
		setLineNumber(from_line);
		const std::string search_string = "<" + keyword + '>';
		cur_keyword = "</" + keyword + ">";
		for (; getLineNumber() < getLineCount(); nextLine()) {
			skipWhitespace();
			if (findKeyword(search_string)) break;
		}
	}

	bool nextTerminates() {
		nextLine();
		skipWhitespace();
		if (findKeyword(cur_keyword)) return true;
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
		if (endOfFile()) return;

		while (!nextTerminates()) {
			bool error = false;
			int number = readInt(error);
			if (!error) {
				basis_set.resize(number);
				break;
			}
		}
		if (!basis_set.size()) {
			throwError("Molecule has no basis functions");
			return;
		}

		seekKeyWord("Primitive Centers");
		if (endOfFile()) {
			throwError("Could not find primitive centers");
			return;
		}
		uint index = 0;
		while (!nextTerminates()) {
			while (!endOfLine()) {
				bool error = false;
				int number = readInt(error);
				if (error || number < 1 || number > basis_set.size()) {
					throwError("Invalid entry for primitive center");
					return;
				}
				skipWhitespace();
				basis_set[index].origin = molecule.atoms[number - 1].position;
				if (index >= basis_set.size()) break;
				++index;
			}
		}

		seekKeyWord("Primitive Types");
		if (endOfFile()) {
			throwError("Could not find primitive types");
			return;
		}
		index = 0;
		while (!nextTerminates()) {
			while (!endOfLine()) {
				bool error = false;
				int number = readInt(error);
				if (error || number < 1 || number > 56) {
					throwError("Unsupported entry for primitive type");
					return;
				}
				number--;
				skipWhitespace();
				basis_set[index].gto_primitives.resize(1);
				basis_set[index].gto_primitives[0] = GTO(1.0, gto_exponents[number].x, gto_exponents[number].y, gto_exponents[number].z, 1.0);
				if (index >= basis_set.size()) break;
				++index;
			}
		}

		seekKeyWord("Primitive Exponents");
		if (endOfFile()) {
			throwError("Could not find primitive exponents");
			return;
		}
		index = 0;
		while (!nextTerminates()) {
			while (!endOfLine()) {
				bool error = false;
				double number = readFloat(error);
				if (error) {
					throwError("Invalid entry for exponent");
					return;
				}
				skipWhitespace();
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
		if (endOfFile()) {
			throwError("Could not find MO energies");
			return;
		}
		index = 0;
		while (!nextTerminates()) {
			while (!endOfLine()) {
				bool error = false;
				double number = readFloat(error);
				if (error) {
					throwError("Invalid entry for energy");
					return;
				}
				skipWhitespace();
				if (index >= mos.size()) mos.resize(index + 1);
				mos[index].energy = number;
				++index;
			}
		}

		seekKeyWord("Molecular Orbital Occupation Numbers");
		if (endOfFile()) {
			throwError("Could not find MO occupation");
			return;
		}
		index = 0;
		while (!nextTerminates()) {
			while (!endOfLine()) {
				bool error = false;
				double number = readFloat(error);
				if (error) {
					throwError("Invalid entry for occupation");
					return;
				}
				skipWhitespace();
				if (index >= mos.size()) mos.resize(index + 1);
				mos[index].occupation = number;
				++index;
			}
		}

		seekKeyWord("Molecular Orbital Spin Types");
		if (endOfFile()) {
			throwError("Could not find MO spin");
			return;
		}
		index = 0;
		while (!nextTerminates()) {
			while (!endOfLine()) {
				Spin spin = Spin::alpha;

				if (findKeyword("Alpha and Beta")) spin = Spin::alpha;
				else if (findKeyword("Alpha")) spin = Spin::alpha;
				else if (findKeyword("Beta")) spin = Spin::beta;
				else {
					throwError("Invalid entry for spin");
					return;
				}

				skipWhitespace();
				if (index >= mos.size()) mos.resize(index + 1);
				mos[index].spin = spin;
				++index;
			}
		}

		seekKeyWord("Molecular Orbital Primitive Coefficients");
		if (endOfFile()) {
			throwError("Could not find LCAO coefficients");
			return;
		}

		while (true) {
			seekKeyWord("MO Number", getLineNumber());
			MolecularOrbital* mo = nullptr;
			if (endOfFile()) {
				break;
			}

			while (!nextTerminates()) {
				bool error = false;
				int number = readInt(error);
				if (!error && number > 0 && number <= mos.size()) {
					mo = &mos[number - 1];
				}
			}
			if (!mo) {
				throwError("MO number not found");
				return;
			}
			
			mo->basis = &basis_set;
			mo->lcao_coefficients.resize(basis_set.size());

			index = 0;
			while (!endOfFile()) {
				nextLine();
				skipWhitespace();

				while (!endOfLine()) {
					bool error = false;
					double number = readFloat(error);
					if (error) break;
					skipWhitespace();

					mo->lcao_coefficients[index] = number;
					++index;
					if (index >= basis_set.size()) break;
				}
				if (index >= basis_set.size()) break;
			}
		}
	}

	void loadFile() {
		molecule.atoms.clear();
		basis_set.clear();
		mos.clear();

		seekKeyWord("Number of Nuclei");
		if (endOfFile()) {
			throwError("Could not find number of nuclei");
			return;
		}

		while (!nextTerminates()) {
			bool error = false;
			int number = readInt(error);
			if (!error) {
				molecule.atoms.resize(number);
				break;
			}
		}
		if (!molecule.atoms.size()) {
			throwError("Molecule has no atoms");
			return;
		}

		seekKeyWord("Atomic Numbers");
		if (endOfFile()) {
			throwError("Could not find atomic numbers");
			return;
		}
		uint atom = 0;
		while (!nextTerminates()) {
			while (!endOfLine()) {
				bool error = false;
				int number = readInt(error);
				if (error) {
					throwError("Invalid entry for atomic number");
					return;
				}
				skipWhitespace();
				molecule.atoms[atom].Z = number;
				++atom;
				if (atom >= molecule.atoms.size()) break;
			}
		}

		seekKeyWord("Nuclear Cartesian Coordinates");
		if (endOfFile()) {
			throwError("Could not find nuclear cartesian coordinates");
			return;
		}
		uint index = 0;
		while (!nextTerminates()) {
			while (!endOfLine()) {
				bool error = false;
				double number = readFloat(error);
				if (error) {
					throwError("Invalid entry for nuclear coordinates");
					return;
				}
				skipWhitespace();
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
