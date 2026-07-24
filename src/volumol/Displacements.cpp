#include "Displacements.h"
#include "TextUtil.h"
#include "Molecule.h"
#include "../logic/Pointer.h"

namespace mol {
	flo::Array<float> normal_modes;
	extern Molecule molecule;

	using namespace FileReader;

namespace Displacements {
	void loadNormalModes() {
		normal_modes.resize(molecule.atoms.size() * molecule.atoms.size() * 9);
		bool error = false;
		bool header = true;
		uint column_count = 0;
		int x_off = 0;
		int y_off = 0;
		while (!endOfFile()) {
			int i = 0;
			skipWhitespace();
			for (; !endOfLine(); ++i) {
				float v = readFloat(error);
				if (error) {
					throwError("Expected floating point");
					return;
				}
				skipWhitespace();
				if (i > column_count && !header) {
					throwError("Got more columns than expected");
					return;
				}
				if (i && !header) {
					normal_modes[(i - 1 + x_off) * 3 * molecule.atoms.size() + y_off] = v;
				}
			}
			if (header) {
				column_count = i;
				if (x_off + column_count > 3 * molecule.atoms.size()) {
					throwError("There are more normal modes than degrees of freedom");
					return;
				}
			}
			else ++y_off;
			header = false;
			if (y_off == 3 * molecule.atoms.size()) {
				header = true;
				x_off += column_count;
				y_off = 0;
			}
			nextLine();
		}
	}
}
}
