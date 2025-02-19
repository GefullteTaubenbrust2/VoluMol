#include "Animation.h"

namespace fgr {
	Animation::Animation(TextureStorage* texture, const SubTexture& subtexture, const glm::ivec2& size) : texture(texture), subtexture(subtexture) {
		unique_subtex = texture->addTexture(size.x, size.y, nullptr);
		target_bounds = texture->getTextureBounds(unique_subtex);
		subtex_bounds = texture->getTextureBounds(subtexture);
	}

	void Animation::addFrame(const glm::ivec2& pos) {
		if (pos.x < subtex_bounds.x || pos.y < subtex_bounds.y || pos.x + target_bounds.z - target_bounds.x >= subtex_bounds.z || pos.y + target_bounds.w - target_bounds.y >= subtex_bounds.w) return;
		frames.push_back(pos);
	}

	void Animation::createFrames(const glm::ivec2& pos, int frame_count, bool vertical) {
		glm::ivec2 size = glm::ivec2(target_bounds.z - target_bounds.x, target_bounds.w - target_bounds.y);
		if (vertical) {
			for (int y = pos.y; y < pos.y + size.y * frame_count; y += size.y) {
				addFrame(glm::ivec2(pos.x, y));
			}
		}
		else {
			for (int x = pos.x; x < pos.x + size.x * frame_count; x += size.x) {
				addFrame(glm::ivec2(x, pos.y));
			}
		}
	}

	void Animation::setFrame(int frame) {
		if (current_frame != frame && frame >= 0 && frame < frames.size() && subtexture.hash != INVALID_INDEX) {
			glm::ivec2 pos = frames[frame];
			texture->loadFromID(texture->id);
			for (int y = target_bounds.y; y < target_bounds.w; ++y) {
				const int xmin = pos.x + subtex_bounds.x;
				const int xmax = pos.x + subtex_bounds.z;
				const int yp = y - target_bounds.y + subtex_bounds.y + pos.y;
				const int min_index = texture->bytes_per_pixel * ((subtexture.layer * texture->height + yp) * texture->width + xmin);
				const int max_index = min_index + texture->bytes_per_pixel * (xmax - xmin);
				const int target_start = texture->bytes_per_pixel * ((unique_subtex.layer * texture->height + y) * texture->width + target_bounds.x);
				std::copy(texture->data.getPtr() + min_index, texture->data.getPtr() + max_index, texture->data.getPtr() + target_start);
			}
			texture->syncTexture();
			current_frame = frame;
		}
	}

	void Animation::dispose() {
		texture->free(unique_subtex);
	}
}