#pragma once
#include "Texture.h"

namespace fgr {
	///<summary>
	/// A struct for handling animations within textures.
	///</summary>
	struct Animation {
		TextureStorage* texture;
		SubTexture subtexture, unique_subtex;
		glm::ivec4 target_bounds, subtex_bounds;
		std::vector<glm::ivec2> frames;
		int current_frame = -1;

		Animation(TextureStorage* texture, const SubTexture& subtexture, const glm::ivec2& size);

		void addFrame(const glm::ivec2& pos);

		void createFrames(const glm::ivec2& pos, int frame_count, bool vertical);

		void setFrame(int frame);

		void dispose();
	};
}