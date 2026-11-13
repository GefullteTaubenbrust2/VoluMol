#include "XYZReader.h"
#include "TextUtil.h"
#include "Molecule.h"
#include "MolRenderer.h"

namespace mol {
	extern glm::vec3 lattice_vectors[3];
}

namespace mol::XYZ {
	using namespace FileReader;

	void loadFile() {
		Molecule molecule;
		
		if (getLineCount() < 3) return;

		uint offset = 0;

		skipWhitespace();

		bool error = false;
		uint number_atoms = readInt(error);
		molecule.atoms.reserve(number_atoms);

		if (error) {
			throwError("Could not read number of atoms");
			return;
		}

		uint real_number_atoms = 0;

		for (setLineNumber(2); !endOfFile(); nextLine()) {
			skipWhitespace();

			if (endOfLine()) continue;

			std::string element_name = readText();
			uint element = ghost_atom;

			int axis = -1;

			if (element_name == "VEC1") axis = 0;
			else if (element_name == "VEC2") axis = 1;
			else if (element_name == "VEC3") axis = 2;
			else {
				for (int i = 0; i < 119; ++i) {
					if (element_names[i] == element_name) {
						element = i;
						break;
					}
				}
			}

			float coordinates[3];
			for (int i = 0; i < 3; ++i) {
				skipWhitespace();
				coordinates[i] = readFloat(error);
				if (error) {
					throwError("Could not read atomic coordinate");
					return;
				}
			}

			if (axis < 0) molecule.atoms.push_back(Atom(element, glm::vec3(coordinates[0], coordinates[1], coordinates[2])));
			else lattice_vectors[axis] = glm::vec3(coordinates[0], coordinates[1], coordinates[2]);
		}
		
		Renderer::setMolecule(molecule);
		return;
	}
}
