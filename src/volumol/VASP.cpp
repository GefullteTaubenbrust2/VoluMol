#include "VASP.h"
#include "Molecule.h"
#include "TextUtil.h"
#include "MolRenderer.h"

namespace mol {
	extern glm::vec3 lattice_vectors[3];
}

namespace mol::VASP {
	using namespace FileReader;

	glm::vec3 readVec(const std::string& err, bool& error) {
		skipWhitespace();
		float v0 = readFloat(error);
		if (error) {
			throwError(err);
			return glm::vec3(0.0);
		}
		skipWhitespace();
		float v1 = readFloat(error);
		if (error) {
			throwError(err);
			return glm::vec3(0.0);
		}
		skipWhitespace();
		float v2 = readFloat(error);
		if (error) {
			throwError(err);
			return glm::vec3(0.0);
		}
		return glm::vec3(v0, v1, v2);
	}

	void loadFile() {
		Molecule molecule;

		nextLine();
		skipWhitespace();

		bool error = false;
		float scale_factor = readFloat(error);
		if (error) {
			throwError("Could not read scaling factor");
			return;
		}

		nextLine();
		skipWhitespace();
		lattice_vectors[0] = readVec("Could not read lattice vector", error) * scale_factor;
		if (error) return;
		nextLine();
		skipWhitespace();
		lattice_vectors[1] = readVec("Could not read lattice vector", error) * scale_factor;
		if (error) return;
		nextLine();
		skipWhitespace();
		lattice_vectors[2] = readVec("Could not read lattice vector", error) * scale_factor;
		if (error) return;
		nextLine();
		skipWhitespace();
		
		std::vector<uint> elements;
		std::vector<int> counts;

		while (!endOfLine()) {
			std::string word = readText();
			uint element = ElementName::ghost_atom;
			for (int i = 0; i < 119; ++i) {
				if (word == element_names[i]) element = i;
			}
			elements.push_back(element);
			skipWhitespace();
		}
		nextLine();
		skipWhitespace();
		while (!endOfLine()) {
			int count = readInt(error);
			if (error) {
				throwError("Could not read element count");
				return;
			}
			skipWhitespace();
			counts.push_back(count);
		}

		if (elements.size() != counts.size()) {
			throwError("Number of specified types and the corresponding numbers of atoms is not equal");
			return;
		}

		nextLine();
		skipWhitespace();

		std::string word = readText();
		if (word[0] == 's' || word[0] == 'S') {
			nextLine();
			skipWhitespace();
			word = readText();
		}
		char c = word[0];
		bool cartesian = c == 'C' || c == 'c' || c == 'K' || c == 'k';

		nextLine();
		skipWhitespace();

		for (int i = 0; i < elements.size(); ++i) {
			for (int j = 0; j < counts[i]; ++j) {
				skipWhitespace();
				glm::vec3 position = readVec("Could not read atom position", error);
				if (error) return;
				if (!cartesian) {
					position = position[0] * lattice_vectors[0] + position[1] * lattice_vectors[1] + position[2] * lattice_vectors[2];
				}
				molecule.atoms.push_back(Atom(elements[i], position));
				nextLine();
				skipWhitespace();
			}
		}

		Renderer::setMolecule(molecule);
		return;
	}
}
