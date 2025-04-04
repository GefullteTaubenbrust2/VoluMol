#include "Settings.h"
#include "Molecule.h"

namespace mol {
	RenderProperties settings;

	extern const glm::vec3 default_element_colors[119];
	extern const uint nonmetals[23];

	RenderProperties::RenderProperties() {
		for (int i = 0; i < 119; ++i) {
			materials[i].color = default_element_colors[i];
			materials[i].metallicity = 1.0;
		}

		for (int i = 0; i < 23; ++i) {
			materials[nonmetals[i]].metallicity = 0.0;
		}
	}

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