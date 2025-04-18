#include "Molden.h"

#include <iostream>

#include "MolRenderer.h"
#include "Orbital.h"
#include "TextUtil.h"
#include "Constants.h"

namespace mol {
	extern std::vector<ContractedBasis> basis_set;
	extern std::vector<MolecularOrbital> mos;
}

namespace mol::Molden {
	using namespace FileReader;

	bool atomic_units = false;
	std::map<uint, glm::dvec3> atom_positions;
	bool spherical_d = false;
	bool spherical_f = false;
	bool spherical_g = false;
	bool error = false;
	uint entry = 0;
	Molecule molecule;
	bool use_stos = false;

	enum class Section {
		search = 0,
		atoms = 1,
		gto = 2,
		sto = 3,
		mo = 4,
	};

	Section section = Section::search;

	bool handleFlag(const std::string& l) {
		if (safeGetChar(getLine(), offset) != '[') return false;
		bool found = false;
		if (findKeyword("[5D]")) {
			spherical_d = true;
			spherical_f = true;
			found = true;
		}
		else if (findKeyword("[7F]")) {
			spherical_d = false;
			spherical_f = true;
			found = true;
		}
		else if (findKeyword("[9G]")) {
			spherical_g = true;
			found = true;
		}
		else if (findKeyword("[5D10F]")) {
			spherical_d = true;
			spherical_f = false;
			found = true;
		}
		else if (findKeyword("[5D7F]")) {
			spherical_d = true;
			spherical_f = true;
			found = true;
		}
		if (found) {
			ignoreLine();
			previousLine();
		}
		return found;
	}

	bool handleKeywords(const std::string& l) {
		if (safeGetChar(getLine(), offset) != '[') return false;
		if (findKeyword("[Atoms]")) {
			section = Section::atoms;
			skipWhitespace();
			if (findKeyword("AU")) atomic_units = true;
			entry = 0;
			return true;
		}
		else if (findKeyword("[GTO]")) {
			section = Section::gto;
			use_stos = false;
			entry = 0;
			return true;
		}
		else if (findKeyword("[STO]")) {
			section = Section::sto;
			use_stos = true;
			entry = 0;
			return true;
		}
		else if (findKeyword("[MO]")) {
			section = Section::mo;
			entry = 0;
			return true;
		}
		return false;
	}

	void handleNextLine(bool expect = false) {
		do {
			nextLine();
			if (getLineNumber() >= getLineCount()) {
				if (expect) throwError("Expected additional lines");
				return;
			}
			skipWhitespace();
		} while (handleKeywords(getLine()));
	}

	void addGTO(const std::vector<glm::dvec2>& contractions, int l, int m, glm::dvec3 position) {
		basis_set.push_back(ContractedBasis());
		ContractedBasis& gto = basis_set[basis_set.size() - 1];
		gto.origin = position;
		for (int i = 0; i < contractions.size(); ++i) {
			std::vector<GTO> gtos = generateSphericalGTO(contractions[i].x, l, m);
			for (GTO& g : gtos) g.coeff *= contractions[i].y;
			gto.gto_primitives.insert(gto.gto_primitives.end(), gtos.begin(), gtos.end());
		}
	}

	void addGTO(const std::vector<glm::dvec2>& contractions, int kx, int ky, int kz, glm::dvec3 position) {
		basis_set.push_back(ContractedBasis());
		ContractedBasis& gto = basis_set[basis_set.size() - 1];
		gto.origin = position;
		gto.gto_primitives.resize(contractions.size());
		for (int i = 0; i < contractions.size(); ++i) {
			gto.gto_primitives[i] = GTO(contractions[i].x, kx, ky, kz, contractions[i].y);
		}
	}

