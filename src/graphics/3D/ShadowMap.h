#pragma once
#include "3D Renderer.h"
#include "../FrameBuffer.h"

namespace fgr {
	struct CascadedShadowMap {
	private:
		std::vector<glm::mat4> views;
		std::vector<float> view_fractions;
		std::vector<float> view_depths;

		uint texture_id = 0;
		std::vector<RenderTarget> fbos;

		float distance = 0.1f;
		uint levels = 3;
		uint resolution = 1024;

	public:
		CascadedShadowMap() = default;

		CascadedShadowMap(uint levels, uint resolution = 1024, float distance = 0.5f);

		CascadedShadowMap(const CascadedShadowMap& copy);

		void operator=(const CascadedShadowMap& other);

		void init();

		void fitFrustum(const View& perspective, glm::vec3 sun_position, float depth_tolerance, float total_depth);

		void fitScene(const std::vector<glm::vec3>& positions, glm::vec3 sun_position, float tolerance);

		void clear();

		void drawShadows(Mesh& mesh);

		void bindUniforms(Shader& shader, uint first_uniform, TextureUnit unit = TextureUnit::texture1);

		void dispose();

		~CascadedShadowMap();
	};
}