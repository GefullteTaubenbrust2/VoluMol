#include "CubeReader.h"

#include <iostream>

#include "TextUtil.h"
#include "Molecule.h"
#include "Orbital.h"
#include "MolRenderer.h"
#include "Constants.h"

#include "../logic/TextReading.h"

namespace mol {
	extern CubeMap cubemap;
	extern std::vector<ContractedBasis> basis_set;
	extern std::vector<MolecularOrbital> mos;
}

namespace mol::Cub {
	void readFile(const std::string& path) {
		std::vector<std::string> file = flo::readFile(path);
		if (!file.size()) {
			std::cerr << "File not found or empty\n";
			return;
		}

		Molecule molecule;
		basis_set.clear();
		mos.clear();

		uint line = 2, offset = 0;
		std::string l = file[line];
		bool error = false;

		skipWhitespace(l, offset);
		int number_of_atoms = readInt(l, offset, error);
		if (error) {
			std::cerr << "Could not read number of atoms!\n";
			return;
		}
		molecule.atoms.resize(glm::abs(number_of_atoms));
		
		glm::vec3 origin;

		skipWhitespace(l, offset);
		origin.x = readFloat(l, offset, error) * a0_A;
		skipWhitespace(l, offset);
		origin.y = readFloat(l, offset, error) * a0_A;
		skipWhitespace(l, offset);
		origin.z = readFloat(l, offset, error) * a0_A;

		if (error) {
			std::cerr << "Could not read cubemap origin!\n";
			return;
		}

		++line;
		offset = 0;
		l = file[line];

		glm::ivec3 resolution;
		glm::mat3 axes;

		for (int i = 0; i < 3; ++i) {
			skipWhitespace(l, offset);
			resolution[i] = readInt(l, offset, error);

			if (error) {
				std::cerr << "Could not read cubemap resolution along " << ("xyz"[i]) << " axis!\n";
				return;
			}

			skipWhitespace(l, offset);
			axes[i].x = readFloat(l, offset, error) * a0_A;
			skipWhitespace(l, offset);
			axes[i].y = readFloat(l, offset, error) * a0_A;
			skipWhitespace(l, offset);
			axes[i].z = readFloat(l, offset, error) * a0_A;

			if (error) {
				std::cerr << "Could not read cubemap " << ("xyz"[i]) << " axis!\n";
				return;
			}

			++line;
			offset = 0;
			l = file[line];
		}
		
		glm::mat3 transform_matrix = glm::mat3(glm::normalize(axes[0]), glm::normalize(axes[1]), glm::normalize(axes[2]));
		cubemap.resize(resolution);
		cubemap.size = glm::vec3(glm::length(axes[0]) * resolution.x, glm::length(axes[1]) * resolution.y, glm::length(axes[2]) * resolution.z);

		for (int i = 0; i < number_of_atoms; ++i) {
			skipWhitespace(l, offset);
			molecule.atoms[i].Z = readInt(l, offset, error);

			if (error) {
				std::cerr << "Could not read atomic number!\n";
				return;
			}

			// Skip
			skipWhitespace(l, offset);
			float f = readFloat(l, offset, error);

			skipWhitespace(l, offset);
			molecule.atoms[i].position.x = readFloat(l, offset, error) * a0_A;
			skipWhitespace(l, offset);
			molecule.atoms[i].position.y = readFloat(l, offset, error) * a0_A;
			skipWhitespace(l, offset);
			molecule.atoms[i].position.z = readFloat(l, offset, error) * a0_A;

			molecule.atoms[i].position = (molecule.atoms[i].position - (origin)) * transform_matrix + glm::vec3(cubemap.size * 0.5) / glm::vec3(resolution);

			if (error) {
				std::cerr << "Could not read atom position!\n";
				return;
			}

			++line;
			offset = 0;
			l = file[line];
		}

		int number_of_ints = 0;
		if (number_of_atoms < 0) {
			skipWhitespace(l, offset);
			number_of_ints = readInt(l, offset, error);
			if (error) {
				std::cerr << "Could not read number of IDs!\n";
				return;
			}
		}
		skipWhitespace(l, offset);

		for (int i = 0; i < number_of_ints; ++i) {
			if (offset >= l.size()) {
				++line;
				if (line >= file.size()) {
					std::cerr << "Not enough data provided!\n";
					return;
				}
				offset = 0;
				l = file[line];

				skipWhitespace(l, offset);
			}

			int data = readInt(l, offset, error);
			skipWhitespace(l, offset);

			if (error) {
				std::cerr << "Error reading IDs!\n";
				return;
			}
		}

		const float volume_norm = glm::pow(a0_A, 1.5f);

		for (int i = 0; i < resolution.x * resolution.y * resolution.z; ++i) {
			if (offset >= l.size()) {
				++line;
				if (line >= file.size()) {
					std::cerr << "Not enough data provided for the given resolution!\n";
					return;
				}
				offset = 0;
				l = file[line];

				skipWhitespace(l, offset);
			} 

			int z = i % resolution.x;
			int y = i / resolution.x % resolution.y;
			int x = i / (resolution.x * resolution.y);
			int j = 4 * (x + resolution.x * (y + resolution.y * z));

			cubemap.texture.data[j] = readFloat(l, offset, error) / volume_norm;
			skipWhitespace(l, offset);

			cubemap.texture.data[j + 1] = 0.f;
			cubemap.texture.data[j + 2] = 0.f;
			cubemap.texture.data[j + 3] = 0.f;

			if (error) {
				std::cerr << "Error reading cubemap data!\n";
				return;
			}
		}

		std::cout << "Successfully read Gaussian Cube file!\n";

		Renderer::setMolecule(molecule);

		if (!cubemap.texture.id) cubemap.texture.createBuffer(GL_CLAMP_TO_BORDER, GL_LINEAR);
		else cubemap.texture.syncTexture();
	}
}