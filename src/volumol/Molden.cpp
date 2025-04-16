#include "Molden.h"

#include <iostream>

#include "../logic/TextReading.h"
#include "MolRenderer.h"
#include "Orbital.h"
#include "TextUtil.h"
#include "Constants.h"

namespace mol {
	extern std::vector<ContractedBasis> basis_set;
	extern std::vector<MolecularOrbital> mos;
}

namespace mol::Molden {
	std::vector<std::string> file;
	uint line = 0;
	uint column = 0;
	bool atomic_units = false;
	std::map<uint, glm::dvec3> atom_positions;
	bool spherical_d = false;
	bool spherical_f = false;
	bool spherical_g = false;
	bool error = false;
	uint entry = 0;
	Molecule molecule;
	bool use_stos = false;

	std::string l;

	enum class Section {
		search = 0,
		atoms = 1,
		gto = 2,
		sto = 3,
		mo = 4,
	};

	Section section = Section::search;

	void skipWhiteSpace() {
		skipWhitespace(l, column);
	}

	void throwError(const std::string& message) {
		error = true;
		std::cerr << "Error occured reading Molden file: \n" << message << "\nIn line: " << line << "\nOffending content:\n";
		for (int i = line - 2; i <= line + 2; ++i) {
			if (i < 0) i = 0;
			if (i >= file.size()) break;
			std::cerr << file[i];
			if (i == line) std::cerr << " <<< ERROR";
			std::cerr << '\n';
		}
	}

	bool handleFlag(const std::string& l) {
		if (safeGetChar(l, column) != '[') return false;
		if (findKeyword(l, "[5D]", column)) {
			spherical_d = true;
			spherical_f = true;
			return true;
		}
		else if (findKeyword(l, "[7F]", column)) {
			spherical_f = true;
			return true;
		}
		else if (findKeyword(l, "[9G]", column)) {
			spherical_g = true;
			return true;
		}
		else if (findKeyword(l, "[5D10F]", column)) {
			spherical_d = true;
			spherical_f = false;
			return true;
		}
		else if (findKeyword(l, "[5D7F]", column)) {
			spherical_d = true;
			spherical_f = true;
			return true;
		}
		return false;
	}

	bool handleKeywords(const std::string& l) {
		if (safeGetChar(l, column) != '[') return false;
		if (findKeyword(l, "[Atoms]", column)) {
			section = Section::atoms;
			column += 7;
			skipWhiteSpace();
			if (findKeyword(l, "AU", column)) atomic_units = true;
			entry = 0;
			return true;
		}
		else if (findKeyword(l, "[GTO]", column)) {
			section = Section::gto;
			use_stos = false;
			entry = 0;
			return true;
		}
		else if (findKeyword(l, "[STO]", column)) {
			section = Section::sto;
			use_stos = true;
			entry = 0;
			return true;
		}
		else if (findKeyword(l, "[MO]", column)) {
			section = Section::mo;
			entry = 0;
			return true;
		}
		return false;
	}

