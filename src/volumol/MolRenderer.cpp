#include "MolRenderer.h"

#include "Constants.h"
#include "Isosurface.h"

#include "../graphics/GErrorHandler.h"
#include "../graphics/3D/ShadowMap.h"
#include "../graphics/Window.h"
#include "../graphics/FrameBuffer.h"
#include "../graphics/Blur.h"
#include "../graphics/Renderstate.h"
#include "../logic/Random.h"

namespace mol {
	RenderProperties settings;
	CubeMap cubemap;
	Molecule molecule;
}

namespace mol::Renderer {
	fgr::Mesh molecule_mesh, isosurface_mesh;
	fgr::Shader mesh_shader, post_shader, ssao_shader, geometry_shader, outline_shader, volumetric_shader, merge_shader;
	fgr::View view;
	fgr::MultiFrameBuffer geometry_fbo;
	fgr::BlurBuffer outline_blur, ssao_blur;
	fgr::FrameBufferMS fbo_ms;
	fgr::FrameBuffer fbo1, fbo2, taa_fbo;
	fgr::CascadedShadowMap csm;
	glm::vec3 ssao_offsets[64];
	glm::mat4 model_matrix = glm::mat4(1.0);
	std::vector<glm::vec3> molecule_positions;

	bool update_molecule = false;

	bool use_volumetric = false;

	std::vector<glm::vec2> taa_jitter_offsets = {glm::vec2(0.f)};

	glm::vec3 camera_position, camera_direction;

	void init() {
		mesh_shader = fgr::Shader("shaders/volumol/basic.vert", "shaders/volumol/basic.frag", std::vector<std::string>{
			"model",			// 0
			"view",				// 1
			"projection",		// 2
			"sun_direction",	// 3
			"sun_color",		// 4
			"ambient_color",	// 5
			"camera_pos",		// 6
			"shadow_map",		// 7
			"light_matrices",	// 8
			"layer_depths",		// 9
			"offset",			// 10
			"camera_dir"		// 11
		});
		mesh_shader.compile("#define SHADOWMAP_LEVELS 8\n");

		geometry_shader = fgr::Shader("shaders/volumol/geometry.vert", "shaders/volumol/geometry.frag", std::vector<std::string>{"model", "view", "projection", "offset"});
		geometry_shader.compile();

		molecule_mesh.init();
		isosurface_mesh.init();
		fbo_ms.init(fgr::window::width, fgr::window::height, GL_RGBA16F);
		fbo1.init(fgr::window::width, fgr::window::height, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_NEAREST);
		fbo2.init(fgr::window::width, fgr::window::height, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR);
		taa_fbo.init(fgr::window::width, fgr::window::height, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_NEAREST);
		outline_blur = fgr::BlurBuffer(settings.outline_radius);
		outline_blur.init();
		ssao_blur = fgr::BlurBuffer(2);
		ssao_blur.init();
		geometry_fbo.allocateAttachments(2);
		geometry_fbo.init(fgr::window::width, fgr::window::height, GL_CLAMP_TO_EDGE, GL_LINEAR);

		ssao_shader = fgr::Shader("shaders/volumol/ssao.vert", "shaders/volumol/ssao.frag", std::vector<std::string>{
			"positions",		// 0
			"normals",			// 1
			"ssao_offsets",		// 2
			"ssao_radius",		// 3
			"ssao_exponent",	// 4
			"projection",		// 5
			"rotation_offset",	// 6
			"iterations",		// 7
		});
		ssao_shader.compile();

		merge_shader = fgr::Shader("shaders/volumol/merge.vert", "shaders/volumol/merge.frag", std::vector<std::string>{
			"texture",			// 0
			"ssao",				// 1
			"outlines",			// 2
			"ssao_intensity",	// 3
			"ssao_exponent",	// 4
			"outline_radius",	// 5
		});
		merge_shader.compile();

		post_shader = fgr::Shader("shaders/volumol/post.vert", "shaders/volumol/post.frag", std::vector<std::string>{"texture", "clear_color", "taa_alpha", "brightness"});
		post_shader.compile("#define PREMULTIPLY_COLOR 1\n");

		outline_shader = fgr::Shader("shaders/volumol/outline.vert", "shaders/volumol/outline.frag", std::vector<std::string>{"depth_tex", "z_near", "z_far"});
		outline_shader.compile();

		volumetric_shader = fgr::Shader("shaders/volumol/volumetric.vert", "shaders/volumol/volumetric.frag", std::vector<std::string>{
			"proj_inv",			// 0
			"view",				// 1
			"camera_position",	// 2
			"cubemap",			// 3
			"cubemap_origin",	// 4
			"cubemap_size",		// 5
			"depth_map",		// 6
			"sun_direction",	// 7
			"sun_color",		// 8
			"ambient_color",	// 9
			"iterations",		// 10
			"light_iterations",	// 11
			"light_distance",	// 12
			"positive_color",	// 13
			"negative_color",	// 14
			"z_near",			// 15
			"z_far",			// 16
			"shadow_map",		// 17
			"light_matrices",	// 18
			"layer_depths",		// 19
			"background",		// 20
			"density_factor",	// 21
			"density_cutoff",	// 22
			"camera_dir",		// 23
			"gradient_factor",	// 24
			"offset",			// 25
		});
		volumetric_shader.compile("#define SHADOWMAP_LEVELS 1\n#define VOLUMETRIC_SHADOWMAP 1\n");

		csm = fgr::CascadedShadowMap(1, 2048, 0.25f);
		csm.init();

		for (int i = 0; i < 64; ++i) {
			glm::vec3 p = glm::vec3(10.);
			while (glm::length(p) > 1.0) {
				p = glm::vec3(flo::random.next(-1.f, 1.f), flo::random.next(-1.f, 1.f), flo::random.next(0.f, 1.f));
				p *= glm::pow(glm::length(p), 0.5f);
			}
			ssao_offsets[i] = p;
		}

		ssao_shader.setVec3Array(2, ssao_offsets, 64);

		updateSettings(RenderProperties());
	}

