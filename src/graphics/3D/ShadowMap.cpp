#include "ShadowMap.h"
#include "../GErrorHandler.h"
#include "../Window.h"

namespace fgr {
	Shader shadowmap_shader;

	CascadedShadowMap::CascadedShadowMap(uint levels, uint resolution, float distance) : levels(levels), resolution(resolution), distance(distance) {
		
	}

	CascadedShadowMap::CascadedShadowMap(const CascadedShadowMap& copy) {
		*this = copy;
	}

	void CascadedShadowMap::operator=(const CascadedShadowMap& other) {
		if (&other == this) return;

		dispose();

		resolution = other.resolution;
		distance = other.distance;
		levels = other.levels;
		views = other.views;

		if (other.texture_id) init();
	}

	void CascadedShadowMap::init() {
		views.resize(levels);
		view_fractions.resize(levels);
		if (levels) view_fractions[0] = 0.f;
		for (int i = 1; i < levels; ++i) {
			view_fractions[i] = view_fractions[i - 1] + 2.0f * (float)i * distance / (levels * levels + levels);
		}
		view_fractions.push_back(distance);

		graphics_check_external();
		
		glGenTextures(1, &texture_id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);

		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, resolution, resolution, levels, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

		fbos.resize(levels);
		for (int i = 0; i < levels; ++i) {
			glGenFramebuffers(levels, &fbos[i].id);
			glBindFramebuffer(GL_FRAMEBUFFER, fbos[i].id);
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_id, 0, i);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			fbos[i].bounds = glm::ivec4(0, 0, resolution, resolution);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if (!shadowmap_shader.shader_program) {
			shadowmap_shader = Shader("shaders/volumol/shadow.vert", "shaders/volumol/shadow.frag", std::vector<std::string>{"model", "projection"});
			shadowmap_shader.compile();
		}

		graphics_check_error();
	}

	void CascadedShadowMap::fitFrustum(const View& perspective, glm::vec3 sun_position, float depth_tolerance, float total_depth) {
		view_depths.resize(view_fractions.size());
		for (int i = 0; i < view_depths.size(); ++i) {
			view_depths[i] = view_fractions[i] * total_depth;
		}

		sun_position = glm::normalize(sun_position);

		for (int i = 0; i < levels; ++i) {
			glm::vec3 corners[8] = {
				glm::vec3(-1., -1., -1.),
				glm::vec3( 1., -1., -1.),
				glm::vec3( 1.,  1., -1.),
				glm::vec3(-1.,  1., -1.),
				glm::vec3(-1., -1.,  1.),
				glm::vec3( 1., -1.,  1.),
				glm::vec3( 1.,  1.,  1.),
				glm::vec3(-1.,  1.,  1.),
			};

			glm::mat4 view_inverse = glm::inverse(perspective.projection * perspective.view);

			for (int j = 0; j < 8; ++j) {
				glm::vec4 p = view_inverse * glm::vec4(corners[j], 1.);
				corners[j] = p / p.w;
			}

			for (int j = 0; j < 4; ++j) {
				glm::vec3 p0 = corners[j];
				glm::vec3 p1 = corners[j + 4];
				corners[j]     = p0 + (p1 - p0) * view_fractions[i];
				corners[j + 4] = p0 + (p1 - p0) * view_fractions[i + 1];
			}

			// Basis vectors for light space
			glm::vec3 right = glm::normalize(corners[4] + corners[6] - corners[0] - corners[2]);
			glm::vec3 up = glm::normalize(glm::cross(right, sun_position));
			right = glm::normalize(glm::cross(up, -sun_position));

			// In light space
			float min_x =  1000000.f;
			float max_x = -1000000.f;
			float min_y =  1000000.f;
			float max_y = -1000000.f;
			float min_z =  1000000.f;
			float max_z = -1000000.f;

			for (int j = 0; j < 8; ++j) {
				glm::vec3 c = corners[j];
				float x = glm::dot(c,  right);
				float y = glm::dot(c,  up);
				float z = glm::dot(c, -sun_position);

				if (x < min_x) min_x = x;
				if (x > max_x) max_x = x;
				if (y < min_y) min_y = y;
				if (y > max_y) max_y = y;
				if (z < min_z) min_z = z;
				if (z > max_z) max_z = z;
			}

			min_z -= depth_tolerance;

			glm::mat4 view_matrix = glm::inverse(glm::mat4{
				glm::vec4(right, 0.),
				glm::vec4(up, 0.),
				glm::vec4(-sun_position, 0.),
				glm::vec4(0., 0., 0., 1.),
			});

			glm::mat4 projection_matrix = glm::mat4{
				glm::vec4(2.f / (max_x - min_x), 0., 0., 0.),
				glm::vec4(0., 2.f / (max_y - min_y), 0., 0.),
				glm::vec4(0., 0., 2.f / (max_z - min_z), 0.),
				glm::vec4(-2. * min_x / (max_x - min_x) - 1., -2. * min_y / (max_y - min_y) - 1., -2. * min_z / (max_z - min_z) - 1., 1.),
			};

			views[i] = projection_matrix * view_matrix;
		}
	}

