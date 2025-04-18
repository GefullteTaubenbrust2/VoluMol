#include "CubeReader.h"

#include <iostream>

#include "TextUtil.h"
#include "Molecule.h"
#include "Orbital.h"
#include "MolRenderer.h"
#include "Constants.h"

namespace mol {
	extern CubeMap cubemap;
	extern std::vector<ContractedBasis> basis_set;
	extern std::vector<MolecularOrbital> mos;
}

namespace mol::Cub {
	using namespace FileReader;

	void readFile() {
		Molecule molecule;
		basis_set.clear();
		mos.clear();

		setLineNumber(2);

		bool error = false;

		skipWhitespace();
		int number_of_atoms = readInt(error);
		if (error) {
			throwError("Could not read number of atoms");
			return;
		}
		molecule.atoms.resize(glm::abs(number_of_atoms));
		
		glm::vec3 origin;

		skipWhitespace();
		origin.x = readFloat(error) * a0_A;
		skipWhitespace();
		origin.y = readFloat(error) * a0_A;
		skipWhitespace();
		origin.z = readFloat(error) * a0_A;

		if (error) {
			throwError( "Could not read cubemap origin");
			return;
		}

		nextLine();

		glm::ivec3 resolution;
		glm::mat3 axes;

		for (int i = 0; i < 3; ++i) {
			skipWhitespace();
			resolution[i] = readInt(error);

			if (error) {
				throwError(std::string("Could not read cubemap resolution along ") + "xyz"[i] + " axis");
				return;
			}

			skipWhitespace();
			axes[i].x = readFloat(error) * a0_A;
			skipWhitespace();
			axes[i].y = readFloat(error) * a0_A;
			skipWhitespace();
			axes[i].z = readFloat(error) * a0_A;

			if (error) {
				throwError(std::string("Could not read cubemap ") + "xyz"[i] + " axis");
				return;
			}

			nextLine();
		}
		
		glm::mat3 transform_matrix = glm::mat3(glm::normalize(axes[0]), glm::normalize(axes[1]), glm::normalize(axes[2]));
		cubemap.resize(resolution);
		cubemap.size = glm::vec3(glm::length(axes[0]) * resolution.x, glm::length(axes[1]) * resolution.y, glm::length(axes[2]) * resolution.z);

		for (int i = 0; i < molecule.atoms.size(); ++i) {
			skipWhitespace();
			molecule.atoms[i].Z = readInt(error);

			if (error) {
				throwError( "Could not read atomic number");
				return;
			}

			// Skip
			skipWhitespace();
			float f = readFloat(error);

			skipWhitespace();
			molecule.atoms[i].position.x = readFloat(error) * a0_A;
			skipWhitespace();
			molecule.atoms[i].position.y = readFloat(error) * a0_A;
			skipWhitespace();
			molecule.atoms[i].position.z = readFloat(error) * a0_A;

			molecule.atoms[i].position = (molecule.atoms[i].position - (origin)) * transform_matrix + glm::vec3(cubemap.size * 0.5) / glm::vec3(resolution);

			if (error) {
				throwError( "Could not read atom position");
				return;
			}

			nextLine();
		}

		int number_of_ints = 0;
		if (number_of_atoms < 0) {
			skipWhitespace();
			number_of_ints = readInt(error);
			if (error) {
				throwError( "Could not read number of IDs");
				return;
			}
		}
		skipWhitespace();

		for (int i = 0; i < number_of_ints; ++i) {
			if (endOfLine()) {
				nextLine();
				if (endOfFile()) {
					throwError( "Not enough data provided");
					return;
				}

				skipWhitespace();
			}

			int data = readInt(error);
			skipWhitespace();

			if (error) {
				throwError( "Error reading IDs");
				return;
			}
		}

		const float volume_norm = glm::pow(a0_A, 1.5f);

		for (int i = 0; i < resolution.x * resolution.y * resolution.z; ++i) {
			if (endOfLine()) {
				nextLine();
				if (endOfFile()) {
					throwError( "Not enough data provided for the given resolution");
					return;
				}

				skipWhitespace();
			}

			int z = i % resolution.x;
			int y = i / resolution.x % resolution.y;
			int x = i / (resolution.x * resolution.y);
			int j = 4 * (x + resolution.x * (y + resolution.y * z));

			cubemap.texture.data[j] = readFloat(error) / volume_norm;
			skipWhitespace();

			cubemap.texture.data[j + 1] = 0.f;
			cubemap.texture.data[j + 2] = 0.f;
			cubemap.texture.data[j + 3] = 0.f;

			if (error) {
				throwError( "Error reading cubemap data");
				return;
			}
		}

		std::cout << "Successfully read Gaussian Cube file!\n";

		Renderer::setMolecule(molecule);

		if (!cubemap.texture.id) cubemap.texture.createBuffer(GL_CLAMP_TO_BORDER, GL_LINEAR);
		else cubemap.texture.syncTexture();
	}
}