	void updateSettings(const RenderProperties& _settings) {
		std::string definitions = "#define SHADOWMAP_LEVELS 1\n";
		if (_settings.volumetric_shadowmap) definitions += "#define VOLUMETRIC_SHADOWMAP 1\n";
		if (_settings.orthographic) definitions += "#define ORTHOGRAPHIC 1\n";
		if (_settings.emissive_volume) definitions += "#define EMISSIVE_VOLUME 1\n";
		if (_settings.premulitply_color) definitions += "#define PREMULTIPLY_COLOR 1\n";
		if (_settings.volumetric_color_mode) definitions += "#define DENSITY_MODE 1\n";

		if (
		_settings.orthographic			!= settings.orthographic			|| 
		_settings.volumetric_shadowmap	!= settings.volumetric_shadowmap	||
		_settings.emissive_volume		!= settings.emissive_volume			||
		_settings.premulitply_color		!= settings.premulitply_color		||
		_settings.volumetric_color_mode	!= settings.volumetric_color_mode
		) {
			mesh_shader.compile(definitions);
			volumetric_shader.compile(definitions);
			outline_shader.compile(definitions);
			post_shader.compile(definitions);
		}

		settings = _settings;
		glm::vec3 sun_position = glm::mat3(model_matrix) * settings.sun_position;
		mesh_shader.setVec3(3, glm::normalize(sun_position));
		mesh_shader.setVec3(4, glm::pow(settings.sun_color, glm::vec3(2.2)));
		mesh_shader.setVec3(5, glm::pow(settings.ambient_color, glm::vec3(2.2)));
		mesh_shader.setVec3(11, glm::vec3(glm::mat3(model_matrix) * camera_direction));
		volumetric_shader.setVec3(7, glm::normalize(sun_position));
		volumetric_shader.setVec3(8, glm::pow(settings.sun_color, glm::vec3(2.2)));
		volumetric_shader.setVec3(9, glm::pow(settings.ambient_color, glm::vec3(2.2)));
		volumetric_shader.setInt(10, settings.volumetric_iterations);
		volumetric_shader.setInt(11, settings.volumetric_light_iterations);
		volumetric_shader.setFloat(12, settings.volumetric_light_distance);
		volumetric_shader.setVec3(13, glm::pow(settings.mo_colors[0], glm::vec3(2.2)));
		volumetric_shader.setVec3(14, glm::pow(settings.mo_colors[1], glm::vec3(2.2)));
		volumetric_shader.setFloat(15, settings.z_near);
		volumetric_shader.setFloat(16, settings.z_far);
		volumetric_shader.setFloat(21, settings.volumetric_density);
		volumetric_shader.setFloat(22, settings.volumetric_cutoff);
		volumetric_shader.setFloat(24, settings.volumetric_gradient);
		ssao_shader.setFloat(3, settings.ao_radius);
		ssao_shader.setFloat(4, settings.ao_exponent);
		ssao_shader.setInt(7, settings.ao_iterations);
		merge_shader.setFloat(3, settings.ao_intensity);
		merge_shader.setFloat(4, settings.ao_exponent);
		merge_shader.setFloat(5, settings.outline_radius);
		post_shader.setVec3(1, settings.clear_color);
		post_shader.setFloat(3, settings.brightness);
		outline_shader.setFloat(1, settings.z_near);
		outline_shader.setFloat(2, settings.z_far);
		ssao_blur.blur_radius = 2.f + glm::min(settings.outline_radius, 3.f);
		outline_blur.blur_radius = settings.outline_radius;
		const uint taa_size = settings.taa_quality;
		taa_jitter_offsets.resize(taa_size * taa_size);
		for (int x = 0; x < taa_size; ++x) {
			for (int y = 0; y < taa_size; ++y) {
				taa_jitter_offsets[x + y * taa_size] = 2.0f * glm::vec2(x, y) / float(taa_size) - 1.0f;
			}
		}
		post_shader.setFloat(2, 1.f / float(taa_size * taa_size));
		csm.fitScene(molecule_positions, sun_position, 4.f);
	}

