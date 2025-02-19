#include <iostream>

#include "Matrices.h"

namespace flo {
	glm::mat3 scale(const glm::mat3& matrix, glm::vec2 scale) {
		const glm::mat3 transform = glm::mat3(
			scale.x, 0.0, 0.0,
			0.0, scale.y, 0.0,
			0.0, 0.0, 1.0
			);
		return transform * matrix;
	}

	glm::mat3 scale(const glm::mat3& matrix, float f, glm::vec2 dir)
	{
		const glm::mat3 transform = glm::mat3(
			dir.x*dir.x * f + dir.y*dir.y, dir.x*dir.y * (f - 1.), 0.0,
			dir.x * dir.y * (f - 1.), dir.x*dir.x + dir.y*dir.y * f, 0.0,
			0.0, 0.0, 1.0
		);
		return transform * matrix;
	}

	glm::mat3 rotate(const glm::mat3& matrix, const float angle) {
		return rotate(matrix, std::cos(angle), std::sin(angle));
	}

	glm::mat3 rotate(const glm::mat3& matrix, const float cos, const float sin) {
		const glm::mat3 transform = glm::mat3(
			cos, sin, 0.0,
			-sin, cos, 0.0,
			0.0, 0.0, 1.0
		);
		return transform * matrix;
	}

	glm::mat3 translate(const glm::mat3& matrix, glm::vec2 offset) {
		const glm::mat3 transform = glm::mat3(
			1.0, 0.0, 0.0,
			0.0, 1.0, 0.0,
			offset.x, offset.y, 1.0
		);
		return transform * matrix;
	}

	glm::mat3 scale_and_translate(const glm::vec2& scale, const glm::vec2& offset) {
		return glm::mat3(
			scale.x, 0.0, 0.0,
			0.0, scale.y, 0.0,
			offset.x, offset.y, 1.0
		);
	}

	glm::mat3 scale_rotate_and_translate(const glm::vec2& scale, const glm::vec2& offset, const glm::vec2& direction) {
		return glm::mat3(
			scale.x * direction.x, direction.y * scale.x, 0.0,
			-direction.y * scale.y, scale.y * direction.x, 0.0,
			offset.x, offset.y, 1.0
		);
	}

	glm::mat3 generate_tsrt_matrix(const glm::vec2& ioffset, const glm::vec2& scale, const glm::vec2& direction, const glm::vec2& offset) {
		glm::vec2 io = scale * ioffset;
		return glm::mat3(
			scale.x * direction.x, direction.y * scale.x, 0.0,
			-direction.y * scale.y, scale.y * direction.x, 0.0,
			offset.x + io.x * direction.x - io.y * direction.y, offset.y + io.y * direction.x + io.x * direction.y, 1.0
		);
	}

	glm::mat3 setDepth(glm::mat3 matrix, float depth) {
		matrix[2].z = depth;
		return matrix;
	}

	ScaleMode::ScaleMode(ScaleModes mode, ScaleModes rounding, short base_dimension, float base_scale, char pixel_size) :
		mode(mode), rounding(rounding), base_dimension(base_dimension), base_scale(base_scale), pixel_size(pixel_size) {
	}

	glm::vec2 applyWindowScale(const int width, const int height, const ScaleMode& scalemode) {
		float _scale = 1.0;
		const int base_dim = scalemode.base_dimension;
		const float w = fixPixelScale(width, scalemode.pixel_size);
		const float h = fixPixelScale(height, scalemode.pixel_size);

		switch (scalemode.mode) {
		case scale_with_width:
			_scale = 1.0;
			break;
		case scale_with_height:
			_scale = h / w;
			break;
		case scale_with_diagonal:
			_scale = std::sqrt(w*w + h*h) / w;
			break;
		case scale_with_largest:
			if (width > height) _scale = 1.0;
			else _scale = h / w;
			break;
		case scale_with_smallest:
			if (width < height) _scale = 1.0;
			else _scale = h / w;
			break;
		case constant_scale:
			_scale = (float)base_dim / w;
			break;
		}

		const float scale_const = (float)base_dim / w;
		float sc = 0.0;
		switch (scalemode.rounding) {
		case round_to_smallest: 
			sc = std::floor(_scale / scale_const);
			_scale = (sc > 0 ? sc : 1) * scale_const;
			break;
		case round_to_largest:
			sc = std::ceil(_scale / scale_const);
			_scale = (sc > 0 ? sc : 1) * scale_const;
			break;
		case round_to_nearest:
			sc = std::ceil(_scale / scale_const - 0.5);
			_scale = (sc > 0 ? sc : 1) * scale_const;
			break;
		}

		glm::vec2 size = glm::vec2(_scale, _scale * w / h) * ((float)scalemode.base_scale);

		return size;
	}

	glm::vec2 centerWindowOrigin(const int width, const int height) {
		return glm::vec2(width % 2 ? 0.5 / (float)width : 0.0, height % 2 ? 0.5 / (float)height : 0.0);
	}

	int getPixelSize(const int width, const int height, const ScaleMode& scalemode) {
		float _scale = 1.0;
		const int base_dim = scalemode.base_dimension;
		const float w = fixPixelScale(width, scalemode.pixel_size);
		const float h = fixPixelScale(height, scalemode.pixel_size);

		switch (scalemode.mode) {
		case scale_with_width:
			_scale = 1.0;
			break;
		case scale_with_height:
			_scale = h / w;
			break;
		case scale_with_diagonal:
			_scale = std::sqrt(w * w + h * h) / w;
			break;
		case scale_with_largest:
			if (width > height) _scale = 1.0;
			else _scale = h / w;
			break;
		case scale_with_smallest:
			if (width < height) _scale = 1.0;
			else _scale = h / w;
			break;
		case constant_scale:
			return scalemode.base_scale * 0.5f * base_dim;
		}

		const float scale_const = (float)base_dim / w;
		float sc = _scale / scale_const;
		switch (scalemode.rounding) {
		case round_to_smallest:
			sc = std::floor(_scale / scale_const);
			break;
		case round_to_largest:
			sc = std::ceil(_scale / scale_const);
			break;
		case round_to_nearest:
			sc = std::ceil(_scale / scale_const - 0.5);
			break;
		}
		return sc > 0 ? sc : 1;
	}

	glm::vec2 fixToPixelPerfection(glm::vec2& pos, const int width, const int height) {
		glm::vec2 f = glm::vec2(2. / width, 2. / height);
		return glm::floor(pos / f) * f;
	}

	glm::vec4 getBounds(const glm::mat3& inverse_transform) {
		glm::vec2 bound_points[4] = {
			inverse_transform * glm::vec3(-1., -1., 1.),
			inverse_transform * glm::vec3(1., -1., 1.),
			inverse_transform * glm::vec3(1., 1., 1.),
			inverse_transform * glm::vec3(-1., 1., 1.),
		};
		glm::vec4 result = glm::vec4(bound_points[0].x, bound_points[0].y, bound_points[0].x, bound_points[0].y);
		for (int i = 1; i < 4; ++i) {
			if (bound_points[i].x < result.x) result.x = bound_points[i].x;
			else if (bound_points[i].x > result.z) result.z = bound_points[i].x;
			if (bound_points[i].y < result.y) result.y = bound_points[i].y;
			else if (bound_points[i].y > result.w) result.w = bound_points[i].y;
		}
		return result;
	}

	int fixPixelScale(int width, int pixel_size) {
		if (pixel_size <= 1) return width;
		return (width - (width % pixel_size));
	}
}