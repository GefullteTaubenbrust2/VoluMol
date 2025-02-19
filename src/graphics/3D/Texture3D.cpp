#include "Texture3D.h"
#include "../GErrorHandler.h"
#include "../Window.h"
#include "../FrameBuffer.h"

namespace fgr {
	TextureHandle3D::TextureHandle3D(uint width, uint height, uint depth, float* _data) : width(width), height(height), depth(depth), data(4 * width * height * depth) {
		std::copy(_data, _data + 4 * width * height * depth, data.getPtr());
	}

	TextureHandle3D::TextureHandle3D(const TextureHandle3D& copy) {
		*this = copy;
	}

	void TextureHandle3D::operator=(const TextureHandle3D& other) {
		dispose();

		width = other.width;
		height = other.height;
		depth = other.depth;

		data = other.data;

		if (other.id) createBuffer(GL_CLAMP_TO_BORDER, GL_LINEAR);
	}

	void TextureHandle3D::resize(uint _width, uint _height, uint _depth) {
		if (width == _width && height == _height && depth == _depth) return;

		width = _width;
		height = _height;
		depth = _depth;
		data.resize(4 * width * height * depth);

		if (id) {
			graphics_check_external();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_3D, id);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, width, height, depth, 0, GL_RGBA, GL_FLOAT, data.getPtr());

			if (fbo) {
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_TEXTURE_3D, id, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			graphics_check_error();
		}
	}

	void TextureHandle3D::loadFromID(const int id) {
		graphics_check_external();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, id);
		glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &height);
		glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &depth);

		TextureHandle3D::id = id;

		data.resize(4 * width * height * depth);

		glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, data.getPtr());

		if (fbo) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, id, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		graphics_check_error();
	}

	void TextureHandle3D::createBuffer(int wrap, int filter) {
		graphics_check_external();

		if (id) return;

		glGenTextures(1, &id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, id);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, width, height, depth, 0, GL_RGBA, GL_FLOAT, data.getPtr());

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);

		graphics_check_error();
	}

	void TextureHandle3D::syncTexture() {
		graphics_check_external();

		int glwidth = 0, glheight = 0, gldepth = 0;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, id);
		glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &glwidth);
		glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &glheight);
		glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &gldepth);

		if (glwidth == width && glheight == height && gldepth == depth) {
			glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RGBA, GL_FLOAT, data.getPtr());
		}
		else {
			int wrap = 0, filter = 0;

			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, width, height, depth, 0, GL_RGBA, GL_FLOAT, data.getPtr());
		}

		if (fbo) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, id, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		graphics_check_error();
	}

	RenderTarget TextureHandle3D::createFrameBuffer() {
		if (!id) return RenderTarget();
		graphics_check_external();

		if (fbo) glDeleteFramebuffers(1, &fbo);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, id, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		graphics_check_error();

		RenderTarget result;
		result.id = fbo;
		result.bounds = glm::ivec4(0, 0, width, height);
		return result;
	}

	void TextureHandle3D::bindToUnit(const TextureUnit unit) {
		graphics_check_external();
		glActiveTexture(UNIT_ENUM_TO_GL_UNIT(unit));
		glBindTexture(GL_TEXTURE_3D, id);
		graphics_check_error();
	}

	void TextureHandle3D::setCuboid(int x, int y, int z, int width, int height, int depth, float* data) {
		graphics_check_external();
		bindToUnit(fgr::TextureUnit::misc);
		glTexSubImage3D(GL_TEXTURE_3D, 0, x, y, z, width, height, depth, GL_RGBA16F, GL_FLOAT, data);
		graphics_check_error();
	}

	void TextureHandle3D::dispose() {
		if (!window::graphicsInitialized()) return;

		data.~Array();
		if (!window::graphicsInitialized()) return;

		graphics_check_external();
		glBindTexture(GL_TEXTURE_3D, 0);
		if (id) glDeleteTextures(1, &id);
		id = 0;
		if (fbo) glDeleteFramebuffers(1, &fbo);
		fbo = 0;
		graphics_check_error();
	}

	TextureHandle3D::~TextureHandle3D() {
		dispose();
	}
}