	void setMolecule(const Molecule& mol) {
		molecule = mol;
		isosurface_mesh.vertices.clear();
		isosurface_mesh.indices.clear();
		isosurface_mesh.update();

		molecule.setBonds(settings);
		molecule_positions.clear();
		molecule_positions.reserve(molecule.atoms.size());
		for (Atom a : molecule.atoms) {
			molecule_positions.push_back(a.position);
		}
		model_matrix = glm::mat4(1.0);
		glm::vec3 sun_position = settings.sun_position;
		csm.fitScene(molecule_positions, sun_position, 4.f);
		use_volumetric = false;
		update_molecule = true;
	}

	void addBond(uint a, uint b) {
		a = molecule.getIndex(a);
		b = molecule.getIndex(b);
		for (int i = 0; i < molecule.bonds.size(); ++i) {
			if ((molecule.bonds[i].x == a && molecule.bonds[i].y == b) || (molecule.bonds[i].x == b && molecule.bonds[i].y == a)) return;
		}
		molecule.bonds.push_back(glm::ivec2(a, b));
		update_molecule = true;
	}

	void removeBond(uint a, uint b) {
		a = molecule.getIndex(a);
		b = molecule.getIndex(b);
		for (int i = 0; i < molecule.bonds.size(); ++i) {
			if ((molecule.bonds[i].x == a && molecule.bonds[i].y == b) || (molecule.bonds[i].x == b && molecule.bonds[i].y == a)) {
				molecule.bonds.erase(molecule.bonds.begin() + i);
				--i;
				continue;
			}
		}
		update_molecule = true;
	}

	void setVolumetric() {
		if (!cubemap.texture.id) return;

		volumetric_shader.setVec3(4, cubemap.origin);
		volumetric_shader.setVec3(5, cubemap.size);

		use_volumetric = true;
	}

	void setIsosurface() {
		float isovalue = settings.isovalue / glm::pow(a0_A, 1.5);
		Mesh iso_mesh = generateIsosurface(cubemap, isovalue, settings.mo_colors[0], glm::vec2(settings.isosurface_roughness, settings.isosurface_metallicity));
		isosurface_mesh.vertices = iso_mesh.vertices;
		isosurface_mesh.indices = iso_mesh.indices;
		iso_mesh = generateIsosurface(cubemap, isovalue, settings.mo_colors[1], glm::vec2(settings.isosurface_roughness, settings.isosurface_metallicity), true);
		isosurface_mesh.mergeMesh(iso_mesh, glm::mat4(1.0));
	}

