#include <iostream>

#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "GErrorHandler.h"

#include "FrameBuffer.h"

#include "Renderstate.h"
#include "Texture.h"

#include "Window.h"

namespace fgr {
	namespace Texture {
		TextureHandle dither0, dither1;
		void bindTextureID(uint id, TextureUnit unit) {
			graphics_check_external();
			glActiveTexture(UNIT_ENUM_TO_GL_UNIT(unit));
			glBindTexture(GL_TEXTURE_2D, id);
			graphics_check_error();
		}
	}

	void init_default_textures() {
		char d0[16] = {
			 0,  8,  2, 10,
			12,  4, 14,  6,
			 3, 11,  1,  9,
			15,  7, 13,  5,
		};
		char d1[16] = {
			 0,  8,  2, 10,
			 4, 12,  6, 14,
			11,  1,  9,  3,
			15,  5, 13,  7,
		};
		Texture::dither0.data.resize(64);
		Texture::dither1.data.resize(64);
		Texture::dither0.width = 4;
		Texture::dither0.height = 4;
		Texture::dither1.width = 4;
		Texture::dither1.height = 4;
		for (int i = 0; i < 16; ++i) {
			Texture::dither0.data[i * 4] = d0[i];
			Texture::dither0.data[i * 4 + 1] = d0[i];
			Texture::dither0.data[i * 4 + 2] = d0[i];
			Texture::dither0.data[i * 4 + 3] = 0;

			Texture::dither1.data[i * 4] = d1[i];
			Texture::dither1.data[i * 4 + 1] = d1[i];
			Texture::dither1.data[i * 4 + 2] = d1[i];
			Texture::dither1.data[i * 4 + 3] = 0;
		}

		Texture::dither0.createBuffer(GL_REPEAT, GL_NEAREST);
		Texture::dither1.createBuffer(GL_REPEAT, GL_NEAREST);
	}

	TextureHandle::TextureHandle(int width, int height, int bytes_per_pixel, u8* data) :
		width(width), height(height), data(width * height * bytes_per_pixel), bytes_per_pixel(bytes_per_pixel) {
		if (data) std::copy(data, data + width * height * bytes_per_pixel, TextureHandle::data.getPtr());
	}

	TextureHandle::TextureHandle(const TextureHandle& copy) {
		*this = copy;
	}

	void TextureHandle::operator=(const TextureHandle& other) {
		dispose();

		width = other.width;
		height = other.height;
		texture_format = other.texture_format;
		bytes_per_pixel = other.bytes_per_pixel;
		data = other.data;
	}

	void TextureHandle::loadFromFile(const std::string& path, bool flip_vertically) {
		stbi_set_flip_vertically_on_load(flip_vertically);
		unsigned char* loaded = stbi_load(path.data(), &width, &height, &bytes_per_pixel, 4);
		if (!loaded) {
			std::cout << "Failed to load the image '" << flip_vertically << "'\n";
			return;
		}
		bytes_per_pixel = 4;
		data.resize(width * height * bytes_per_pixel);
		std::copy(loaded, loaded + width * height * bytes_per_pixel, data.getPtr());
		stbi_image_free(loaded);
	}

	void TextureHandle::saveFile(const std::string& path, bool flip_vertically) {
		stbi_flip_vertically_on_write(flip_vertically);
		int success = stbi_write_png(path.data(), width, height, bytes_per_pixel, data.getPtr(), bytes_per_pixel * width);
		if (!success) std::cerr << "Failed to save a texture to '" << path << "'\n";
	}