	void CascadedShadowMap::fitScene(const std::vector<glm::vec3>& positions, glm::vec3 sun_position, float tolerance) {
		levels = 1;

		sun_position = glm::normalize(sun_position);
		view_depths = { 0.f, 100000000000.f };

		glm::vec3 right = glm::normalize(sun_position.z * sun_position.z > 0.5 ? glm::vec3(1.0, 0.0, 0.0) : glm::vec3(0.0, 0.0, 1.0));
		glm::vec3 up = glm::normalize(glm::cross(right, sun_position));
		right = glm::normalize(glm::cross(up, -sun_position));

		// In light space
		float min_x = 1000000.f;
		float max_x = -1000000.f;
		float min_y = 1000000.f;
		float max_y = -1000000.f;
		float min_z = 1000000.f;
		float max_z = -1000000.f;

		for (int i = 0; i < positions.size(); ++i) {
			glm::vec3 c = positions[i];
			float x = glm::dot(c, right);
			float y = glm::dot(c, up);
			float z = glm::dot(c, -sun_position);

			if (x < min_x) min_x = x;
			if (x > max_x) max_x = x;
			if (y < min_y) min_y = y;
			if (y > max_y) max_y = y;
			if (z < min_z) min_z = z;
			if (z > max_z) max_z = z;
		}

		min_x -= tolerance;
		max_x += tolerance;
		min_y -= tolerance;
		max_y += tolerance;
		min_z -= tolerance;
		max_z += tolerance;

		glm::mat4 view_matrix = glm::inverse(glm::mat4{
			glm::vec4(right, 0.),
			glm::vec4(up, 0.),
			glm::vec4(-sun_position, 0.),
			glm::vec4(0., 0., 0., 1.),
		});

		glm::mat4 projection_matrix = glm::mat4{
			glm::vec4(2.f / (max_x - min_x), 0., 0., 0.),
			glm::vec4(0., 2.f / (max_y - min_y), 0., 0.),
			glm::vec4(0., 0., 2.f / (max_z - min_z), 0.),
			glm::vec4(-2. * min_x / (max_x - min_x) - 1., -2. * min_y / (max_y - min_y) - 1., -2. * min_z / (max_z - min_z) - 1., 1.),
		};

		views[0] = projection_matrix * view_matrix;
	}

	void CascadedShadowMap::clear() {
		for (int i = 0; i < levels; ++i) {
			fbos[i].bind();

			graphics_check_external();

			glClearDepth(1.0);
			glClear(GL_DEPTH_BUFFER_BIT);

			graphics_check_error();
		}
	}

	void CascadedShadowMap::drawShadows(Mesh& mesh) {
		for (int i = 0; i < levels; ++i) {
			fbos[i].bind();

			shadowmap_shader.setMat4(0, mesh.model_matrix);
			shadowmap_shader.setMat4(1, views[i]);

			mesh.render(shadowmap_shader, false, true);

			fbos[i].unbind();
		}
	}

	void CascadedShadowMap::bindUniforms(Shader& shader, uint first_uniform, TextureUnit unit) {
		glActiveTexture(UNIT_ENUM_TO_GL_UNIT(unit));
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
		shader.setInt(first_uniform, unit);
		shader.setMat4Array(first_uniform + 1, views.data(), levels);
		shader.setFloatArray(first_uniform + 2, view_depths.data(), levels + 1);
	}

	void CascadedShadowMap::dispose() {
		if (!window::graphicsInitialized()) return;

		graphics_check_external();

		if (texture_id) {
			glBindTexture(GL_TEXTURE_2D, 0);
			glDeleteTextures(1, &texture_id);
			texture_id = 0;
			for (int i = 0; i < fbos.size(); ++i) {
				glDeleteFramebuffers(levels, &fbos[i].id);
			}
			fbos.clear();
		}

		graphics_check_error();
	}

	CascadedShadowMap::~CascadedShadowMap() {
		dispose();
	}
}