	void nextLine(bool expect = false) {
		do {
			++line;
			column = 0;
			if (line >= file.size()) {
				if (expect) throwError("Expected additional lines");
				return;
			}
			l = file[line];
			skipWhiteSpace();
		} while (handleKeywords(l));
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

	void loadFile(const std::string& path) {
		file = flo::readFile(path);
		if (!file.size()) {
			std::cout << "File not found for empty\n";
			return;
		}

		line = 0;
		column = 0;
		atomic_units = false;
		spherical_d = false;
		spherical_f = false;
		spherical_g = false;
		error = false;
		use_stos = false;
		section = Section::search;
		molecule.atoms.clear();
		atom_positions.clear();

		for (; line < file.size(); nextLine()) {
			if (handleFlag(l)) {
				file.erase(file.begin() + line);
				--line;
			}
		}

		line = 0;
		column = 0;

		for (; line < file.size(); nextLine()) {
			switch (section) {
			case(Section::atoms): {
				std::string element = readText(l, column);
				skipWhiteSpace();
				int id = readInt(l, column, error);
				skipWhiteSpace();
				int Z = readInt(l, column, error);
				skipWhiteSpace();
				glm::dvec3 position;
				position.x = readFloat(l, column, error);
				skipWhiteSpace();
				position.y = readFloat(l, column, error);
				skipWhiteSpace();
				position.z = readFloat(l, column, error);
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
				uint atom = readInt(l, column, error);
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

				for (nextLine(); line < file.size(); nextLine()) {
					if (column >= l.size()) break;

					char label = l[column];

					uint shell = 0;
					if (label == 'p') shell = 1;
					if (label == 'd') shell = 2;
					if (label == 'f') shell = 3;
					if (label == 'g') shell = 4;

					++column;

					skipWhiteSpace();

					int primitive_count = readInt(l, column, error);
					if (error) {
						throwError("Incorrectly formatted number (primitive count)");
						return;
					}

					uint last_line = line + primitive_count;
					if (last_line >= file.size()) {
						throwError("GTO definition contains less primitives than specified");
						return;
					}

					std::vector<glm::dvec2> contractions;
					for (nextLine(); line ; nextLine()) {
						double alpha = readFloat(l, column, error) / (a0_A * a0_A);
						skipWhiteSpace();
						double c = readFloat(l, column, error);
						if (error) {
							throwError("Incorrectly formatted number (contraction parameters)");
							return;
						}
						contractions.push_back(glm::dvec2(alpha, c));
						if (line >= last_line) break;
					}
					addGTO(contractions, shell, position);
				}
				break;
			}
			case(Section::sto): {
				int id = readInt(l, column, error);
				skipWhiteSpace();
				int kx = readInt(l, column, error);
				skipWhiteSpace();
				int ky = readInt(l, column, error);
				skipWhiteSpace();
				int kz = readInt(l, column, error);
				skipWhiteSpace();
				int kr = readInt(l, column, error);
				skipWhiteSpace();
				double alpha = readFloat(l, column, error);
				skipWhiteSpace();
				double coeff = readFloat(l, column, error);
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
				if (column >= l.size()) {
					section = Section::search;
					break;
				}
				if (line + 4 >= file.size()) {
					throwError("Expected additional data for MO");
					return;
				}

				mos.push_back(MolecularOrbital());
				MolecularOrbital& mo = mos[mos.size() - 1];
				mo.lcao_coefficients = std::vector<double>(basis_set.size(), 0.0);
				mo.basis = &basis_set;
				mo.use_stos = use_stos;

				if (!findKeyword(l, "Sym=", column)) {
					throwError("Expected 'Sym' keyword");
					return;
				}
				column += 4;
				skipWhiteSpace();
				mo.name = readText(l, column);
				
				nextLine();

				if (!findKeyword(l, "Ene=", column)) {
					throwError("Expected 'Ene' keyword");
					return;
				}
				column += 4;
				skipWhiteSpace();
				mo.energy = readFloat(l, column, error);
				if (error) {
					throwError("Incorrectly formatted number (MO Ene)");
					return;
				}

				nextLine();

				if (!findKeyword(l, "Spin=", column)) {
					throwError("Expected 'Spin' keyword");
					return;
				}
				column += 5;
				skipWhiteSpace();
				std::string spin = readText(l, column);
				if (spin != "Alpha" && spin != "Beta") {
					throwError("Expected 'Spin' to be 'Alpha' or 'Beta'");
					return;
				}
				mo.spin = spin[0] == 'A' ? Spin::alpha : Spin::beta;

				nextLine();

				if (!findKeyword(l, "Occup=", column)) {
					throwError("Expected 'Occup' keyword");
					return;
				}
				column += 6;
				skipWhiteSpace();
				mo.occupation = readFloat(l, column, error);
				if (error) {
					throwError("Incorrectly formatted number (MO Occup)");
					return;
				}
				
				nextLine();

				while (line < file.size()) {
					if (!isDigit(safeGetChar(l, column))) {
						--line;
						break;
					}
					
					uint ao_index = readInt(l, column, error) - 1;
					skipWhiteSpace();
					double lcao_coefficient = readFloat(l, column, error);
					if (error) {
						throwError("Incorrectly formatted number MO (LCAO coefficient)");
						return;
					}
					if (ao_index >= basis_set.size()) {
						throwError("AO index out of bounds");
						return;
					}
					mo.lcao_coefficients[ao_index] = lcao_coefficient;

					nextLine();
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