	Atom getAtom(uint atom) {
		return molecule.getAtom(atom);
	}

	void setTransform(const glm::mat4& transform) {
		model_matrix = glm::inverse(transform);
		updateSettings(settings);
	}

	glm::mat4 getTransform(uint atom0, uint atom1, uint atom2, const glm::vec3 position0, const glm::vec3 dir01, const glm::vec3 dir02) {
		Atom a0 = molecule.getAtom(atom0);
		Atom a1 = molecule.getAtom(atom1);
		Atom a2 = molecule.getAtom(atom2);

		glm::vec3 d01 = glm::normalize(a1.position - a0.position);
		glm::vec3 d02 = glm::normalize(a2.position - a0.position);
		d02 = glm::normalize(d02 - d01 * glm::dot(d01, d02));
		glm::vec3 normal = glm::normalize(glm::cross(d01, d02));

		glm::mat4 intermediate_space = glm::inverse(glm::mat4(
			glm::vec4(d01, 0.f),
			glm::vec4(d02, 0.f),
			glm::vec4(normal, 0.f),
			glm::vec4(a0.position, 1.f)
		));

		return glm::mat4(
			glm::vec4(dir01, 0.f),
			glm::vec4(dir02, 0.f),
			glm::vec4(glm::normalize(glm::cross(dir01, dir02)), 0.f),
			glm::vec4(position0, 1.f)
		) * intermediate_space;
	}

	void orientCamera(const glm::vec3& position, const glm::vec3& direction) {
		glm::vec3 p = model_matrix * glm::vec4(position, 1.f);
		glm::vec3 d = glm::mat3(model_matrix) * glm::normalize(direction);
		view.setOrientation(p, d, glm::mat3(model_matrix) * glm::vec3(0., 0., 1.));
		camera_position = position;
		camera_direction = direction;
		mesh_shader.setVec3(6, p);
		mesh_shader.setVec3(11, glm::vec3(glm::mat3(model_matrix) * camera_direction));
		volumetric_shader.setVec3(2, p);
		volumetric_shader.setVec3(23, d);
	}

