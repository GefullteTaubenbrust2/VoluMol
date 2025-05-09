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
		std::vector<glm::ivec2> bonds;
		
		Molecule() = default;

		void setBonds();

		void generateMesh(Mesh& mesh) const;

		uint getIndex(uint id) const;

		Atom getAtom(uint atom) const;
	};

	extern const float vdw_radii_A[119];
	extern const float covalent_radii_A[119];
	extern const std::string element_names[119];
	extern const bool element_metallic[119];

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
