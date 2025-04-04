#include "Orbital.h"

#include "../logic/MathUtil.h"
#include "../graphics/FrameBuffer.h"
#include "../graphics/Renderstate.h"
#include "../graphics/ComputeShader.h"
#include "Molecule.h"
#include "Settings.h"

#include <thread>

namespace mol {
	glm::ivec3 Y_exponents[150] = {
		glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),

		glm::ivec3(0, 1, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(0, 0, 1), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(1, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),

		glm::ivec3(1, 1, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(0, 1, 1), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(2, 0, 0), glm::ivec3(0, 2, 0), glm::ivec3(0, 0, 2), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(1, 0, 1), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(2, 0, 0), glm::ivec3(0, 2, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),

		glm::ivec3(2, 1, 0), glm::ivec3(0, 3, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(1, 1, 1), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(2, 1, 0), glm::ivec3(0, 3, 0), glm::ivec3(0, 1, 2), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(0, 0, 3), glm::ivec3(2, 0, 1), glm::ivec3(0, 2, 1), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(3, 0, 0), glm::ivec3(1, 2, 0), glm::ivec3(1, 0, 2), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(2, 0, 1), glm::ivec3(0, 2, 1), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(3, 0, 0), glm::ivec3(1, 2, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),

		glm::ivec3(3, 1, 0), glm::ivec3(1, 3, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(2, 1, 1), glm::ivec3(0, 3, 1), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(3, 1, 0), glm::ivec3(1, 3, 0), glm::ivec3(1, 1, 2), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(2, 1, 1), glm::ivec3(0, 3, 1), glm::ivec3(0, 1, 3), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(4, 0, 0), glm::ivec3(0, 4, 0), glm::ivec3(0, 0, 4), glm::ivec3(2, 2, 0), glm::ivec3(2, 0, 2), glm::ivec3(0, 2, 2),
		glm::ivec3(3, 0, 1), glm::ivec3(1, 2, 1), glm::ivec3(1, 0, 3), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(4, 0, 0), glm::ivec3(2, 0, 2), glm::ivec3(0, 2, 2), glm::ivec3(0, 4, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(3, 0, 1), glm::ivec3(1, 2, 1), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
		glm::ivec3(4, 0, 0), glm::ivec3(2, 2, 0), glm::ivec3(0, 4, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 0),
	};

	double Y_coefficients[150] = {
		  1.0,  0.0,  0.0,  0.0,  0.0,  0.0,

		  1.0,  0.0,  0.0,  0.0,  0.0,  0.0,
		  1.0,  0.0,  0.0,  0.0,  0.0,  0.0,
		  1.0,  0.0,  0.0,  0.0,  0.0,  0.0,

		  1.0,  0.0,  0.0,  0.0,  0.0,  0.0,
		  1.0,  0.0,  0.0,  0.0,  0.0,  0.0,
		 -1.0, -1.0,  2.0,  0.0,  0.0,  0.0,
		  1.0,  0.0,  0.0,  0.0,  0.0,  0.0,
		  1.0, -1.0,  0.0,  0.0,  0.0,  0.0,

		  3.0, -1.0,  0.0,  0.0,  0.0,  0.0,
		  1.0,  0.0,  0.0,  0.0,  0.0,  0.0,
		 -1.0, -1.0,  4.0,  0.0,  0.0,  0.0,
		  2.0, -3.0, -3.0,  0.0,  0.0,  0.0,
		 -1.0, -1.0,  4.0,  0.0,  0.0,  0.0,
		  1.0, -1.0,  0.0,  0.0,  0.0,  0.0,
		  1.0, -3.0,  0.0,  0.0,  0.0,  0.0,

		  1.0, -1.0,  0.0,  0.0,  0.0,  0.0,
		  3.0, -1.0,  0.0,  0.0,  0.0,  0.0,
		 -1.0, -1.0,  6.0,  0.0,  0.0,  0.0,
		 -3.0, -3.0,  4.0,  0.0,  0.0,  0.0,
		  3.0,  3.0,  8.0,  3.0,-27.0,-27.0,
		 -3.0, -3.0,  4.0,  0.0,  0.0,  0.0,
		 -1.0,  6.0, -6.0,  1.0,  0.0,  0.0,
		  1.0, -3.0,  0.0,  0.0,  0.0,  0.0,
		  1.0, -6.0,  1.0,  0.0,  0.0,  0.0,
	};

	double Y_normalization[25] = {
		0.2820947918,

		0.4886025119,
		0.4886025119,
		0.4886025119,

		1.092548431,
		1.092548431,
		0.3153915653,
		1.092548431,
		0.5462742153,

		0.5900435899,
		2.043970953,
		0.4570457995,
		0.2638755154,
		0.4570457995,
		1.021985476,
		0.5900435899,

		2.503342842,
		1.77013077,
		0.9461746958,
		0.6690465436,
		0.05967319366,
		0.6690465436,
		0.4730873479,
		1.77013077,
		0.6258357354,
	};

	extern CubeMap cubemap;

	bool resize_cubemap = true;

	std::vector<ContractedBasis> basis_set;
	std::vector<MolecularOrbital> mos;

	fgr::Shader gto_shader, sto_shader, density_shader;
	fgr::ComputeShader gto_compute, sto_compute, density_compute;

	double pow(double x, int e) {
		double r = 1.0;
		for (int i = 0; i < e; ++i) r *= x;
		return r;
	}

	GTO::GTO(double e_r, int e_x, int e_y, int e_z, double c) : 
	e_r(e_r), e_x(e_x), e_y(e_y), e_z(e_z) {
		coeff = glm::pow(2.0 * e_r / PI, 0.75);
		double N = glm::pow(8.0 * e_r, e_x + e_y + e_z);
		for (int i = e_x + 1; i <= e_x * 2; ++i) N /= (double)i;
		for (int i = e_y + 1; i <= e_y * 2; ++i) N /= (double)i;
		for (int i = e_z + 1; i <= e_z * 2; ++i) N /= (double)i;
		coeff *= glm::sqrt(N);
		coeff *= c;
	}

	double GTO::phi(const glm::dvec3& pos) {
		return glm::exp(-e_r * glm::dot(pos, pos)) * pow(pos.x, e_x) * pow(pos.y, e_y) * pow(pos.z, e_z);
	}

	STO::STO(double alpha, int e_r, int e_x, int e_y, int e_z, double c) :
		alpha(alpha), e_r(e_r), e_x(e_x), e_y(e_y), e_z(e_z), coeff(c) {

	}

	double STO::phi(const glm::dvec3& pos) {
		double r = glm::length(pos);
		return glm::exp(-alpha * r) * pow(r, e_r) * pow(pos.x, e_x) * pow(pos.y, e_y) * pow(pos.z, e_z);
	}

#if 0
	STO::STO(double e_r, int n, int l, int m, double c) :
		e_r(e_r), coeff(c), n(n) {
		for (int i = 0; i < 6; ++i) {
			e_Y[i] = Y_exponents[(l * l + l + m) * 6 + i];
			c_Y[i] = Y_coefficients[(l * l + l + m) * 6 + i] * Y_normalization[l * l + l + m];
		}

		double Nr = 4. * glm::max((double)(n - l - 1), 1.) / ((double)(n + l) * (double)n) * e_r * e_r * e_r;
		for (int i = 2; i < n - l - 1; i++) {
			Nr *= float(i);
		}
		for (int i = 2; i < n + l; i++) {
			Nr /= float(i);
		}
		Nr = glm::sqrt(Nr);

		for (int i = 0; i <= n - l - 1 ; ++i) R[i] = i & 1 ? -Nr : Nr;
		for (int i = n - l; i < 10; ++i) R[i] = 0.0;

		// Factor: (N+L)!/(N-M)!/(L+M)!/(M!)
		// N = n - l - 1
		// L = 2 * l + 1
		for (int M = 0; M <= n - l - 1; ++M) {
			for (int j = 1; j <= l + n        ; ++j) R[M] *= (double)j;
			for (int j = 1; j <= n - l - 1 - M; ++j) R[M] /= (double)j;
			for (int j = 1; j <= 2 * l + 1 + M; ++j) R[M] /= (double)j;
			for (int j = 1; j <= M            ; ++j) R[M] /= (double)j;
		}
	}
#endif

	std::vector<GTO> generateSphericalGTO(double exponent, int l, int m) {
		if (l == 0) {
			return std::vector<GTO>{GTO(exponent, 0, 0, 0, 1.0)};
		}
		else if (l == 1) {
			if (m == -1)     return std::vector<GTO>{GTO(exponent, 1, 0, 0, 1.0)};
			else if (m == 0) return std::vector<GTO>{GTO(exponent, 0, 0, 1, 1.0)};
			else             return std::vector<GTO>{GTO(exponent, 0, 1, 0, 1.0)};
		}
		else if (l == 2) {
			if (m == -2)      return std::vector<GTO>{GTO(exponent, 1, 1, 0, 1.0)}; 
			else if (m == -1) return std::vector<GTO>{GTO(exponent, 0, 1, 1, 1.0)};
			else if (m == 0)  return std::vector<GTO>{GTO(exponent, 0, 0, 2, 1.0), GTO(exponent, 2, 0, 0, -0.5), GTO(exponent, 0, 2, 0, -0.5)};
			else if (m == 1)  return std::vector<GTO>{GTO(exponent, 1, 0, 1, 1.0)};
			else              return std::vector<GTO>{GTO(exponent, 2, 0, 0, glm::sqrt(0.75)), GTO(exponent, 0, 2, 0, -glm::sqrt(0.75))};
		}
		else if (l == 3) {
			if (m == -3)      return std::vector<GTO>{GTO(exponent, 0, 3, 0, -glm::sqrt(5.0 / 8.0)), GTO(exponent, 2, 1, 0, glm::sqrt(9.0 / 8.0))};
			else if (m == -2) return std::vector<GTO>{GTO(exponent, 1, 1, 1, 1.0)};
			else if (m == -1) return std::vector<GTO>{GTO(exponent, 0, 1, 2, glm::sqrt(1.2)), GTO(exponent, 0, 3, 0, -glm::sqrt(3.0 / 8.0)), GTO(exponent, 2, 1, 0, -glm::sqrt(3.0 / 40.0))};
			else if (m == 0)  return std::vector<GTO>{GTO(exponent, 0, 0, 3, 1.0), GTO(exponent, 2, 0, 1, -3.0 / 2.0 / glm::sqrt(5.0)), GTO(exponent, 0, 2, 1, -3.0 / 2.0 / glm::sqrt(5.0))};
			else if (m == 1)  return std::vector<GTO>{GTO(exponent, 1, 0, 2, glm::sqrt(1.2)), GTO(exponent, 3, 0, 0, -glm::sqrt(3.0 / 8.0)), GTO(exponent, 1, 2, 0, -glm::sqrt(3.0 / 40.0))};
			else if (m == 2)  return std::vector<GTO>{GTO(exponent, 2, 0, 1, glm::sqrt(0.75)), GTO(exponent, 0, 2, 1, -glm::sqrt(0.75))};
			else              return std::vector<GTO>{GTO(exponent, 3, 0, 0, glm::sqrt(5.0 / 8.0)), GTO(exponent, 1, 2, 0, -glm::sqrt(9.0 / 8.0))};
		}
		else if (l == 4) {
			if (m == -4)      return std::vector<GTO>{GTO(exponent, 3, 1, 0, glm::sqrt(5.0 / 4.0)), GTO(exponent, 1, 3, 0, -glm::sqrt(5.0 / 4.0))};
			else if (m == -3) return std::vector<GTO>{GTO(exponent, 0, 3, 1, -glm::sqrt(5.0 / 8.0)), GTO(exponent, 2, 1, 1, glm::sqrt(9.0 / 8.0))};
			else if (m == -2) return std::vector<GTO>{GTO(exponent, 1, 1, 2, glm::sqrt(9.0 / 7.0)), GTO(exponent, 3, 1, 0, -glm::sqrt(5.0 / 28.0)), GTO(exponent, 1, 3, 0, -glm::sqrt(5.0 / 28.0))};
			else if (m == -1) return std::vector<GTO>{GTO(exponent, 0, 1, 3, glm::sqrt(10.0 / 7.0)), GTO(exponent, 0, 3, 1, -glm::sqrt(45.0 / 56.0)), GTO(exponent, 2, 1, 1, -glm::sqrt(9.0 / 56.0))};
			else if (m == 0)  return std::vector<GTO>{GTO(exponent, 0, 0, 4, 1.0), GTO(exponent, 4, 0, 0, glm::sqrt(9.0 / 64.0)), GTO(exponent, 0, 4, 0, glm::sqrt(9.0 / 64.0)), GTO(exponent, 2, 0, 2, -glm::sqrt(27.0 / 35.0)), GTO(exponent, 0, 2, 2, -glm::sqrt(27.0 / 35.0)), GTO(exponent, 2, 2, 0, glm::sqrt(1.0 / 16.0))};
			else if (m == 1)  return std::vector<GTO>{GTO(exponent, 1, 0, 3, glm::sqrt(10.0 / 7.0)), GTO(exponent, 3, 0, 1, -glm::sqrt(45.0 / 56.0)), GTO(exponent, 1, 2, 1, -glm::sqrt(9.0 / 56.0))};
			else if (m == 2)  return std::vector<GTO>{GTO(exponent, 2, 0, 2, glm::sqrt(27.0 / 28.0)), GTO(exponent, 0, 2, 2, -glm::sqrt(27.0 / 28.0)), GTO(exponent, 4, 0, 0, -glm::sqrt(5.0 / 16.0)), GTO(exponent, 0, 4, 0, glm::sqrt(5.0 / 16.0))};
			else if (m == 3)  return std::vector<GTO>{GTO(exponent, 3, 0, 1, glm::sqrt(5.0 / 8.0)), GTO(exponent, 1, 2, 1, -glm::sqrt(9.0 / 8.0))};
			else              return std::vector<GTO>{GTO(exponent, 4, 0, 0, glm::sqrt(35.0) / 8.0), GTO(exponent, 0, 4, 0, glm::sqrt(35.0) / 8.0), GTO(exponent, 2, 2, 0, -glm::sqrt(27.0 / 16.0))}; 
		}
		return std::vector<GTO>{};
	}

	GTO generateCartesianGTO(double exponent, int x, int y, int z) {
		GTO result;
		result.e_r = exponent;
		result.e_x = x;
		result.e_y = y;
		result.e_z = z;
		result.coeff = 1.0;
		return result;
	}

	CubeMap::CubeMap(glm::ivec3 resolution) {
		resize(resolution);
	}

	void CubeMap::resize(glm::ivec3 resolution) {
		resolution = glm::max(resolution, glm::ivec3(4));
		texture.resize(resolution.x, resolution.y, resolution.z);
	}

	float CubeMap::sample(glm::ivec3 coord) {
		coord = glm::clamp(coord, glm::ivec3(0, 0, 0), glm::ivec3(texture.width - 1, texture.height - 1, texture.depth - 1));
		return texture.data[4 * (coord.x + (coord.y + coord.z * texture.height) * texture.width)];
	}

	glm::vec3 CubeMap::sampleGradient(glm::ivec3 coord) {
		return glm::vec3(
			sample(coord + glm::ivec3(1, 0, 0)) - sample(coord + glm::ivec3(-1, 0, 0)),
			sample(coord + glm::ivec3(0, 1, 0)) - sample(coord + glm::ivec3(0, -1, 0)),
			sample(coord + glm::ivec3(0, 0, 1)) - sample(coord + glm::ivec3(0, 0, -1))
			);
	}

	double ContractedBasis::sample(glm::dvec3 r) {
		r -= origin;

		glm::dvec4 polynomial_terms[10];
		polynomial_terms[0] = glm::dvec4(1.0);
		polynomial_terms[1] = glm::dvec4(r, glm::length(r));

		for (int i = 2; i < 10; ++i) {
			polynomial_terms[i] = polynomial_terms[i - 1] * polynomial_terms[1];
		}

		double psi = 0.;
		double r2 = r.x * r.x + r.y * r.y + r.z * r.z;
		for (int i = 0; i < gto_primitives.size(); ++i) {
			GTO& p = gto_primitives[i];
			double psi_r = glm::exp(-p.e_r * r2);
			psi += p.coeff * psi_r * polynomial_terms[p.e_x].x * polynomial_terms[p.e_y].y * polynomial_terms[p.e_z].z;
		}
		double R = glm::length(r);
		for (int i = 0; i < sto_primitives.size(); ++i) {
			STO& p = sto_primitives[i];
			double psi_r = glm::exp(-2.0 * p.alpha * R);
			psi += p.coeff * psi_r * polynomial_terms[p.e_r].w * polynomial_terms[p.e_x].x * polynomial_terms[p.e_y].y * polynomial_terms[p.e_z].z;
		}
		return psi;
	}

	void MolecularOrbital::writeCubeSlice(CubeMap& map, uint z_min, uint z_max, bool print_progress) const {
		glm::ivec3 dimensions = glm::ivec3(map.texture.width, map.texture.height, map.texture.depth);

		int ao_count = glm::min(lcao_coefficients.size(), basis->size());
		for (int i = 0; i < ao_count; ++i) {
			if (print_progress) std::cout << "Cubemap: AO " << (i + 1) << "/" << ao_count << '\n';

			ContractedBasis& b = (*basis)[i];
			double coeff = lcao_coefficients[i];
			double radius = 0.0;
			for (STO primitive : b.sto_primitives) {
				double r = 2.5 / primitive.alpha + (double)(glm::max(primitive.e_x, glm::max(primitive.e_y, primitive.e_z)) * primitive.e_r);
				if (r > radius) radius = r;
			}
			for (GTO primitive : b.gto_primitives) {
				double r = 2.5 / glm::sqrt(primitive.e_r) + (double)glm::max(primitive.e_x, glm::max(primitive.e_y, primitive.e_z));
				if (r > radius) radius = r;
			}
			glm::ivec3 min = glm::clamp(glm::ivec3(((b.origin - radius - map.origin) / map.size) * (glm::dvec3)dimensions), glm::ivec3(0, 0, z_min), glm::ivec3(dimensions.x, dimensions.y, z_max));
			glm::ivec3 max = glm::clamp(glm::ivec3(((b.origin + radius - map.origin) / map.size) * (glm::dvec3)dimensions), glm::ivec3(0, 0, z_min), glm::ivec3(dimensions.x, dimensions.y, z_max));

			for (int z = min.z; z < max.z; ++z) {
				for (int y = min.y; y < max.y; ++y) {
					for (int x = min.x; x < max.x; ++x) {
						glm::dvec3 p = map.origin + map.size * glm::dvec3(x + 0.5, y + 0.5, z + 0.5) / glm::dvec3(dimensions);
						double phi = b.sample(p);

						map.texture.data[4 * (x + map.texture.width * (y + map.texture.height * z))] += coeff * phi;
					}
				}
			}
		}
	}

	void MolecularOrbital::threadWriteCubeSlice(MolecularOrbital* mo, CubeMap* cubemap, uint z_min, uint z_max) {
		mo->writeCubeSlice(*cubemap, z_min, z_max);
	}

	void fitCubeMap(CubeMap& map, std::vector<ContractedBasis>& basis) {
		map.origin = basis[0].origin;
		map.size = basis[0].origin;

		for (int i = 1; i < basis.size(); ++i) {
			glm::dvec3 pos = basis[i].origin;
			if (pos.x < map.origin.x) map.origin.x = pos.x;
			if (pos.y < map.origin.y) map.origin.y = pos.y;
			if (pos.z < map.origin.z) map.origin.z = pos.z;
			if (pos.x > map.size.x) map.size.x = pos.x;
			if (pos.y > map.size.y) map.size.y = pos.y;
			if (pos.z > map.size.z) map.size.z = pos.z;
		}

		map.size -= map.origin;
		map.size += 2.0 * settings.cubemap_clearance;
		map.origin -= settings.cubemap_clearance;

		if (resize_cubemap) map.resize(glm::ivec3((glm::vec3)map.size * settings.cubemap_density));
	}

	void generateSliceVertexArrays(std::vector<fgr::VertexArray>& vas, uint total_slices) {
		vas.resize(settings.cubemap_slice_count);
		for (int i = 0; i < vas.size(); ++i) {
			fgr::VertexArray& va = vas[i];

			va.init();

			uint start_slice = i * total_slices / (settings.cubemap_slice_count);
			uint end_slice = (i + 1) * total_slices / (settings.cubemap_slice_count);

			va.vertices.resize(6 * (end_slice - start_slice));
			for (int z = 0; z < end_slice - start_slice; ++z) {
				float zp = (float)(z + start_slice);
				va.vertices[z * 6    ] = fgr::Vertex(glm::vec3(-1.0, -1.0, zp), glm::vec2(), glm::vec4());
				va.vertices[z * 6 + 1] = fgr::Vertex(glm::vec3( 1.0, -1.0, zp), glm::vec2(), glm::vec4());
				va.vertices[z * 6 + 2] = fgr::Vertex(glm::vec3(-1.0,  1.0, zp), glm::vec2(), glm::vec4());
				va.vertices[z * 6 + 3] = fgr::Vertex(glm::vec3(-1.0,  1.0, zp), glm::vec2(), glm::vec4());
				va.vertices[z * 6 + 4] = fgr::Vertex(glm::vec3( 1.0, -1.0, zp), glm::vec2(), glm::vec4());
				va.vertices[z * 6 + 5] = fgr::Vertex(glm::vec3( 1.0,  1.0, zp), glm::vec2(), glm::vec4());
			}

			va.update();
		}
	}

	void drawSlicesToFBO(std::vector<fgr::VertexArray>& vas, fgr::RenderTarget& fbo, fgr::Shader& shader, fgr::ComputeShader& compute, CubeMap& cubemap) {
#if USE_COMPUTE_SHADERS
		compute.bindImage(0, cubemap.texture.id, true, true, true, GL_RGBA16F);
		compute.dispatch();
#else
		fbo.bind();
		for (auto va : vas)
			va.draw(shader);
		fbo.unbind();
#endif
	}

	void setGTOUniforms(glm::vec3* origins, glm::vec3* exponents3, float* alpha, float* coeffs, uint primitive_count) {
#if USE_COMPUTE_SHADERS
		gto_compute.setVec3Array(2, origins, primitive_count);
		gto_compute.setVec3Array(3, exponents3, primitive_count);
		gto_compute.setFloatArray(4, alpha, primitive_count);
		gto_compute.setFloatArray(5, coeffs, primitive_count);
#else
		gto_shader.setVec3Array(3, origins, primitive_count);
		gto_shader.setVec3Array(4, exponents3, primitive_count);
		gto_shader.setFloatArray(5, alpha, primitive_count);
		gto_shader.setFloatArray(6, coeffs, primitive_count);
#endif
	}

	void setSTOUniforms(glm::vec3* origins, glm::vec4* exponents4, float* alpha, float* coeffs, uint primitive_count) {
#if USE_COMPUTE_SHADERS
		sto_compute.setVec3Array(2, origins, primitive_count);
		sto_compute.setVec4Array(3, exponents4, primitive_count);
		sto_compute.setFloatArray(4, alpha, primitive_count);
		sto_compute.setFloatArray(5, coeffs, primitive_count);
#else
		sto_shader.setVec3Array(3, origins, primitive_count);
		sto_shader.setVec4Array(4, exponents4, primitive_count);
		sto_shader.setFloatArray(5, alpha, primitive_count);
		sto_shader.setFloatArray(6, coeffs, primitive_count);
#endif
	}

	void loadShader(bool sto, CubeMap& cubemap) {
#if USE_COMPUTE_SHADERS
		if (sto) {
			if (!sto_compute.loaded) {
				sto_compute = fgr::ComputeShader("shaders/volumol/sto.comp", std::vector<std::string>{
					"cubemap_origin",	// 0
					"cubemap_size",		// 1
					"origin",			// 2
					"exponents",		// 3
					"alpha",			// 4
					"coeff",			// 5
				});
				sto_shader.compile();
			}

			sto_compute.setVec3(0, cubemap.origin);
			sto_compute.setVec3(1, cubemap.size);
			sto_compute.work_group_count = glm::uvec3((glm::ivec3(cubemap.texture.width, cubemap.texture.height, cubemap.texture.depth) + 3) / 4);
		}
		else {
			if (!gto_compute.loaded) {
				gto_compute = fgr::ComputeShader("shaders/volumol/gto.comp", std::vector<std::string>{
					"cubemap_origin",	// 0
					"cubemap_size",		// 1
					"origin",			// 2
					"exponents",		// 3
					"alpha",			// 4
					"coeff",			// 5
				});
				gto_compute.compile();
			}

			gto_compute.setVec3(0, cubemap.origin);
			gto_compute.setVec3(1, cubemap.size);
			gto_compute.work_group_count = glm::uvec3((glm::ivec3(cubemap.texture.width, cubemap.texture.height, cubemap.texture.depth) + 3) / 4);
		}
#else
		if (sto) {
			if (!sto_shader.loaded) {
				sto_shader = fgr::Shader("shaders/volumol/gto.vert", "shaders/volumol/sto.frag", "shaders/volumol/gto.geom", std::vector<std::string>{
					"cubemap_origin",	// 0
					"cubemap_size",		// 1
					"layer_count",		// 2
					"origin",			// 3
					"exponents",		// 4
					"alpha",			// 5
					"coeff",			// 6
				});
				sto_shader.compile();
			}

			sto_shader.setVec3(0, cubemap.origin);
			sto_shader.setVec3(1, cubemap.size);
			sto_shader.setInt(2, cubemap.texture.depth);
		}
		else {
			if (!gto_shader.loaded) {
				gto_shader = fgr::Shader("shaders/volumol/gto.vert", "shaders/volumol/gto.frag", "shaders/volumol/gto.geom", std::vector<std::string>{
					"cubemap_origin",	// 0
					"cubemap_size",		// 1
					"layer_count",		// 2
					"origin",			// 3
					"exponents",		// 4
					"alpha",			// 5
					"coeff",			// 6
				});
				gto_shader.compile();
			}

			gto_shader.setVec3(0, cubemap.origin);
			gto_shader.setVec3(1, cubemap.size);
			gto_shader.setInt(2, cubemap.texture.depth);
		}
#endif
	}

	void MolecularOrbital::writeCubeMap(CubeMap& map, bool print_progress) {
		if (!basis) return;
		if (!basis->size()) return;

		fitCubeMap(map, *basis);

		glm::ivec3 dimensions = glm::ivec3(map.texture.width, map.texture.height, map.texture.depth);

		if (settings.cubemap_use_gpu) {

			loadShader(use_stos, map);

			if (print_progress) {
#if USE_COMPUTE_SHADERS
				std::cout << "Using compute shaders for rendering\n";
#else
				std::cout << "Using geometry shaders for rendering\n";
#endif
			}

			std::vector<fgr::VertexArray> vas;
			generateSliceVertexArrays(vas, dimensions.z);

			if (!map.texture.id) {
				map.texture.createBuffer(GL_CLAMP_TO_BORDER, GL_LINEAR);
			}

			fgr::RenderTarget fbo = map.texture.createFrameBuffer();
			fbo.clear(glm::vec4(0.), false);

			fgr::setBlending(fgr::Blending::none);
			fgr::setDepthTesting(false);

			constexpr uint primitive_count = 16;
			glm::vec3 origins[primitive_count];
			glm::vec3 exponents3[primitive_count];
			glm::vec4 exponents4[primitive_count];
			float alpha[primitive_count];
			float coeffs[primitive_count];
			int index = 0;

			int ao_count = glm::min(lcao_coefficients.size(), basis->size());

			if (use_stos) {
				for (int i = 0; i < ao_count; ++i) {
					ContractedBasis& b = (*basis)[i];
					double coeff = lcao_coefficients[i];

					for (STO sto : b.sto_primitives) {
						exponents4[index] = glm::vec4(sto.e_x, sto.e_y, sto.e_z, sto.e_r);
						alpha[index] = sto.alpha;
						coeffs[index] = sto.coeff * coeff;
						origins[index] = b.origin;
						++index;

						if (index == primitive_count) {
							index = 0;

							setSTOUniforms(origins, exponents4, alpha, coeffs, primitive_count);

							if (print_progress) std::cout << "Cubemap: AO " << (i + 1) << "/" << ao_count << '\n';

							drawSlicesToFBO(vas, fbo, sto_shader, sto_compute, map);

							fgr::setBlending(fgr::Blending::additive);
						}
					}
				}

				if (index) {
					for (int i = index; i < primitive_count; ++i) {
						origins[i] = glm::vec3(0.0);
						exponents4[i] = glm::vec4(0.0);
						alpha[i] = 0.f;
						coeffs[i] = 0.f;
					}

					setSTOUniforms(origins, exponents4, alpha, coeffs, primitive_count);

					drawSlicesToFBO(vas, fbo, sto_shader, sto_compute, map);
				}
			}
			else {
				for (int i = 0; i < ao_count; ++i) {
					ContractedBasis& b = (*basis)[i];
					double coeff = lcao_coefficients[i];

					for (GTO gto : b.gto_primitives) {
						exponents3[index] = glm::vec3(gto.e_x, gto.e_y, gto.e_z);
						alpha[index] = gto.e_r;
						coeffs[index] = gto.coeff * coeff;
						origins[index] = b.origin;
						++index;

						if (index == primitive_count) {
							index = 0;

							setGTOUniforms(origins, exponents3, alpha, coeffs, primitive_count);

							if (print_progress) std::cout << "Cubemap: AO " << (i + 1) << "/" << ao_count << '\n';

							drawSlicesToFBO(vas, fbo, gto_shader, gto_compute, map);

							fgr::setBlending(fgr::Blending::additive);
						}
					}
				}

				if (index) {
					for (int i = index; i < primitive_count; ++i) {
						origins[i] = glm::vec3(0.0);
						exponents3[i] = glm::vec3(0.0);
						alpha[i] = 0.f;
						coeffs[i] = 0.f;
					}

					setGTOUniforms(origins, exponents3, alpha, coeffs, primitive_count);

					drawSlicesToFBO(vas, fbo, gto_shader, gto_compute, map);
				}
			}

			fgr::setBlending(fgr::Blending::linear);

			map.texture.loadFromID(map.texture.id);
		}
		else {
			for (int z = 0; z < dimensions.z; ++z) {
				for (int y = 0; y < dimensions.y; ++y) {
					for (int x = 0; x < dimensions.x; ++x) {
						map.texture.data[4 * (x + dimensions.x * (y + dimensions.y * z))] = 0.0;
						map.texture.data[4 * (x + dimensions.x * (y + dimensions.y * z)) + 1] = 0.0;
						map.texture.data[4 * (x + dimensions.x * (y + dimensions.y * z)) + 2] = 0.0;
						map.texture.data[4 * (x + dimensions.x * (y + dimensions.y * z)) + 3] = 0.0;
					}
				}
			}

			const uint thread_count = settings.cubemap_slice_count;

			std::vector<std::unique_ptr<std::thread>> threads(thread_count - 1);

			for (int i = 0; i < thread_count - 1; ++i) {
				uint min = map.texture.depth * i / thread_count;
				uint max = map.texture.depth * (i + 1) / thread_count;
				threads[i] = std::make_unique<std::thread>(&MolecularOrbital::threadWriteCubeSlice, this, &map, min, max);
			}

			writeCubeSlice(map, map.texture.depth * (thread_count - 1) / thread_count, map.texture.depth, true);

			std::cout << "Thread 1 finished\n";

			for (int i = 0; i < thread_count - 1; ++i) {
				threads[i]->join();
				std::cout << "Thread " << i + 2 << " finished\n";
			}

			if (!map.texture.id) {
				map.texture.createBuffer(GL_CLAMP_TO_BORDER, GL_LINEAR);
			}
			else {
				map.texture.syncTexture();
			}
		}
	}

	void resizeCubeMap(uint x, uint y, uint z) {
		if (!x && !y && !z) {
			resize_cubemap = true;
			return;
		}
		cubemap.resize(glm::ivec3(x, y, z));
		resize_cubemap = false;
	}

	void MOCubeMap(uint orbital) {
		if (orbital >= mos.size()) return;
		mos[orbital].writeCubeMap(cubemap);
	}

	void densityCubeMapMO() {
		CubeMap psi_map;
		uint size = 0;
		if (!resize_cubemap) psi_map.resize(glm::ivec3(cubemap.texture.width, cubemap.texture.height, cubemap.texture.depth));
		else fitCubeMap(psi_map, basis_set);
		cubemap.resize(glm::ivec3(psi_map.texture.width, psi_map.texture.height, psi_map.texture.depth));
		size = psi_map.texture.width * psi_map.texture.height * psi_map.texture.depth;

		bool inited = false;

		std::vector<fgr::VertexArray> vas;
		fgr::RenderTarget fbo;
		if (settings.cubemap_use_gpu) {
#if USE_COMPUTE_SHADERS
			std::cout << "Using compute shaders for rendering\n";
			if (!density_compute.loaded) {
				density_compute = fgr::ComputeShader("shaders/volumol/density.comp", std::vector<std::string>{"occupation"});
				density_compute.compile();
				density_compute.work_group_count = glm::uvec3((glm::ivec3(cubemap.texture.width, cubemap.texture.height, cubemap.texture.depth) + 3) / 4);
			}
#else
			std::cout << "Using geometry shaders for rendering\n";
			if (!density_shader.loaded) {
				density_shader = fgr::Shader("shaders/volumol/density.vert", "shaders/volumol/density.frag", "shaders/volumol/density.geom", std::vector<std::string>{"layer_count", "orbital", "occupation"});
				density_shader.compile();
			}

			density_shader.setInt(0, cubemap.texture.depth);
			density_shader.setInt(1, fgr::TextureUnit::texture0);

			generateSliceVertexArrays(vas, cubemap.texture.depth);
#endif

			cubemap.texture.createBuffer(GL_CLAMP_TO_BORDER, GL_LINEAR);
			fbo = cubemap.texture.createFrameBuffer();
			fbo.clear(glm::vec4(0.), false);
		}

		for (MolecularOrbital& mo : mos) {
			if (mo.occupation < 0.001 && mo.occupation > -0.001) continue;
			float occuption = mo.occupation;

			std::cout << "Rendering MO " << mo.name << '\n';

			mo.writeCubeMap(psi_map, false);

			if (!inited) {
				cubemap.origin = psi_map.origin;
				cubemap.size = psi_map.size;
				if (!settings.cubemap_use_gpu) {
					for (int i = 0; i < 4 * size; ++i) {
						cubemap.texture.data[i] = 0.f;
					}
				}
				inited = true;
			}

			if (settings.cubemap_use_gpu) {
				fgr::setBlending(fgr::Blending::additive);
				psi_map.texture.bindToUnit(fgr::TextureUnit::texture0);
#if USE_COMPUTE_SHADERS
				density_compute.setFloat(0, mo.occupation);
				density_compute.bindImage(1, psi_map.texture.id, false, true, true, GL_RGBA16F);
#else
				density_shader.setFloat(2, mo.occupation);
#endif
				drawSlicesToFBO(vas, fbo, density_shader, density_compute, cubemap);
				fgr::setBlending(fgr::Blending::linear);
			}
			else {
				for (int i = 0; i < size; ++i) {
					float& psi = psi_map.texture.data[4 * i];
					cubemap.texture.data[4 * i] += occuption * psi * psi;
				}
			}
		}
		if (settings.cubemap_use_gpu) {
			cubemap.texture.loadFromID(cubemap.texture.id);
		}
		else {
			cubemap.texture.syncTexture();
		}
		cubemap.texture.createBuffer(GL_CLAMP_TO_BORDER, GL_LINEAR);
	}

	uint findLUMO() {
		double lowest_energy = 1000000000000000.;
		uint result = mos.size();
		for (uint i = 0; i < mos.size(); ++i) {
			MolecularOrbital& mo = mos[i];
			if (mo.occupation < 0.5 && mo.energy < lowest_energy) {
				lowest_energy = mo.energy;
				result = i;
			}
		}
		return result;
	}

	uint MOcount() {
		return mos.size();
	}

	MolecularOrbital empty_mo;

	MolecularOrbital& getMO(uint number) {
		if (number >= mos.size()) return empty_mo;
		return mos[number];
	}
}