	void renderFrame(uint width, uint height) {
		if (update_molecule) {
			molecule.generateMesh(molecule_mesh, settings);
			update_molecule = false;
		}

		if (settings.orthographic) view.setOrthographic(settings.fov * (float)width / (float)height, settings.fov, settings.z_near, settings.z_far);
		else view.setPerspective(glm::radians(settings.fov), width, height, settings.z_near, settings.z_far);

		geometry_fbo.resize(width, height);
		fbo_ms.resize(width, height);
		fbo1.resize(width, height);
		fbo2.resize(width, height);
		taa_fbo.resize(width, height);

		taa_fbo.clear(glm::vec4(0.0));

		csm.clear();
		csm.drawShadows(molecule_mesh);
		if (isosurface_mesh.vertices.size()) csm.drawShadows(isosurface_mesh);

		for (int i = 0; i < taa_jitter_offsets.size(); ++i) {
			fbo1.clear(glm::vec4(0.0));
			geometry_fbo.clear(glm::vec4(0.0));

			fgr::setBlending(fgr::Blending::none);

			mesh_shader.setVec2(10, taa_jitter_offsets[i] / glm::vec2(width, height));
			geometry_shader.setVec2(3, taa_jitter_offsets[i] / glm::vec2(width, height));

			mesh_shader.setMat4(0, glm::mat4(1.0));
			mesh_shader.setMat4(1, view.view);
			mesh_shader.setMat4(2, view.projection);
			csm.bindUniforms(mesh_shader, 7);

			fbo1.bind();
			//fbo_ms.bind();
			molecule_mesh.render(mesh_shader);
			if (isosurface_mesh.vertices.size()) isosurface_mesh.render(mesh_shader);
			//fbo_ms.unbind();
			fbo1.unbind();

			//fbo_ms.resolve(fbo1.fbo_id);

			geometry_shader.setMat4(0, glm::mat4(1.0));
			geometry_shader.setMat4(1, view.view);
			geometry_shader.setMat4(2, view.projection);

			geometry_fbo.bind();
			molecule_mesh.render(geometry_shader);
			if (isosurface_mesh.vertices.size()) isosurface_mesh.render(geometry_shader);

			geometry_fbo.unbind();

			if (settings.outline_radius > 0.f) {
				geometry_fbo.bindDepthTexture(fgr::TextureUnit::texture0);
				outline_shader.setInt(0, fgr::TextureUnit::texture0);

				fbo2.bind();
				fgr::drawRectangle(glm::mat3(1.), outline_shader);
				fbo2.unbind();

				outline_blur.blur(fbo2);
			}
			else {
				fbo2.clear(glm::vec4(0.));
				outline_blur.blur(fbo2);
			}

			if (settings.ao_intensity > 0.f) {
				fbo2.bind();
				fbo2.clear(glm::vec4(0.0));
				geometry_fbo.bindContent(0, fgr::TextureUnit::texture0);
				geometry_fbo.bindContent(1, fgr::TextureUnit::texture1);
				ssao_shader.setInt(0, fgr::TextureUnit::texture0);
				ssao_shader.setInt(1, fgr::TextureUnit::texture1);
				ssao_shader.setMat4(5, view.projection);
				ssao_shader.setFloat(6, (float)i / (float)(settings.taa_quality * settings.taa_quality));

				fgr::drawRectangle(glm::mat3(1.), ssao_shader);
				fbo2.unbind();

				ssao_blur.blur(fbo2);
			}
			else {
				fbo2.clear(glm::vec4(1.));
				ssao_blur.blur(fbo2);
			}

			fbo2.bind();
			fbo2.clear(glm::vec4(0.0));
			fbo1.bindContent(fgr::TextureUnit::texture0);
			ssao_blur.bindContent(fgr::TextureUnit::texture1);
			outline_blur.bindContent(fgr::TextureUnit::texture2);
			merge_shader.setInt(0, fgr::TextureUnit::texture0);
			merge_shader.setInt(1, fgr::TextureUnit::texture1);
			merge_shader.setInt(2, fgr::TextureUnit::texture2);

			fgr::drawRectangle(glm::mat3(1.), merge_shader);
			fbo2.unbind();

			if (use_volumetric) {
				fbo1.bind();
				fbo1.clear(glm::vec4(0.0));
				cubemap.texture.bindToUnit(fgr::TextureUnit::texture0);
				geometry_fbo.bindDepthTexture(fgr::TextureUnit::texture1);
				fbo2.bindContent(fgr::TextureUnit::texture2);
				csm.bindUniforms(volumetric_shader, 17, fgr::TextureUnit::texture3);
				volumetric_shader.setMat4(0, glm::inverse(view.projection));
				volumetric_shader.setMat4(1, view.view);
				volumetric_shader.setInt(3, fgr::TextureUnit::texture0);
				volumetric_shader.setInt(6, fgr::TextureUnit::texture1);
				volumetric_shader.setInt(20, fgr::TextureUnit::texture2);
				volumetric_shader.setVec2(25, taa_jitter_offsets[i] * (float)settings.taa_quality);

				fgr::drawRectangle(glm::mat3(1.), volumetric_shader);
				fbo1.unbind();

				fbo1.bindContent(fgr::TextureUnit::texture0);
			}
			else {
				fbo2.bindContent(fgr::TextureUnit::texture0);
			}
			
			post_shader.setInt(0, fgr::TextureUnit::texture0);

			fgr::setBlending(fgr::Blending::additive);

			taa_fbo.bind();
			fgr::drawRectangle(glm::mat3(1.), post_shader);
			taa_fbo.unbind();

			fgr::setBlending(fgr::Blending::linear);
		}
		taa_fbo.bindContent(fgr::TextureUnit::texture0);
		fgr::Shader::textured.setInt(0, fgr::TextureUnit::texture0);
		fgr::drawRectangle(glm::mat3(1.0), fgr::Shader::textured);
	}

	void saveImage(const std::string& path, uint width, uint height) {
		fgr::TextureHandle texture = fgr::TextureHandle(fgr::window::width, fgr::window::height);

		renderFrame(width, height);

		fgr::FrameBuffer fbo;
		fbo.init(width, height, GL_RGBA, GL_CLAMP_TO_BORDER, GL_LINEAR);
		taa_fbo.resolve(fbo);

		texture.loadFromID(fbo.texture_id);
		texture.saveFile(path);
	}
}
