#include "FrameBuffer.h"

#include "Window.h"

#include <iostream>

#include "GErrorHandler.h"

namespace fgr {
	RenderTarget RenderTarget::bound;

	FrameBuffer::FrameBuffer(const FrameBuffer& copy) {
		*this = copy;
	}

	void FrameBuffer::operator=(const FrameBuffer& other) {
		dispose(true);

		if (other.inited) {
			init(other.tex_width, other.tex_height, other.format, other.wrap, other.filter);
		}
	}

	void FrameBuffer::init(const int width, const int height, const uint fmt, const uint wrap, const uint filter) {
		graphics_check_external();

		tex_width = width;
		tex_height = height;
		if (inited) dispose(true);
		inited = true;

		FrameBuffer::format = fmt;
		FrameBuffer::filter = filter;
		FrameBuffer::wrap = wrap;

		glGenFramebuffers(1, &fbo_id);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		target.id = fbo_id;
		target.bounds = glm::ivec4(0, 0, width, height);

		graphics_check_error();
	}

	void FrameBuffer::resize(const int width, const int height) {
		graphics_check_external();

		if (!inited) {
			return;
		}

		if (width != tex_width || height != tex_height) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

			glDeleteTextures(1, &texture_id);

			glGenTextures(1, &texture_id);
			glBindTexture(GL_TEXTURE_2D, texture_id);

			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

			glDeleteRenderbuffers(1, &rbo);
			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			target.id = fbo_id;
			target.bounds = glm::ivec4(0, 0, width, height);
		}

		tex_width = width;
		tex_height = height;

