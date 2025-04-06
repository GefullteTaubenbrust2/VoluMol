#pragma once
#include <glm/glm.hpp>
#include "../logic/Types.h"

namespace mol {
	struct ElementMaterial {
		glm::vec3 color = glm::vec3(1.);
		float metallicity = 0.0f;
		float roughness = 0.5f;

		ElementMaterial() = default;
	};

	struct RenderProperties {
		ElementMaterial materials[119];

		float size_factor = 0.2f;
		bool smooth_bonds = false;
		float bond_thickness = 0.2f;
		float bond_length_tolerance = 0.3f;
		bool multicenter_coordination;

		glm::vec3 ambient_color = glm::vec3(0.4);
		glm::vec3 sun_color = glm::vec3(2.);
		glm::vec3 sun_position = glm::vec3(2., 1., 1.);

		uint sphere_subdivisions = 3;
		uint cylinder_resolution = 32;

		float fov = 70.f;
		float z_near = 0.3f;
		float z_far = 300.f;
		bool orthographic = false;

		float outline_radius = 2.f;

		float ao_intensity = 1.f;
		float ao_radius = 0.5f;
		float ao_exponent = 2.f;
		uint ao_iterations = 16;

		glm::vec3 mo_colors[2] = { glm::vec3(1.0, 0.25, 0.0), glm::vec3(0.0, 0.4, 1.0) };

		float cubemap_clearance = 4.f;
		float cubemap_density = 8.f;

		uint volumetric_iterations = 100;
		uint volumetric_light_iterations = 5;
		float volumetric_light_distance = 3.f;
		float volumetric_density = 50.f;
		float volumetric_cutoff = 0.00001f;
		float volumetric_gradient = 1.f;
		bool volumetric_shadowmap = true;
		bool emissive_volume = false;
		bool volumetric_color_mode = false;

		glm::vec3 clear_color = glm::vec3(1.0);
		bool premulitply_color = true;
		float brightness = 1.f;

		uint taa_quality = 1;

		float isovalue = 0.02f;
		float isosurface_roughness = 0.5f;
		float isosurface_metallicity = 0.f;

		uint cubemap_slice_count = 1;
		bool cubemap_use_gpu = true;

		RenderProperties();
	};

	extern RenderProperties settings;
}