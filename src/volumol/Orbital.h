#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "../graphics/3D/Texture3D.h"

namespace mol {
	struct GTO {
		double e_r;
		int e_x, e_y, e_z;
		double coeff;

		GTO() = default;

		GTO(double e_r, int e_x, int e_y, int e_z, double c);

		double phi(const glm::dvec3& pos);
	};

	struct STO {
		double alpha;
		int e_r, e_x, e_y, e_z;
		double coeff;

		STO() = default;

		STO(double alpha, int e_r, int e_x, int e_y, int e_z, double c);

		double phi(const glm::dvec3& pos);
	};

	/*struct STO {
		double e_r;
		glm::ivec3 e_Y[6];
		double c_Y[6];
		double R[10];
		uint n;

		double coeff;

		STO() = default;

		STO(double e_r, int n, int l, int m, double c);
	};*/
	
	struct CubeMap {
		fgr::TextureHandle3D texture;
		glm::dvec3 origin;
		glm::dvec3 size;

		CubeMap() = default;

		CubeMap(glm::ivec3 resolution);

		void resize(glm::ivec3 resolution);

		float sample(glm::ivec3 coord);

		glm::vec3 sampleGradient(glm::ivec3 coord);
	};

	struct ContractedBasis {
		std::vector<GTO> gto_primitives;
		std::vector<STO> sto_primitives;
		glm::dvec3 origin = glm::dvec3(0.0);

		double sample(glm::dvec3 r);
	};

	std::vector<GTO> generateSphericalGTO(double exponent, int l, int m);

	GTO generateCartesianGTO(double exponent, int x, int y, int z);

	enum class Spin {
		up = 0,
		down = 1,
		alpha = 0,
		beta = 1,
	};

	struct MolecularOrbital {
		std::vector<ContractedBasis>* basis = nullptr;
		bool use_stos = false;
		std::vector<double> lcao_coefficients;
		double energy = 0.0;
		Spin spin = Spin::up;
		double occupation = 0.0;
		std::string name;

		MolecularOrbital() = default;

		void writeCubeMap(CubeMap& cubemap, bool print_progress = true);

	private:
		static void threadWriteCubeSlice(MolecularOrbital* mo, CubeMap* cubemap, uint z_min, uint z_max);

		void writeCubeSlice(CubeMap& cubemap, uint z_min, uint z_max, bool print_progess = false) const;
	};

	void resizeCubeMap(uint x, uint y, uint z);

	void MOCubeMap(uint orbital);

	void densityCubeMapMO();

	uint findLUMO();

	uint MOcount();

	MolecularOrbital& getMO(uint number);
}