	void addGTO(const std::vector<glm::dvec2>& contractions, int l, glm::dvec3 position) {
		switch (l) {
		case 0:
			addGTO(contractions, 0, 0, 0, position);
			break;
		case 1:
			addGTO(contractions, 1, 0, 0, position);
			addGTO(contractions, 0, 1, 0, position);
			addGTO(contractions, 0, 0, 1, position);
			break;
		case 2:
			if (spherical_d) {
				addGTO(contractions, 2, 0, position);
				addGTO(contractions, 2, 1, position);
				addGTO(contractions, 2,-1, position);
				addGTO(contractions, 2, 2, position);
				addGTO(contractions, 2,-2, position);
			}
			else {
				addGTO(contractions, 2, 0, 0, position);
				addGTO(contractions, 0, 2, 0, position);
				addGTO(contractions, 0, 0, 2, position);
				addGTO(contractions, 1, 1, 0, position);
				addGTO(contractions, 1, 0, 1, position);
				addGTO(contractions, 0, 1, 1, position);
			}
			break;
		case 3:
			if (spherical_f) {
				addGTO(contractions, 3, 0, position);
				addGTO(contractions, 3, 1, position);
				addGTO(contractions, 3, -1, position);
				addGTO(contractions, 3, 2, position);
				addGTO(contractions, 3, -2, position);
				addGTO(contractions, 3, 3, position);
				addGTO(contractions, 3, -3, position);
			}
			else {
				addGTO(contractions, 3, 0, 0, position);
				addGTO(contractions, 0, 3, 0, position);
				addGTO(contractions, 0, 0, 3, position);
				addGTO(contractions, 1, 2, 0, position);
				addGTO(contractions, 2, 1, 0, position);
				addGTO(contractions, 2, 0, 1, position);
				addGTO(contractions, 1, 0, 2, position);
				addGTO(contractions, 0, 1, 2, position);
				addGTO(contractions, 0, 2, 1, position);
				addGTO(contractions, 1, 1, 1, position);
			}
			break;
		case 4:
			if (spherical_g) {
				addGTO(contractions, 4, 0, position);
				addGTO(contractions, 4, 1, position);
				addGTO(contractions, 4, -1, position);
				addGTO(contractions, 4, 2, position);
				addGTO(contractions, 4, -2, position);
				addGTO(contractions, 4, 3, position);
				addGTO(contractions, 4, -3, position);
				addGTO(contractions, 4, 4, position);
				addGTO(contractions, 4, -4, position);
			}
			else {
				addGTO(contractions, 4, 0, 0, position);
				addGTO(contractions, 0, 4, 0, position);
				addGTO(contractions, 0, 0, 4, position);
				addGTO(contractions, 3, 1, 0, position);
				addGTO(contractions, 3, 0, 1, position);
				addGTO(contractions, 1, 3, 0, position);
				addGTO(contractions, 0, 3, 1, position);
				addGTO(contractions, 1, 0, 3, position);
				addGTO(contractions, 0, 1, 3, position);
				addGTO(contractions, 2, 2, 0, position);
				addGTO(contractions, 2, 0, 2, position);
				addGTO(contractions, 0, 2, 2, position);
				addGTO(contractions, 2, 1, 1, position);
				addGTO(contractions, 1, 2, 1, position);
				addGTO(contractions, 1, 1, 2, position);
			}
			break;
		}
	}

