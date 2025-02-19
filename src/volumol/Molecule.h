#pragma once
#include "../graphics/3D/3D Renderer.h"

using namespace fgr;

namespace mol {
	struct Atom {
		uint Z = 0;
		glm::vec3 position = glm::vec3(0.0);

		Atom() = default;

		Atom(uint atomic_number, const glm::vec3& position);
	};

	struct RenderProperties;

	struct Molecule {
		std::vector<Atom> atoms;
		std::vector<uint> index_map;
		
		Molecule() = default;

		void generateMesh(Mesh& mesh, const RenderProperties& properties) const;

		Atom getAtom(uint atom);
	};

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

		glm::vec3 ambient_color = glm::vec3(0.25);
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

		glm::vec3 clear_color = glm::vec3(1.0);
		bool premulitply_color = true;
		float brightness = 1.f;

		uint taa_quality = 1;

		float isovalue = 0.04f;
		float isosurface_roughness = 0.5f;
		float isosurface_metallicity = 0.f;

		uint cubemap_thread_count = 8;
		bool cubemap_use_gpu = true;

		RenderProperties();
	};

	extern const float vdw_radii_A[119];
	extern const float covalent_radii_A[119];
	extern const std::string element_names[119];

	enum ElementName {
		ghost_atom = 0,
		hydrogen = 1,
		helium = 2,
		lithium = 3,
		beryllium = 4,
		boron = 5,
		carbon = 6,
		nitrogen = 7,
		oxygen = 8,
		fluorine = 9,
		neon = 10,
		sodium = 11,
		magnesium = 12,
		aluminium = 13,
		silicon = 14,
		phosphorus = 15,
		sulfur = 16,
		chlorine = 17,
		argon = 18,
		potassium = 19,
		calcium = 20,
		scandium = 21,
		titanium = 22,
		vanadium = 23,
		chromium = 24,
		manganese = 25,
		iron = 26,
		cobalt = 27,
		nickel = 28,
		copper = 29,
		zinc = 30,
		gallium = 31,
		germanium = 32,
		arsenic = 33,
		selenium = 34,
		bromine = 35,
		krypton = 36,
		rubidium = 37,
		strontium = 38,
		yttrium = 39,
		zirconium = 40,
		niobium = 41,
		molybdenum = 42,
		technetium = 43,
		ruthenium = 44,
		rhodium = 45,
		palladium = 46,
		silver = 47,
		cadmium = 48,
		indium = 49,
		tin = 50,
		antimony = 51,
		tellurium = 52,
		iodine = 53,
		xenon = 54,
		caesium = 55,
		barium = 56,
		lanthanum = 57,
		cerium = 58,
		praeseodymium = 59,
		neodymium = 60,
		prometheum = 61,
		samarium = 62,
		europium = 63,
		gadolinum = 64,
		terbium = 65,
		dysprosium = 66,
		holmium = 67,
		erbium = 68,
		thulium = 69,
		ytterbium = 70,
		lutetium = 71,
		hafnium = 72,
		tantalum = 73,
		tungsten = 74,
		rhenium = 75,
		osmium = 76,
		iridium = 77,
		platinum = 78,
		gold = 79,
		mercury = 80,
		thallium = 81,
		lead = 82,
		bismuth = 83,
		pollonium = 84,
		astatine = 85,
		radon = 86,
		francium = 87,
		radium = 88,
		actinium = 89,
		protactinium = 90,
		thorium = 91,
		uranium = 92,
		neptunium = 93,
		plutonium = 94,
		americium = 95,
		curium = 96,
		berkelium = 97,
		californium = 98,
		einsteinium = 99,
		fermium = 100,
		mendelevium = 101,
		nobellium = 102,
		lawrencium = 103,
		rutherfordium = 104,
		dubnium = 105,
		seaborgium = 106,
		bohrium = 107,
		hassium = 108,
		meitnerium = 109,
		darmstadtium = 110,
		roentgenium = 111,
		copernicium = 112,
		nihonium = 113,
		flevorium = 114,
		moscovium = 115,
		livermorium = 116,
		tennessine = 117,
		oganesson = 118,
		invalid = 119,
	};
}
