#include "Blur.h"

#include "Renderstate.h"

namespace fgr {
	Shader blur5_shader, blur10_shader, scale_shader;

	BlurBuffer::BlurBuffer(float blur_radius) : 
	blur_radius(blur_radius) {
	}

	BlurBuffer::BlurBuffer(const BlurBuffer& copy) {
		*this = copy;
	}

	void BlurBuffer::operator=(const BlurBuffer& other) {
		dispose();

		blur_radius = other.blur_radius;
		increased_blur = other.increased_blur;
		if (other.io_fbo.inited) init();
	}

	void BlurBuffer::init() {
		if (!blur5_shader.loaded) blur5_shader.loadFromFile("shaders/textured.vert", "shaders/blur5.frag", std::vector<std::string>{
			"texture", "direction"
		});
		if (!blur10_shader.loaded) blur10_shader.loadFromFile("shaders/textured.vert", "shaders/blur10.frag", std::vector<std::string>{
			"texture", "direction"
		});
		if (!scale_shader.loaded) scale_shader.loadFromFile("shaders/textured.vert", "shaders/downscale.frag", std::vector<std::string>{
			"texture", "scale_factor"
		});
		io_fbo.init(10, 10, GL_RGBA16, GL_MIRRORED_REPEAT, GL_LINEAR);
		util_fbo.init(10, 10, GL_RGBA16, GL_MIRRORED_REPEAT, GL_LINEAR);
	}

	void BlurBuffer::blur(const FrameBuffer& fbo) {
		blur(fbo.texture_id, glm::ivec2(fbo.tex_width, fbo.tex_height));
	}

	void BlurBuffer::blur(uint texture_id, const glm::ivec2 tex_size) {
		float radius = increased_blur ? 10.f : 5.f;

		float downsample = glm::max(blur_radius / radius, 1.f);

		float radius_factor = glm::min(1.f, blur_radius / radius);

		glm::ivec2 size = (glm::vec2)tex_size / downsample;

		RenderTarget rt = RenderTarget::bound;

		setBlending(Blending::none);

		io_fbo.resize(size.x, size.y);
		util_fbo.resize(size.x, size.y);
		
		util_fbo.clear(glm::vec4(0.));
		
		fgr::Texture::bindTextureID(texture_id, TextureUnit::misc);

		if (downsample > 1.f) {
			io_fbo.clear(glm::vec4(0.));
			io_fbo.bind();

			scale_shader.setInt(0, fgr::TextureUnit::misc);
			scale_shader.setVec2(1, glm::vec2(downsample));
			drawRectangle(glm::mat3(1.), scale_shader);
			io_fbo.bindContent(TextureUnit::misc);
		}

		util_fbo.bind();

		Shader& blur_shader = increased_blur ? blur10_shader : blur5_shader;

		blur_shader.setInt(0, TextureUnit::misc);
		blur_shader.setVec2(1, glm::vec2(radius_factor / (float)util_fbo.tex_width, 0.));
		drawRectangle(glm::mat3(1.), blur_shader);

		io_fbo.bind();
		util_fbo.bindContent(TextureUnit::misc);

		blur_shader.setVec2(1, glm::vec2(0., radius_factor / (float)util_fbo.tex_height));
		drawRectangle(glm::mat3(1.), blur_shader);

		setBlending(Blending::linear);
		rt.bind();
	}

	void BlurBuffer::bindContent(fgr::TextureUnit unit) {
		io_fbo.bindContent(unit);
	}
	
	void BlurBuffer::dispose() {
		io_fbo.dispose(true);
		util_fbo.dispose(true);
	}
}