	void loadFile() {
		atomic_units = false;
		spherical_d = false;
		spherical_f = false;
		spherical_g = false;
		error = false;
		use_stos = false;
		section = Section::search;
		molecule.atoms.clear();
		atom_positions.clear();

		for (; !endOfFile(); nextLine()) {
			handleFlag(getLine());
		}

		setLineNumber(0);

		for (; !endOfFile(); handleNextLine()) {
			switch (section) {
			case(Section::atoms): {
				std::string element = readText();
				skipWhitespace();
				int id = readInt(error);
				skipWhitespace();
				int Z = readInt(error);
				skipWhitespace();
				glm::dvec3 position;
				position.x = readFloat(error);
				skipWhitespace();
				position.y = readFloat(error);
				skipWhitespace();
				position.z = readFloat(error);
				if (error) {
					throwError("Illegal formatting of atom definition");
					return;
				}
				if (atomic_units) position *= a0_A;
				molecule.atoms.push_back(Atom(Z, position));
				molecule.index_map.push_back(id);
				atom_positions.insert(std::make_pair(id, position));
				break;
			}
			case(Section::gto): {
				uint atom = readInt(error);
				if (error) {
					throwError("Incorrectly formatted number (GTO atom number)");
					return;
				}

				auto iter = atom_positions.find(atom);
				if (iter == atom_positions.end()) {
					throwError("Invalid atom number");
					return;
				}

				glm::dvec3 position = iter->second;

				for (handleNextLine(); !endOfFile(); handleNextLine()) {
					if (offset >= getLine().size()) break;

					char label = getLine()[offset];

					uint shell = 0;
					if (label == 'p') shell = 1;
					if (label == 'd') shell = 2;
					if (label == 'f') shell = 3;
					if (label == 'g') shell = 4;

					++offset;

					skipWhitespace();

					int primitive_count = readInt(error);
					if (error) {
						throwError("Incorrectly formatted number (primitive count)");
						return;
					}

					uint last_line = getLineNumber() + primitive_count;
					if (last_line >= getLineCount()) {
						throwError("GTO definition contains less primitives than specified");
						return;
					}

					std::vector<glm::dvec2> contractions;
					for (handleNextLine(); !endOfFile(); handleNextLine()) {
						double alpha = readFloat(error) / (a0_A * a0_A);
						skipWhitespace();
						double c = readFloat(error);
						if (error) {
							throwError("Incorrectly formatted number (contraction parameters)");
							return;
						}
						contractions.push_back(glm::dvec2(alpha, c));
						if (getLineNumber() >= last_line) break;
					}
					addGTO(contractions, shell, position);
				}
				break;
			}
			case(Section::sto): {
				int id = readInt(error);
				skipWhitespace();
				int kx = readInt(error);
				skipWhitespace();
				int ky = readInt(error);
				skipWhitespace();
				int kz = readInt(error);
				skipWhitespace();
				int kr = readInt(error);
				skipWhitespace();
				double alpha = readFloat(error);
				skipWhitespace();
				double coeff = readFloat(error);
				if (error) {
					throwError("Illegal formatting of STO definition");
				}

				auto iter = atom_positions.find(id);
				if (iter == atom_positions.end()) {
					throwError("Invalid atom number");
					return;
				}

				glm::dvec3 position = iter->second;

				basis_set.push_back(ContractedBasis());
				basis_set[basis_set.size() - 1].sto_primitives = std::vector<STO>{ STO(alpha, kr, kx, ky, kz, coeff) };
				basis_set[basis_set.size() - 1].origin = position;
				break;
			}
			case(Section::mo): {
				if (offset >= getLine().size()) {
					section = Section::search;
					break;
				}
				if (getLineNumber() + 4 >= getLineCount()) {
					throwError("Expected additional data for MO");
					return;
				}

				mos.push_back(MolecularOrbital());
				MolecularOrbital& mo = mos[mos.size() - 1];
				mo.lcao_coefficients = std::vector<double>(basis_set.size(), 0.0);
				mo.basis = &basis_set;
				mo.use_stos = use_stos;

				if (!findKeyword("Sym=")) {
					throwError("Expected 'Sym' keyword");
					return;
				}
				skipWhitespace();
				mo.name = readText();
				
				handleNextLine();

				if (!findKeyword("Ene=")) {
					throwError("Expected 'Ene' keyword");
					return;
				}
				skipWhitespace();
				mo.energy = readFloat(error);
				if (error) {
					throwError("Incorrectly formatted number (MO Ene)");
					return;
				}

				handleNextLine();

				if (!findKeyword("Spin=")) {
					throwError("Expected 'Spin' keyword");
					return;
				}
				skipWhitespace();
				std::string spin = readText();
				if (spin != "Alpha" && spin != "Beta") {
					throwError("Expected 'Spin' to be 'Alpha' or 'Beta'");
					return;
				}
				mo.spin = spin[0] == 'A' ? Spin::alpha : Spin::beta;

				handleNextLine();

				if (!findKeyword("Occup=")) {
					throwError("Expected 'Occup' keyword");
					return;
				}
				skipWhitespace();
				mo.occupation = readFloat(error);
				if (error) {
					throwError("Incorrectly formatted number (MO Occup)");
					return;
				}
				
				handleNextLine();

				while (!endOfFile()) {
					if (!isDigit(safeGetChar(getLine(), offset))) {
						setLineNumber(getLineNumber() - 1);
						break;
					}
					
					uint ao_index = readInt(error) - 1;
					skipWhitespace();
					double lcao_coefficient = readFloat(error);
					if (error) {
						throwError("Incorrectly formatted number MO (LCAO coefficient)");
						return;
					}
					if (ao_index >= basis_set.size()) {
						throwError("AO index out of bounds");
						return;
					}
					mo.lcao_coefficients[ao_index] = lcao_coefficient;

					handleNextLine();
				}
				break;
			}
			default: {
				break;
			}
			}
		}
		std::cout << "Successfully loaded Molden file!\n";
		Renderer::setMolecule(molecule);
	}
}