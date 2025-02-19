#pragma once
#include "../graphics/3D/3D Renderer.h"

using namespace fgr;

namespace mol {
	void generateIsosphere(Mesh& mesh, uint subdivisions);

	void generateCylinder(Mesh& mesh, uint resolution, float thickness, bool hard_cut);
}