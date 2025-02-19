#include "MathUtil.h"

namespace flo{
	float cross2D(glm::vec2 a, glm::vec2 b) {
		return a.x * b.y - b.x * a.y;
	}

	glm::vec2 skew(glm::vec2 v) {
		return glm::vec2(v.y, -v.x);
	}
}