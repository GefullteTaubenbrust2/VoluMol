#include "Molecule.h"
#include "MeshGenerator.h"

namespace mol {
	extern const glm::vec3 default_element_colors[119];
	extern const uint nonmetals[23];

	Atom::Atom(uint atomic_number, const glm::vec3& position) :
		Z(atomic_number), position(position) {}

	RenderProperties::RenderProperties() {
		for (int i = 0; i < 119; ++i) {
			materials[i].color = default_element_colors[i];
			materials[i].metallicity = 1.0;
		}

		for (int i = 0; i < 23; ++i) {
			materials[nonmetals[i]].metallicity = 0.0;
		}
	}

	void Molecule::setBonds(const RenderProperties& properties) {
		bonds.clear();
		for (int i = 0; i < atoms.size(); ++i) {
			glm::vec3 pos = atoms[i].position;
			for (int j = i + 1; j < atoms.size(); ++j) {
				glm::vec3 r = atoms[j].position - pos;
				float distance = glm::length(r);
				if (distance < (1.f + properties.bond_length_tolerance) * (covalent_radii_A[atoms[i].Z] + covalent_radii_A[atoms[j].Z])) {
					bonds.push_back(glm::ivec2(i, j));
				}
			}
		}
	}

	void Molecule::generateMesh(Mesh& mesh, const RenderProperties& properties) const {
		mesh.vertices.clear();
		mesh.indices.clear();
		Mesh sphere_mesh;
		generateIsosphere(sphere_mesh, properties.sphere_subdivisions);
		Mesh cylinder_mesh;
		generateCylinder(cylinder_mesh, properties.cylinder_resolution, properties.bond_thickness, !properties.smooth_bonds);

		for (int i = 0; i < atoms.size(); ++i) {
			glm::vec3 pos = atoms[i].position;
			glm::vec3 color = properties.materials[atoms[i].Z].color;
			glm::vec2 uv = glm::vec2(properties.materials[atoms[i].Z].roughness, properties.materials[atoms[i].Z].metallicity);
			float size = vdw_radii_A[atoms[i].Z] * properties.size_factor;
			if (!atoms[i].Z) size = properties.size_factor * 0.5f;
			glm::mat4 transform = glm::mat4{
				glm::vec4(size, 0.0, 0.0, 0.0),
				glm::vec4(0.0, size, 0.0, 0.0),
				glm::vec4(0.0, 0.0, size, 0.0),
				glm::vec4(pos, 1.0)
			};

			Mesh sphere = sphere_mesh;
			for (Vertex3D& v : sphere.vertices) {
				v.color = color;
				v.tex_coord = uv;
			}
			
			mesh.mergeMesh(sphere, transform);
		}

		for (const glm::ivec2& bond : bonds) {
			const Atom& a0 = atoms[bond.x];
			const Atom& a1 = atoms[bond.y];

			glm::vec3 pos0 = a0.position;
			glm::vec3 color0 = properties.materials[a0.Z].color;
			glm::vec2 uv0 = glm::vec2(properties.materials[a0.Z].roughness, properties.materials[a0.Z].metallicity);

			glm::vec3 pos1 = a1.position;
			glm::vec3 color1 = properties.materials[a1.Z].color;
			glm::vec2 uv1 = glm::vec2(properties.materials[a1.Z].roughness, properties.materials[a1.Z].metallicity);

			glm::vec3 r = pos1 - pos0;
			float distance = glm::length(r);
			glm::vec3 nr = glm::normalize(r);
			glm::vec3 s = glm::normalize(glm::cross(nr, nr.z * nr.z > 0.5 ? glm::vec3(1.0, 0.0, 0.0) : glm::vec3(0.0, 0.0, 1.0)));
			glm::vec3 t = glm::cross(nr, s);

			glm::mat4 transform = glm::mat4{
				glm::vec4(s, 0.0),
				glm::vec4(t, 0.0),
				glm::vec4(r, 0.0),
				glm::vec4(pos0, 1.0)
			};

			Mesh cylinder = cylinder_mesh;
			for (Vertex3D& v : cylinder.vertices) {
				v.tex_coord = v.color.r * uv0 + v.color.g * uv1;
				v.color = v.color.r * color0 + v.color.g * color1;
			}

			mesh.mergeMesh(cylinder, transform);
		}

		mesh.generateNormals();
		mesh.update();
	}

	uint Molecule::getIndex(uint id) const {
		for (int i = 0; i < index_map.size(); ++i) if (index_map[i] == id) return i;
		return id;
	}

