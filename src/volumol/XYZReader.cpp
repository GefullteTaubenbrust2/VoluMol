#include "XYZReader.h"
#include "TextUtil.h"
#include "Molecule.h"
#include "MolRenderer.h"

namespace mol::XYZ {
	using namespace FileReader;

	void loadFile() {
		Molecule molecule;
		
		if (getLineCount() < 3) return;

		uint offset = 0;

		skipWhitespace();

		bool error = false;
		uint number_atoms = readInt(error);

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

			for (int i = 0; i < 119; ++i) {
				if (element_names[i] == element_name) {
					element = i;
					break;
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

			molecule.atoms.push_back(Atom(element, glm::vec3(coordinates[0], coordinates[1], coordinates[2])));
		}
		
		Renderer::setMolecule(molecule);
		return;
	}
}
