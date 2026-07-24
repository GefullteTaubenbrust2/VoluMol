#include "SDFReader.h"
#include "TextUtil.h"
#include "Molecule.h"
#include "MolRenderer.h"

namespace mol::SDF {
	using namespace FileReader;

	void loadFile() {
		Molecule molecule;
		bool reading = false;
		bool v3000 = false;
		uint atom_count = 0;
		uint bond_count = 0;
		int index = 0;
		while (!endOfFile()) {
			skipWhitespace();
			if (reading) {
				if (index < atom_count) {
					if (v3000) {
						std::string a1 = readText();
						skipWhitespace();
						std::string a2 = readText();
						skipWhitespace();
						std::string a3 = readText();
						skipWhitespace();

						if (a1 == "M" && a2 == "V30") {
							Atom atom;
							std::string symbol = readText();
							atom.Z = ElementName::ghost_atom;
							for (int i = 0; i < 119; ++i) {
								if (element_names[i] == symbol) {
									atom.Z = i;
									break;
								}
							}
							skipWhitespace();
							bool error = false;
							atom.position.x = readFloat(error);
							skipWhitespace();
							atom.position.y = readFloat(error);
							skipWhitespace();
							atom.position.z = readFloat(error);
							if (error) {
								throwError("Could not read float for XYZ coordinates.");
								return;
							}
							molecule.atoms.push_back(atom);
							++index;
						}
					}
					else {
						Atom atom;
						bool error = false;
						atom.position.x = readFloat(error);
						skipWhitespace();
						atom.position.y = readFloat(error);
						skipWhitespace();
						atom.position.z = readFloat(error);
						if (error) {
							throwError("Could not read float for XYZ coordinates.");
							return;
						}
						skipWhitespace();
						std::string symbol = readText();
						atom.Z = ElementName::ghost_atom;
						for (int i = 0; i < 119; ++i) {
							if (element_names[i] == symbol) {
								atom.Z = i;
								break;
							}
						}
						molecule.atoms.push_back(atom);
						++index;
					}
				}
				else if (index < atom_count + bond_count) {
					if (v3000) {
						std::string a1 = readText();
						skipWhitespace();
						std::string a2 = readText();
						skipWhitespace();
						std::string a3 = readText();
						skipWhitespace();

						if (a1 == "M" && a2 == "V30") {
							bool error = false;
							int order = readInt(error);
							skipWhitespace();
							int a = readInt(error);
							skipWhitespace();
							int b = readInt(error);
							if (error) {
								throwError("Could not read int for bond.");
								return;
							}
							molecule.bonds.push_back(glm::ivec3(a - 1, b - 1, order));
							++index;
						}
					}
					else {
						bool error = false;
						int a = readInt(error);
						skipWhitespace();
						int b = readInt(error);
						skipWhitespace();
						int order = readInt(error);
						if (error) {
							throwError("Could not read int for bond.");
							return;
						}
						molecule.bonds.push_back(glm::ivec3(a - 1, b - 1, order));
						++index;
					}
				}
				else {
					break;
				}
			}
			if (!reading) {
				bool error = false;
				std::string a1 = readContent();
				skipWhitespace();
				std::string a2 = readContent();
				skipWhitespace();
				std::string a3 = readContent();
				skipWhitespace();
				std::string a4 = readContent();
				skipWhitespace();
				std::string a5 = readContent();
				skipWhitespace();

				std::string last = readContent();
				skipWhitespace();
				while (!endOfLine()) {
					last = readContent();
					skipWhitespace();
				}

				if (last == "V2000") {
					reading = true;
					v3000 = false;
					atom_count = std::stoi(a1);
					bond_count = std::stoi(a2);
					molecule.atoms.reserve(atom_count);
					molecule.bonds.reserve(bond_count);
				}
				else if (a1 == "M" && a2 == "V30" && a3 == "COUNTS") {
					reading = true;
					v3000 = true;
					atom_count = std::stoi(a4);
					bond_count = std::stoi(a5);
					molecule.atoms.reserve(atom_count);
					molecule.bonds.reserve(bond_count);
				}
			}
			nextLine();
		}
		mol::Renderer::setMolecule(molecule, false);
	}
}