	Atom Molecule::getAtom(uint atom) const {
		if (!index_map.size() && atom < atoms.size()) return atoms[atom];
		for (int i = 0; i < index_map.size(); ++i) if (index_map[i] == atom) return atoms[i];
		return Atom();
	}

	// S. S. Batsanov, Inorganic Materials 37, 2001, 871-885.
	// Using crystallographic radii. These are not provided for noble gases 
	// and elements beyond Bi, except Th and U. These values are thus extremely 
	// crude estimates.
	const float vdw_radii_A[119] = {
		1.00, 
		1.15,                                                                                                 1.30,
		2.20, 1.90,                                                             1.80, 1.70, 1.60, 1.55, 1.50, 1.50,
		2.40, 2.20,                                                             2.10, 2.10, 1.95, 1.80, 1.80, 1.80,
		2.80, 2.40, 2.30, 2.15, 2.05, 2.05, 2.05, 2.05, 2.00, 2.00, 2.00, 2.10, 2.10, 2.10, 2.05, 1.90, 1.90, 1.90,
		2.90, 2.55, 2.40, 2.30, 2.15, 2.10, 2.05, 2.05, 2.00, 2.05, 2.10, 2.20, 2.20, 2.25, 2.20, 2.10, 2.10, 2.10,
		3.00, 2.70, 2.50, 2.40, 2.40, 2.30, 2.30, 2.30, 2.30, 2.30, 2.30, 2.30, 2.30, 2.30, 2.30, 2.30,
		            2.50, 2.25, 2.20, 2.10, 2.05, 2.00, 2.00, 2.05, 2.10, 2.05, 2.20, 2.30, 2.30, 2.20, 2.20, 2.20,
		3.10, 2.90, 2.40, 2.40, 2.40, 2.30, 2.30, 2.30, 2.30, 2.30, 2.20, 2.20, 2.20, 2.20, 2.20, 2.20,
		            2.20, 2.20, 2.20, 2.20, 2.20, 2.20, 2.20, 2.20, 2.20, 2.20, 2.20, 2.20, 2.20, 2.20, 2.20, 2.20
	};

	// V. Pekka Pyykkö, M. Atsumi, Chem. Eur. J. 15, 2008, 186-197.
	const float covalent_radii_A[119] = {
		0.80,
		0.32,                                                                                                 0.46,
		1.33, 1.02,                                                             0.85, 0.75, 0.71, 0.63, 0.64, 0.67,
		1.55, 1.39,                                                             1.26, 1.16, 1.11, 1.03, 0.99, 0.96,
		1.96, 1.71, 1.48, 1.36, 1.34, 1.22, 1.19, 1.16, 1.11, 1.10, 1.12, 1.18, 1.24, 1.21, 1.21, 1.16, 1.14, 1.17,
		2.10, 1.85, 1.63, 1.54, 1.47, 1.38, 1.28, 1.25, 1.25, 1.20, 1.28, 1.36, 1.42, 1.40, 1.40, 1.36, 1.33, 1.31,
		2.32, 1.96, 1.80, 1.63, 1.76, 1.74, 1.73, 1.72, 1.68, 1.69, 1.68, 1.67, 1.66, 1.65, 1.64, 1.70,
		            1.62, 1.52, 1.46, 1.37, 1.31, 1.29, 1.22, 1.23, 1.24, 1.33, 1.44, 1.44, 1.51, 1.45, 1.47, 1.42,
		2.23, 2.01, 1.86, 1.75, 1.69, 1.70, 1.71, 1.72, 1.66, 1.66, 1.68, 1.68, 1.65, 1.67, 1.73, 1.76,
		            1.61, 1.57, 1.49, 1.43, 1.41, 1.34, 1.29, 1.28, 1.21, 1.22, 1.36, 1.43, 1.62, 1.75, 1.65, 1.57
	};

	const std::string element_names[119] = {
		"X",
		"H",                                                                                 "He",
		"Li","Be",                                                   "B", "C", "N", "O", "F","Ne",
		"Na","Mg",                                                  "Al","Si", "P", "S","Cl","Ar",
		 "K","Ca","Sc","Ti", "V","Cr","Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr",
		"Rb","Sr", "Y","Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd","In","Sn","Sb","Te", "I","Xe",
		"Cs","Ba","La","Ce","Pr","Nd","Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb",
		          "Lu","Hf","Ta", "W","Re","Os","Ir","Pt","Au","Hg","Tl","Pb","Bi","Po","At","Rn",
		"Fr","Ra","Ac","Th","Pa", "U","Np","Pu","Am","Cm","Bk","Cf","Es","Fm","Md","No",
		          "Lr","Rf","Db","Sg","Bh","Hs","Mt","Ds","Rg","Cn","Nh","Fl","Mc","Lv","Ts","Og"
	};

