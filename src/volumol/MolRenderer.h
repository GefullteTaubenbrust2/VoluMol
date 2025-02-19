#pragma once
#include "Molecule.h"
#include "Orbital.h"

namespace mol::Renderer {
	void init();

	void updateSettings(const RenderProperties& settings);

	void setMolecule(const Molecule& molecule);

	void setVolumetric();

	void setIsosurface();

	Atom getAtom(uint atom);

	void setTransform(const glm::mat4& transform);

	glm::mat4 getTransform(uint atom0, uint atom1, uint atom2, const glm::vec3 position0, const glm::vec3 dir01, const glm::vec3 dir02);

	void orientCamera(const glm::vec3& position, const glm::vec3& direction);

	void renderFrame(uint width, uint height);

	void saveImage(const std::string& path, uint width, uint height);
}
