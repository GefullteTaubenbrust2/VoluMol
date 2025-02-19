#pragma once
#include "../graphics/3D/3D Renderer.h"
#include "Orbital.h"

namespace mol {
	fgr::Mesh generateIsosurface(CubeMap& cubemap, float isovalue, const glm::vec3& color, const glm::vec2& material_params, bool flip = false);
}