		graphics_check_error();
	}

	void FrameBuffer::resolve(FrameBuffer& ofbo) {
		graphics_check_external();

		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ofbo.fbo_id);
		glBlitFramebuffer(0, 0, tex_width, tex_height, 0, 0, ofbo.tex_width, ofbo.tex_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		unbind();
	}

	void FrameBuffer::bindContent(const TextureUnit unit) {
		graphics_check_external();

		glActiveTexture(UNIT_ENUM_TO_GL_UNIT(unit));
		glBindTexture(GL_TEXTURE_2D, texture_id);

		graphics_check_error();
	}

	void FrameBuffer::bind() {
		target.bind();
	}

	void FrameBuffer::unbind() {
		target.unbind();
	}

	void FrameBuffer::dispose(bool clearTexture) {
		if (!window::graphicsInitialized()) return;

		graphics_check_external();

		if (clearTexture && texture_id) {
			glDeleteTextures(1, &texture_id);
			texture_id = 0;
		}
		if (fbo_id) {
			glDeleteFramebuffers(1, &fbo_id);
			fbo_id = 0;
		}
		if (rbo) {
			glDeleteRenderbuffers(1, &rbo);
			rbo = 0;
		}
		inited = false;
		target = RenderTarget();

		graphics_check_error();
	}

	void FrameBuffer::clear(const glm::vec4& color) {
		graphics_check_external();

		RenderTarget t = RenderTarget::bound;
		bind();
		glClearColor(color.r, color.g, color.b, color.a);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		t.bind();

		graphics_check_error();
	}

	FrameBuffer::~FrameBuffer() {
		dispose(true);
	}

	FrameBufferMS::FrameBufferMS(const FrameBufferMS& copy) {
		*this = copy;
	}

	void FrameBufferMS::operator=(const FrameBufferMS& other) {
		dispose();

		if (other.inited) {
			init(other.tex_width, other.tex_height, other.format);
		}
	}

	void FrameBufferMS::init(const int width, const int height, const uint format) {
		graphics_check_external();

		tex_width = width;
		tex_height = height;
		if (inited) dispose();
		inited = true;

		FrameBufferMS::format = format;

		glGenFramebuffers(1, &fbo_id);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_id);

		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, format, width, height, GL_TRUE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture_id, 0);

		glGenTextures(1, &depth_tex_id);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depth_tex_id);

		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT, width, height, GL_TRUE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth_tex_id, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		target.id = fbo_id;
		target.bounds = glm::ivec4(0, 0, width, height);

		graphics_check_error();
	}

	void FrameBufferMS::resolve(const uint output_fbo) {
		graphics_check_external();

		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, output_fbo);
		glBlitFramebuffer(0, 0, tex_width, tex_height, 0, 0, tex_width, tex_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		graphics_check_error();

		unbind();
	}

	void FrameBufferMS::resize(const int width, const int height) {
		graphics_check_external();

		if (!inited) {
			return;
		}

		if (width != tex_width || height != tex_height) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

			glDeleteTextures(1, &texture_id);

			glGenTextures(1, &texture_id);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_id);

			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, format, width, height, GL_TRUE);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture_id, 0);

			glDeleteTextures(1, &depth_tex_id);

			glGenTextures(1, &depth_tex_id);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depth_tex_id);

			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT, width, height, GL_TRUE);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth_tex_id, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			target.id = fbo_id;
			target.bounds = glm::ivec4(0, 0, width, height);
		}

		tex_width = width;
		tex_height = height;

		graphics_check_error();
	}

	void FrameBufferMS::bind() {
		target.bind();
	}

	void FrameBufferMS::unbind() {
		target.unbind();
	}

	void FrameBufferMS::dispose() {
		if (!window::graphicsInitialized()) return;
	
		graphics_check_external();

		if (texture_id) {
			glDeleteTextures(1, &texture_id);
			texture_id = 0;
		}
		if (fbo_id) {
			glDeleteFramebuffers(1, &fbo_id);
			fbo_id = 0;
		}
		if (depth_tex_id) {
			glDeleteTextures(1, &depth_tex_id);
			depth_tex_id = 0;
		}

		target = RenderTarget();

		inited = false;

		graphics_check_error();
	}

	void FrameBufferMS::clear(const glm::vec4& color) {
		graphics_check_external();

		RenderTarget t = RenderTarget::bound;
		bind();
		glClearColor(color.r, color.g, color.b, color.a);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		t.bind();

		graphics_check_error();
	}

	FrameBufferMS::~FrameBufferMS() {
		dispose();
	}

	void RenderTarget::bind() {
		graphics_check_external();
		RenderTarget::bound = *this;
		glBindFramebuffer(GL_FRAMEBUFFER, id);
		glViewport(bounds.x, bounds.y, bounds.z - bounds.x, bounds.w - bounds.y);
		graphics_check_error();
	}

	void RenderTarget::unbind() {
		graphics_check_external();
		window::framebuffer.bind();
		graphics_check_error();
	}

	void RenderTarget::clear(const glm::vec4& color, bool clear_depth) {
		graphics_check_external();
		glBindFramebuffer(GL_FRAMEBUFFER, id);
		glClearColor(color.r, color.g, color.b, color.a);
		if (clear_depth) glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | (GL_DEPTH_BUFFER_BIT * clear_depth));
		window::framebuffer.bind();
		graphics_check_error();
	}

	MultiFrameBuffer::MultiFrameBuffer(const MultiFrameBuffer& copy) {
		*this = copy;
	}

	void MultiFrameBuffer::operator=(const MultiFrameBuffer& other) {
		dispose();

		if (other.inited) {
			textures = other.textures;
			init(other.tex_width, other.tex_height, other.wrap, other.filter);
		}
	}

	void MultiFrameBuffer::allocateAttachments(uint count) {
		if (inited) return;

		textures.resize(count);

		for (uint& i : textures) i = 0;
	}

	void MultiFrameBuffer::init(const int width, const int height, const uint wrap, const uint filter) {
		graphics_check_external();

		tex_width = width;
		tex_height = height;
		if (inited) dispose();
		inited = true;

		MultiFrameBuffer::filter = filter;
		MultiFrameBuffer::wrap = wrap;

		glGenFramebuffers(1, &fbo_id);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

		flo::Array<uint> attachments(textures.size());

		for (int i = 0; i < textures.size(); ++i) {
			glGenTextures(1, &textures[i]);
			glBindTexture(GL_TEXTURE_2D, textures[i]);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);

			attachments[i] = GL_COLOR_ATTACHMENT0 + i;
		}

		glDrawBuffers(textures.size(), attachments.getPtr()); 

		glGenTextures(1, &depth_texture);

		glBindTexture(GL_TEXTURE_2D, depth_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		target.id = fbo_id;
		target.bounds = glm::ivec4(0, 0, width, height);

		graphics_check_error();
	}

	void MultiFrameBuffer::resize(const int width, const int height) {
		graphics_check_external();

		if (!inited) {
			return;
		}

		if (width != tex_width || height != tex_height) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

			for (int i = 0; i < textures.size(); ++i) {
				glDeleteTextures(1, &textures[i]);

				glGenTextures(1, &textures[i]);
				glBindTexture(GL_TEXTURE_2D, textures[i]);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);
			}

			glDeleteTextures(1, &depth_texture);

			glGenTextures(1, &depth_texture);
			glBindTexture(GL_TEXTURE_2D, depth_texture);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			target.id = fbo_id;
			target.bounds = glm::ivec4(0, 0, width, height);
		}

		tex_width = width;
		tex_height = height;

		graphics_check_error();
	}

	void MultiFrameBuffer::bindContent(uint attachment, const TextureUnit unit) {
		graphics_check_external();

		glActiveTexture(UNIT_ENUM_TO_GL_UNIT(unit));
		glBindTexture(GL_TEXTURE_2D, textures[attachment]);

		graphics_check_error();
	}

	void MultiFrameBuffer::bindDepthTexture(const TextureUnit unit) {
		graphics_check_external();

		glActiveTexture(UNIT_ENUM_TO_GL_UNIT(unit));
		glBindTexture(GL_TEXTURE_2D, depth_texture);

		graphics_check_error();
	}

	void MultiFrameBuffer::bind() {
		target.bind();
	}

	void MultiFrameBuffer::unbind() {
		target.unbind();
	}

	void MultiFrameBuffer::dispose() {
		if (!window::graphicsInitialized()) return;

		graphics_check_external();

		if (fbo_id) {
			glDeleteFramebuffers(1, &fbo_id);
			fbo_id = 0;
		}
		if (depth_texture) {
			glDeleteTextures(1, &depth_texture);
			depth_texture = 0;
		}
		if (textures.size()) {
			glDeleteTextures(textures.size(), textures.data());
			textures.clear();
		}

		inited = false;
		target = RenderTarget();

		graphics_check_error();
	}

	void MultiFrameBuffer::clear(const glm::vec4& color) {
		graphics_check_external();

		RenderTarget t = RenderTarget::bound;
		bind();
		glClearColor(color.r, color.g, color.b, color.a);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		t.bind();

		graphics_check_error();
	}

	MultiFrameBuffer::~MultiFrameBuffer() {
		dispose();
	}
}