	const glm::vec3 default_element_colors[119] = {
		glm::vec3(0.20, 0.50, 1.00),	// X
		glm::vec3(1.0),					// H
		glm::vec3(0.72, 0.94, 1.00),	// He
		glm::vec3(0.95, 0.75, 0.88),	// Li
		glm::vec3(0.62, 0.98, 0.24),	// Be
		glm::vec3(0.92, 0.67, 0.72),	// B
		glm::vec3(0.5),					// C
		glm::vec3(0.15, 0.28, 1.00),	// N
		glm::vec3(1.00, 0.15, 0.15),	// O
		glm::vec3(0.82, 1.00, 0.45),	// F
		glm::vec3(0.57, 0.92, 1.00),	// Ne
		glm::vec3(0.95, 0.54, 0.83),	// Na
		glm::vec3(0.37, 0.76, 0.13),	// Mg
		glm::vec3(0.81, 0.64, 0.64),	// Al
		glm::vec3(0.84, 0.68, 0.57),	// Si
		glm::vec3(0.91, 0.45, 0.14),	// P
		glm::vec3(0.88, 0.85, 0.29),	// S
		glm::vec3(0.21, 0.88, 0.09),	// Cl
		glm::vec3(0.34, 0.83, 0.95),	// Ar
		glm::vec3(0.88, 0.28, 0.71),	// K
		glm::vec3(0.24, 0.62, 0.09),	// Ca
		glm::vec3(0.49, 0.59, 0.71),	// Sc
		glm::vec3(0.63),				// Ti
		glm::vec3(0.44),				// V
		glm::vec3(0.41, 0.57, 0.82),	// Cr
		glm::vec3(0.65, 0.47, 0.62),	// Mn
		glm::vec3(0.90, 0.35, 0.20),	// Fe
		glm::vec3(0.95, 0.48, 0.63),	// Co
		glm::vec3(0.43, 0.80, 0.45),	// Ni
		glm::vec3(0.80, 0.52, 0.37),	// Cu
		glm::vec3(0.60, 0.71, 0.79),	// Zn
		glm::vec3(0.61, 0.48, 0.48),	// Ga
		glm::vec3(0.48, 0.58, 0.63),	// Ge
		glm::vec3(0.70, 0.47, 0.71),	// As
		glm::vec3(0.67, 0.48, 0.10),	// Se
		glm::vec3(0.58, 0.14, 0.11),	// Br
		glm::vec3(0.08, 0.69, 0.84),	// Kr
		glm::vec3(0.74, 0.14, 0.57),	// Rb
		glm::vec3(0.15, 0.47, 0.07),	// Sr
		glm::vec3(0.50, 0.63, 0.65),	// Y
		glm::vec3(0.27, 0.72, 0.80),	// Zr
		glm::vec3(0.32, 0.51, 0.54),	// Nb
		glm::vec3(0.15, 0.54, 0.44),	// Mo
		glm::vec3(0.60, 0.79, 0.45),	// Tc
		glm::vec3(0.51, 0.21, 0.21),	// Ru
		glm::vec3(0.41, 0.38, 0.73),	// Rh
		glm::vec3(0.22, 0.34, 0.37),	// Pd
		glm::vec3(1.0),					// Ag
		glm::vec3(0.81, 0.77, 0.61),	// Cd
		glm::vec3(0.44, 0.35, 0.35),	// In
		glm::vec3(0.33, 0.40, 0.43),	// Sn
		glm::vec3(0.61, 0.39, 0.60),	// Sb
		glm::vec3(0.50, 0.36, 0.07),	// Te
		glm::vec3(0.55, 0.07, 0.55),	// I
		glm::vec3(0.06, 0.48, 0.58),	// Xe
		glm::vec3(0.52, 0.10, 0.40),	// Cs
		glm::vec3(0.06, 0.36, 0.06),	// Ba
		glm::vec3(0.27, 0.43, 0.27),	// La
		glm::vec3(0.27, 0.43, 0.27),	// Ce
		glm::vec3(0.27, 0.43, 0.27),	// Pr
		glm::vec3(0.27, 0.43, 0.27),	// Nd
		glm::vec3(0.27, 0.43, 0.27),	// Pm
		glm::vec3(0.27, 0.43, 0.27),	// Sm
		glm::vec3(0.27, 0.43, 0.27),	// Eu
		glm::vec3(0.27, 0.43, 0.27),	// Gd
		glm::vec3(0.27, 0.43, 0.27),	// Tb
		glm::vec3(0.27, 0.43, 0.27),	// Dy
		glm::vec3(0.27, 0.43, 0.27),	// Ho
		glm::vec3(0.27, 0.43, 0.27),	// Er
		glm::vec3(0.27, 0.43, 0.27),	// Tm
		glm::vec3(0.27, 0.43, 0.27),	// Yb
		glm::vec3(0.27, 0.43, 0.27),	// Lu
		glm::vec3(0.11, 0.31, 0.50),	// Hf
		glm::vec3(0.11, 0.31, 0.50),	// Ta
		glm::vec3(0.25, 0.35, 0.46),	// W
		glm::vec3(0.11, 0.31, 0.50),	// Re
		glm::vec3(0.04, 0.18, 0.43),	// Os
		glm::vec3(0.23, 0.24, 0.44),	// Ir
		glm::vec3(0.73, 0.89, 1.00),	// Pt
		glm::vec3(1.00, 0.75, 0.34),	// Au
		glm::vec3(0.74, 0.74, 0.74),	// Hg
		glm::vec3(0.32, 0.25, 0.25),	// Tl
		glm::vec3(0.21, 0.25, 0.27),	// Pb
		glm::vec3(0.43, 0.24, 0.44),	// Bi
		glm::vec3(0.34, 0.25, 0.05),	// Po
		glm::vec3(0.38, 0.24, 0.16),	// At
		glm::vec3(0.04, 0.32, 0.39),	// Rn
		glm::vec3(0.32, 0.06, 0.25),	// Fr
		glm::vec3(0.04, 0.23, 0.04),	// Ra
		glm::vec3(0.30, 0.29, 0.74),	// Ac
		glm::vec3(0.09, 0.49, 0.86),	// Th
		glm::vec3(0.30, 0.29, 0.74),	// Pa
		glm::vec3(0.10, 0.27, 0.91),	// U
		glm::vec3(0.30, 0.29, 0.74),	// Np
		glm::vec3(0.30, 0.29, 0.74),	// Pu
		glm::vec3(0.30, 0.29, 0.74),	// Am
		glm::vec3(0.30, 0.29, 0.74),	// Cm
		glm::vec3(0.30, 0.29, 0.74),	// Bk
		glm::vec3(0.30, 0.29, 0.74),	// Cf
		glm::vec3(0.30, 0.29, 0.74),	// Es
		glm::vec3(0.30, 0.29, 0.74),	// Fm
		glm::vec3(0.30, 0.29, 0.74),	// Md
		glm::vec3(0.30, 0.29, 0.74),	// No
		glm::vec3(0.30, 0.29, 0.74),	// Lr
		glm::vec3(0.54, 0.14, 0.29),	// Rf
		glm::vec3(0.54, 0.14, 0.29),	// Db
		glm::vec3(0.54, 0.14, 0.29),	// Sg
		glm::vec3(0.54, 0.14, 0.29),	// Bh
		glm::vec3(0.54, 0.14, 0.29),	// Hs
		glm::vec3(0.54, 0.14, 0.29),	// Mt
		glm::vec3(0.54, 0.14, 0.29),	// Ds
		glm::vec3(0.54, 0.14, 0.29),	// Rg
		glm::vec3(0.54, 0.14, 0.29),	// Cn
		glm::vec3(0.25, 0.20, 0.20),	// Nh
		glm::vec3(0.14, 0.16, 0.18),	// Fl
		glm::vec3(0.27, 0.15, 0.28),	// Mc
		glm::vec3(0.17, 0.12, 0.03),	// Lv
		glm::vec3(0.25, 0.14, 0.10),	// Ts
		glm::vec3(0.03, 0.20, 0.24),	// Og
	};

	const uint nonmetals[23] = {
		ghost_atom,
		hydrogen,
		helium,
		boron,
		carbon,
		nitrogen,
		oxygen,
		fluorine,
		neon,
		silicon,
		phosphorus,
		sulfur, chlorine,
		argon,
		arsenic,
		selenium,
		bromine,
		krypton,
		tellurium,
		iodine,
		xenon,
		astatine,
		radon
	};
}
