#include "Molecule.h"
#include "MeshGenerator.h"
#include "Settings.h"

namespace mol {
	Atom::Atom(uint atomic_number, const glm::vec3& position) :
		Z(atomic_number), position(position) {}

	void Molecule::setBonds() {
		bonds.clear();
		for (int i = 0; i < atoms.size(); ++i) {
			glm::vec3 pos = atoms[i].position;
			for (int j = i + 1; j < atoms.size(); ++j) {
				glm::vec3 r = atoms[j].position - pos;
				float distance = glm::length(r);
				if (distance < (1.f + settings.bond_length_tolerance) * (covalent_radii_A[atoms[i].Z] + covalent_radii_A[atoms[j].Z])) {
					bonds.push_back(glm::ivec2(i, j));
				}
			}
		}
		if (!settings.multicenter_coordination) return;
		for (int i = 0; i < bonds.size(); ++i) {
			glm::ivec2 bond = bonds[i];
			if (atoms[bond.x].Z != carbon) {
				int x = bond.x;
				bond.x = bond.y;
				bond.y = x;
			}
			
			if (atoms[bond.x].Z != carbon || !element_metallic[atoms[bond.y].Z]) continue;
			
			int metallic_index = bond.y;
			bonds.erase(bonds.begin() + i);
			--i;

			std::vector<int> ligand_centers{ bond.x };
			std::vector<int> explore{ bond.x };
			std::vector<int> candidates;

			while (explore.size()) {
				int current = explore[explore.size() - 1];
				explore.resize(explore.size() - 1);
				for (int j = 0; j < bonds.size(); ++j) {
					bond = bonds[j];
					if (atoms[bond.x].Z != carbon || atoms[bond.y].Z != carbon) continue;

					int new_candidate = 0;
					if (bond.x == current) new_candidate = bond.y;
					else if (bond.y == current) new_candidate = bond.x;
					else continue;

					bool contained = false;
					for (int x : ligand_centers) if (x == new_candidate) {
						contained = true;
						break;
					}
					if (!contained) candidates.push_back(new_candidate);
				}
				while(candidates.size()) {
					current = candidates[candidates.size() - 1];
					candidates.resize(candidates.size() - 1);
					for (int j = 0; j < bonds.size(); ++j) {
						bond = bonds[j];
						if (!(bond.x == current && bond.y == metallic_index) && !(bond.y == current && bond.x == metallic_index)) continue;

						bonds.erase(bonds.begin() + j);
						--j;
						bool contained = false;
						for (int x : explore) if (x == current) {
							contained = true;
							break;
						}
						if (!contained) {
							explore.push_back(current);
							ligand_centers.push_back(current);
						}
					}
				}
			}
			if (ligand_centers.size() == 1) {
				++i;
				bonds.insert(bonds.begin() + i, glm::ivec2(ligand_centers[0], metallic_index));
			}
			else {
				glm::vec3 avg_position = glm::vec3(0.);
				for (int a : ligand_centers) {
					avg_position += atoms[a].position;
				}
				avg_position /= ligand_centers.size();

				bonds.push_back(glm::ivec2(atoms.size(), metallic_index));
				atoms.push_back(Atom(ghost_atom, avg_position));
			}
		}
	}

	void Molecule::generateMesh(Mesh& mesh) const {
		mesh.vertices.clear();
		mesh.indices.clear();
		Mesh sphere_mesh;
		generateIsosphere(sphere_mesh, settings.sphere_subdivisions);
		Mesh cylinder_mesh;
		generateCylinder(cylinder_mesh, settings.cylinder_resolution, settings.bond_thickness, !settings.smooth_bonds);

		for (int i = 0; i < atoms.size(); ++i) {
			glm::vec3 pos = atoms[i].position;
			glm::vec3 color = settings.materials[atoms[i].Z].color;
			glm::vec2 uv = glm::vec2(settings.materials[atoms[i].Z].roughness, settings.materials[atoms[i].Z].metallicity);
			float size = vdw_radii_A[atoms[i].Z] * settings.size_factor;
			if (!atoms[i].Z) size = settings.bond_thickness;
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
			if (bond.x < 0 || bond.x >= atoms.size() || bond.y < 0 || bond.y >= atoms.size()) continue;
			const Atom& a0 = atoms[bond.x];
			const Atom& a1 = atoms[bond.y];

			glm::vec3 pos0 = a0.position;
			glm::vec3 color0 = settings.materials[a0.Z].color;
			glm::vec2 uv0 = glm::vec2(settings.materials[a0.Z].roughness, settings.materials[a0.Z].metallicity);

			glm::vec3 pos1 = a1.position;
			glm::vec3 color1 = settings.materials[a1.Z].color;
			glm::vec2 uv1 = glm::vec2(settings.materials[a1.Z].roughness, settings.materials[a1.Z].metallicity);

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
		return Atom(invalid, glm::vec3(0., 0., 0.));
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

	// V. Pekka Pyykk�, M. Atsumi, Chem. Eur. J. 15, 2008, 186-197.
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

	const bool element_metallic[119] = {
		false,
		false,                                                                                                                 false,
		true , true ,                                                                       false, false, false, false, false, false,
		true , true ,                                                                       true , false, false, false, false, false,
		true , true , true , true , true , true , true , true , true , true , true , true , true , false, false, false, false, false,
		true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , false, false, false,
		true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true ,
		              true , true , true , true , true , true , true , true , true , true , true , true , true , false, false, false,
		true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true ,
		              true , true , true , true , true , true , true , true , true , true , true , true , true , true , false, false,
	};
}
