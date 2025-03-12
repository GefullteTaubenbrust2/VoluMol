#include "XYZReader.h"
#include "TextUtil.h"

namespace mol {
	Molecule readXYZ(const std::vector<std::string>& file) {
		Molecule molecule;
		
		if (file.size() < 3) return molecule;

		uint offset = 0;

		for (; offset < file[0].size(); ++offset) if (!isWhiteSpace(file[0][offset])) break;

		bool error = false;
		uint number_atoms = readInt(file[0], offset, error);

		if (error) return molecule;

		uint real_number_atoms = 0;

		for (int l = 2; l < file.size(); ++l) {
			std::string line = file[l];
			offset = 0;

			if (line.size() < 1) continue;

			for (; offset < line.size(); ++offset) if (!isWhiteSpace(line[offset])) break;

			if (!isText(line[offset])) break;

			std::string element_name = readText(line, offset);
			uint element = ghost_atom;

			for (int i = 0; i < 119; ++i) {
				if (element_names[i] == element_name) {
					element = i;
					break;
				}
			}

			float coordinates[3];
			for (int i = 0; i < 3; ++i) {
				for (; offset < line.size(); ++offset) if (!isWhiteSpace(line[offset])) break;
				coordinates[i] = readFloat(line, offset, error);
				if (error) return molecule;
			}

			molecule.atoms.push_back(Atom(element, glm::vec3(coordinates[0], coordinates[1], coordinates[2])));
		}
		return molecule;
	}
}