	void TextureHandle::loadFromID(const int id, int texture_format) {
		graphics_check_external();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, id);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		int size = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &size);
		bytes_per_pixel = size / 2;

		TextureHandle::texture_format = texture_format;
		TextureHandle::id = id;

		data.resize(width * height * bytes_per_pixel);

		glGetTexImage(GL_TEXTURE_2D, 0, texture_format, GL_UNSIGNED_BYTE, data.getPtr());

		graphics_check_error();
	}

	void TextureHandle::createBuffer(int wrap, int filter, int texture_format) {
		graphics_check_external();

		TextureHandle::texture_format = texture_format;

		glGenTextures(1, &id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, id);

		glTexImage2D(GL_TEXTURE_2D, 0, texture_format, width, height, 0, texture_format, GL_UNSIGNED_BYTE, data.getPtr());
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

		graphics_check_error();
	}

	void TextureHandle::syncTexture() {
		graphics_check_external();

		int glwidth = 0, glheight = 0;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, id);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &glwidth);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &glheight);

		if (glwidth == width && glheight == height) {
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, texture_format, GL_UNSIGNED_BYTE, data.getPtr());
		}
		else {
			int wrap = 0, filter = 0;

			glTexImage2D(GL_TEXTURE_2D, 0, texture_format, width, height, 0, texture_format, GL_UNSIGNED_BYTE, data.getPtr());
		}

		graphics_check_error();
	}

	void TextureHandle::bindToUnit(const TextureUnit unit) {
		Texture::bindTextureID(id, unit);
	}

	bool TextureHandle::readRect(int x, int y, int width, int height, int format, u8* data) {
		graphics_check_external();
		bindToUnit(fgr::TextureUnit::misc);
		glReadPixels(x, y, width, height, texture_format, format, data);
		graphics_check_error();
		return false;
	}

	void TextureHandle::setRect(int x, int y, int width, int height, u8* data) {
		graphics_check_external();
		bindToUnit(fgr::TextureUnit::misc);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, texture_format, GL_UNSIGNED_BYTE, data);
		graphics_check_error();
	}

	void TextureHandle::dispose() {
		if (!window::graphicsInitialized()) return;

		graphics_check_external();
		glBindTexture(GL_TEXTURE_2D, 0);
		if (id) glDeleteTextures(1, &id);
		id = 0;
		graphics_check_error();
	}

	int TextureHandle::splitTexture(const TextureHandle& texture, int* output, const int width, const int height) {
		graphics_check_external();

		if (texture.width % width && texture.height % height) return 0;

		const int size = texture.width * texture.height / (width * height);
		glGenTextures(size, (GLuint*)output);

		flo::Array<char> data(4 * width * height);

		for (int x = 0; x < texture.width / width; ++x) {
			for (int y = 0; y < texture.height / height; ++y) {
				const int index = x + y * texture.width / width;
				glBindTexture(GL_TEXTURE_2D, output[index]);

				for (int yp = 0; yp < height; ++yp) {
					const int xa = x * width;
					const int ya = y * height + yp;
					std::copy(texture.data.getPtr() + (xa + (ya) * texture.width) * 4, texture.data.getPtr() + (xa + width + (ya) * texture.width) * 4, data.getPtr() + yp * width * 4);
				}

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.getPtr());

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
		}
		graphics_check_error();

		return size;
	}

	TextureHandle::~TextureHandle() {
		dispose();
	}

	ArrayTexture::ArrayTexture(int width, int height, int layer_count, int bytes_per_pixel) : width(width), height(height), layer_count(layer_count), bytes_per_pixel(bytes_per_pixel) {
		data.resize(width * height * layer_count * bytes_per_pixel);
	}

	ArrayTexture::ArrayTexture(const ArrayTexture& copy) {
		*this = copy;
	}

	void ArrayTexture::operator=(const ArrayTexture& other) {
		dispose();

		width = other.width;
		height = other.height;
		layer_count = other.layer_count;
		bytes_per_pixel = other.bytes_per_pixel;
		data = other.data;
	}

	void ArrayTexture::loadFromFile(const std::string& path, int layer, bool flip_vertically) {
		stbi_set_flip_vertically_on_load(flip_vertically);
		int lwidth = 0, lheight = 0, lbytes_per_pixel = 0;
		unsigned char* loaded = stbi_load(path.data(), &lwidth, &lheight, &lbytes_per_pixel, 4);
		if (!loaded) {
			std::cout << "Failed to load the image '" << flip_vertically << "'\n";
			return;
		}
		lbytes_per_pixel = 4;
		if (lwidth == width && lheight == height) {
			std::copy(loaded, loaded + width * height * bytes_per_pixel, data.getPtr() + width * height * bytes_per_pixel * layer);
		}
		stbi_image_free(loaded);
	}

	void ArrayTexture::loadFromID(const int id, int texture_format) {
		graphics_check_external();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, id);
		glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_HEIGHT, &height);
		glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_DEPTH, &layer_count);
		int size = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_BLUE_SIZE, &size);
		bytes_per_pixel = size / 2;

		ArrayTexture::texture_format = texture_format;
		ArrayTexture::id = id;

		data.resize(width * height * bytes_per_pixel * layer_count);

		glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, texture_format, GL_UNSIGNED_BYTE, data.getPtr());

		destroyFBO();

		graphics_check_error();
	}

	void ArrayTexture::createBuffer(int wrap, int filter, int texture_format) {
		graphics_check_external();

		ArrayTexture::texture_format = texture_format;

		glGenTextures(1, &id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, id);

		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, texture_format, width, height, layer_count, 0, texture_format, GL_UNSIGNED_BYTE, data.getPtr());

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, filter);

		destroyFBO();

		graphics_check_error();
	}

	void ArrayTexture::syncTexture() {
		graphics_check_external();

		int glwidth = 0, glheight = 0, gldepth = 0;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, id);
		glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_WIDTH, &glwidth);
		glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_HEIGHT, &glheight);
		glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_DEPTH, &gldepth);
		if (glwidth == width && glheight == height && gldepth == layer_count) {
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, layer_count, texture_format, GL_UNSIGNED_BYTE, data.getPtr());
		}
		else {
			int wrap = 0, filter = 0;

			glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_WRAP_S, &wrap);
			glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_MIN_FILTER, &filter);

			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			glDeleteTextures(1, &id);

			createBuffer(wrap, filter, texture_format);
		}

		destroyFBO();

		graphics_check_error();
	}

	FrameBuffer ArrayTexture::createFBO(int layer) {
		graphics_check_external();

		if (fbo_id) glDeleteFramebuffers(1, &fbo_id);

		glGenFramebuffers(1, &fbo_id);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, id, 0, layer);

		glGenRenderbuffers(1, &depth_rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		fbo_layer = layer;

		FrameBuffer _fbo;
		_fbo.fbo_id = fbo_id;
		_fbo.inited = true;
		_fbo.tex_width = width;
		_fbo.tex_height = height;
		_fbo.texture_id = id;
		_fbo.rbo = depth_rbo;
		_fbo.target.id = fbo_id;
		_fbo.target.bounds = glm::ivec4(0, 0, width, height);

		graphics_check_error();

		return _fbo;
	}

	void ArrayTexture::destroyFBO() {
		graphics_check_external();
		if (fbo_id) glDeleteFramebuffers(1, &fbo_id);
		fbo_id = 0;
		if (depth_rbo) glDeleteRenderbuffers(1, &depth_rbo);
		depth_rbo = 0;
		fbo_layer = -1;
		graphics_check_error();
	}

	void ArrayTexture::bindToUnit(const TextureUnit unit) {
		graphics_check_external();
		glActiveTexture(UNIT_ENUM_TO_GL_UNIT(unit));
		glBindTexture(GL_TEXTURE_2D_ARRAY, id);
		graphics_check_error();
	}

	bool ArrayTexture::readRect(int x, int y, int width, int height, int layer, u8* data) {
		if (x < 0 || y < 0 || x + width >= ArrayTexture::width || y + height >= ArrayTexture::height || layer < 0 || layer >= layer_count) return true;
		for (int yp = y; yp < y + height; ++yp) {
			std::copy(
				ArrayTexture::data.getPtr() + (layer * ArrayTexture::width * ArrayTexture::height + x + yp * ArrayTexture::width) * 4,
				ArrayTexture::data.getPtr() + (layer * ArrayTexture::width * ArrayTexture::height + x + width + yp * ArrayTexture::width) * 4,
				data + width * (yp - y) * 4);
		}
		return false;
	}

	void ArrayTexture::setRect(int x, int y, int width, int height, int layer, u8* data) {
		graphics_check_external();
		bindToUnit(fgr::TextureUnit::misc);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, x, y, layer, width, height, 1, texture_format, GL_UNSIGNED_BYTE, data);
		graphics_check_error();
	}

	void ArrayTexture::dispose() {
		if (!window::graphicsInitialized()) return;

		graphics_check_external();
		destroyFBO();
		glBindTexture(GL_TEXTURE_2D, 0);
		if (id) glDeleteTextures(1, &id);
		id = 0;
		graphics_check_error();
	}

	ArrayTexture::~ArrayTexture() {
		dispose();
	}

	TextureStorage::TextureStorage(int width, int height, int bytes_per_pixel) :
	ArrayTexture(width, height, 1, bytes_per_pixel) {
		allocators.push_back(flo::SpriteSheet(width, height));
	}

	void TextureStorage::addLayer() {
		graphics_check_external();
		allocators.push_back(flo::SpriteSheet(width, height));
		++layer_count;

		flo::Array<u8> new_data = flo::Array<u8>(layer_count * width * height * bytes_per_pixel);
		std::copy(data.getPtr(), data.getPtr() + (layer_count - 1) * width * height * bytes_per_pixel, new_data.getPtr());
		data.swap(new_data);

		if (id) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, id);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, texture_format, width, height, layer_count, 0, texture_format, GL_UNSIGNED_BYTE, data.getPtr());
		}
		if (fbo_id) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, id, 0, fbo_layer);
		}
		graphics_check_error();
	}

	SubTexture TextureStorage::addTexture(int width, int height, u8* data) {
		glm::ivec2 size = leave_free_edges ? glm::ivec2(width + 2, height + 2) : glm::ivec2(width, height);
		if (size.x > TextureStorage::width || size.y > TextureStorage::height) return SubTexture();
		u32 hash = INVALID_INDEX;
		int layer = 0;
		for (int i = 0; i < allocators.size(); ++i) {
			hash = allocators[i].allocate(size);
			if (hash != INVALID_INDEX) {
				layer = i;
				break;
			}
		}
		if (hash == INVALID_INDEX) {
			addLayer();
			layer = allocators.size() - 1;
			hash = allocators[layer].allocate(size);
		}
		SubTexture result;
		result.layer = layer;
		if (hash != INVALID_INDEX) {
			glm::ivec4 edge_bounds = allocators[layer].getBounds(hash);
			glm::ivec4 bounds = leave_free_edges ? edge_bounds + glm::ivec4(1, 1, -1, -1) : edge_bounds;
			if (data) {
				for (int i = 0; i < height; ++i) {
					const u32 index = (layer * (TextureStorage::width * TextureStorage::height) + (bounds.y + i) * TextureStorage::width + bounds.x) * bytes_per_pixel;
					std::copy(data + i * width * bytes_per_pixel, data + (i + 1) * width * bytes_per_pixel, TextureStorage::data.getPtr() + index);
				}
			}
			result.hash = hash;
		}
		return result;
	}

	SubTexture TextureStorage::addImage(const std::string& path, bool flip_vertically) {
		stbi_set_flip_vertically_on_load(flip_vertically);
		int lwidth = 0, lheight = 0, lbytes_per_pixel = 0;
		unsigned char* loaded = stbi_load(path.data(), &lwidth, &lheight, &lbytes_per_pixel, 4);
		if (!loaded) {
			std::cout << "Failed to load the image '" << flip_vertically << "'\n";
			return SubTexture();
		}
		SubTexture result;
		result = addTexture(lwidth, lheight, loaded);
		stbi_image_free(loaded);
		return result;
	}

	glm::ivec4 TextureStorage::getTextureBounds(SubTexture texture_unit) const {
		if (texture_unit.hash == INVALID_INDEX) return glm::ivec4(-1);
		return leave_free_edges ? allocators[texture_unit.layer].getBounds(texture_unit.hash) + glm::ivec4(1, 1, -1, -1) : allocators[texture_unit.layer].getBounds(texture_unit.hash);
	}

	glm::vec4 TextureStorage::getTextureBoundsNormalized(SubTexture texture_unit, bool vertically_flipped) const {
		if (texture_unit.hash == INVALID_INDEX) return glm::ivec4(-1);
		return getTextureCoordinates(leave_free_edges ? allocators[texture_unit.layer].getBounds(texture_unit.hash) + glm::ivec4(1, 1, -1, -1) : allocators[texture_unit.layer].getBounds(texture_unit.hash), vertically_flipped);
	}

	glm::vec4 TextureStorage::getTextureCoordinates(const glm::ivec4& bounds, bool vertically_flipped) const {
		glm::ivec4 b = vertically_flipped ? glm::ivec4(bounds.x, bounds.w, bounds.z, bounds.y) : bounds;
		return glm::vec4((float)b.x / (float)width, (float)b.y / (float)height, (float)b.z / (float)width, (float)b.w / (float)height);
	}

	void TextureStorage::free(SubTexture& texture_unit) {
		u32 layer = texture_unit.layer;
		if (layer >= allocators.size()) return;
		allocators[layer].free(texture_unit.hash);
		texture_unit.hash = INVALID_INDEX;
	}

	void TextureStorage::bindFrameBuffer(SubTexture texture) {
		graphics_check_external();
		if (!fbo_id) {
			glGenFramebuffers(1, &fbo_id);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
		graphics_check_error();
		glActiveTexture(UNIT_ENUM_TO_GL_UNIT(TextureUnit::misc));
		glBindTexture(GL_TEXTURE_2D_ARRAY, id);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, id, 0, texture.layer);
		graphics_check_error();

		if (!depth_rbo) {
			glGenRenderbuffers(1, &depth_rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);
		}

		glm::ivec4 bounds = getTextureBounds(texture);
		RenderTarget target;
		target.id = fbo_id;
		target.bounds = bounds;
		fbo_layer = texture.layer;

		graphics_check_error();

		target.bind();

		graphics_check_error();
	}

	void TextureStorage::clearFramebuffer(SubTexture texture, const glm::vec4& color) {
		graphics_check_external();
		
		RenderTarget t = RenderTarget::bound;
		bindFrameBuffer(texture);

		if (leave_free_edges) {
			glm::ivec4 bounds = getTextureBounds(texture) + glm::ivec4(-1, -1, 1, 1);
			glViewport(bounds.x, bounds.y, bounds.z - bounds.x, bounds.w - bounds.y);
		}

		setBlending(Blending::none);
		fgr::Shader::basic.setVec4(0, color);
		drawRectangle(glm::mat3(1.), fgr::Shader::basic);
		setBlending(Blending::linear);
		t.bind();

		graphics_check_error();
	}

	SubTexture::SubTexture() : 
	hash(INVALID_INDEX), layer(0) {
	}
}
