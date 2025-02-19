#include "MeshGenerator.h"

namespace mol {
	glm::vec3 icosahedron_vertices[12] = {
		glm::vec3(    0.0,    0.0,    1.0), // 0 top
		glm::vec3( 0.8944,    0.0, 0.4472), // 1
		glm::vec3( 0.2764,-0.8506, 0.4472), // 2
		glm::vec3(-0.7236,-0.5257, 0.4472), // 3
		glm::vec3(-0.7236, 0.5257, 0.4472), // 4
		glm::vec3( 0.2764, 0.8506, 0.4472), // 5
		glm::vec3( 0.7236,-0.5257,-0.4472), // 6
		glm::vec3(-0.2764,-0.8506,-0.4472), // 7
		glm::vec3(-0.8944,    0.0,-0.4472), // 8
		glm::vec3(-0.2764, 0.8506,-0.4472), // 9
		glm::vec3( 0.7236, 0.5257,-0.4472), // 10
		glm::vec3(    0.0,    0.0,   -1.0)  // 11 bottom
	};

	uint icosahedron_indices[60] = {
		1, 0, 2,
		2, 0, 3,
		3, 0, 4,
		4, 0, 5,
		5, 0, 1,

		1, 2,  6,
		2, 3,  7,
		3, 4,  8,
		4, 5,  9,
		5, 1, 10,

		 6, 2,  7,
		 7, 3,  8,
		 8, 4,  9,
		 9, 5, 10,
		10, 1,  6,

		 6,  7, 11,
		 7,  8, 11,
		 8,  9, 11,
		 9, 10, 11,
		10,  6, 11
	};

	void generateIsosphere(Mesh& mesh, uint subdivisions) {
		mesh.vertices.resize(12);
		for (int i = 0; i < 12; ++i) {
			mesh.vertices[i] = Vertex3D(icosahedron_vertices[i], glm::vec3(1.0, 0.0, 0.0), glm::vec2(0.));
		}
		mesh.indices.resize(60);
		for (int i = 0; i < 60; ++i) {
			mesh.indices[i] = icosahedron_indices[i];
		}

		for (int i = 0; i < subdivisions; ++i) {
			std::vector<uint> new_indices(mesh.indices.size() * 4);
			const uint s = mesh.vertices.size();
			std::vector<glm::uvec2> pairs;

			for (int i = 0; i < mesh.indices.size() / 3; ++i) {
				const uint i0 = mesh.indices[i * 3];
				const uint i1 = mesh.indices[i * 3 + 1];
				const uint i2 = mesh.indices[i * 3 + 2];

				uint i3 = 0;
				uint i4 = 0;
				uint i5 = 0;

				for (int j = 0; j < pairs.size(); ++j) {
					if ((pairs[j].x == i0 && pairs[j].y == i1) || (pairs[j].x == i1 && pairs[j].y == i0)) i3 = s + j;
					if ((pairs[j].x == i1 && pairs[j].y == i2) || (pairs[j].x == i2 && pairs[j].y == i1)) i4 = s + j;
					if ((pairs[j].x == i2 && pairs[j].y == i0) || (pairs[j].x == i0 && pairs[j].y == i2)) i5 = s + j;
					if (i3 && i4 && i5) break;
				}

				if (!i3) {
					i3 = mesh.vertices.size();
					mesh.vertices.push_back(Vertex3D(glm::normalize(mesh.vertices[i0].position + mesh.vertices[i1].position), glm::vec3(1.0, 0.0, 0.0), glm::vec2(0.0)));
					pairs.push_back(glm::uvec2(i0, i1));
				}
				if (!i4) {
					i4 = mesh.vertices.size();
					mesh.vertices.push_back(Vertex3D(glm::normalize(mesh.vertices[i1].position + mesh.vertices[i2].position), glm::vec3(1.0, 0.0, 0.0), glm::vec2(0.0)));
					pairs.push_back(glm::uvec2(i1, i2));
				}
				if (!i5) {
					i5 = mesh.vertices.size();
					mesh.vertices.push_back(Vertex3D(glm::normalize(mesh.vertices[i2].position + mesh.vertices[i0].position), glm::vec3(1.0, 0.0, 0.0), glm::vec2(0.0)));
					pairs.push_back(glm::uvec2(i2, i0));
				}

				new_indices[i * 12]     = i3;
				new_indices[i * 12 + 1] = i4;
				new_indices[i * 12 + 2] = i5;

				new_indices[i * 12 + 3] = i0;
				new_indices[i * 12 + 4] = i3;
				new_indices[i * 12 + 5] = i5;

				new_indices[i * 12 + 6] = i1;
				new_indices[i * 12 + 7] = i4;
				new_indices[i * 12 + 8] = i3;
				
				new_indices[i * 12 + 9]  = i2;
				new_indices[i * 12 + 10] = i5;
				new_indices[i * 12 + 11] = i4;
			}

			mesh.indices = new_indices;
		}

		mesh.generateNormals();
		mesh.update();
	}

	void generateCylinder(Mesh& mesh, uint resolution, float thickness, bool hard_cut) {
		std::vector<glm::vec2> base_vertices(resolution);
		for (int i = 0; i < resolution; ++i) {
			float angle = (float)i / (float)resolution * 3.141592653589 * 2.0;
			base_vertices[i] = glm::vec2(glm::cos(angle), glm::sin(angle)) * thickness;
		}

		if (hard_cut) {
			mesh.vertices.resize(resolution * 4);
			mesh.indices.resize(resolution * 12);
			for (int i = 0; i < resolution; ++i) {
				mesh.vertices[i]                  = Vertex3D(glm::vec3(base_vertices[i], 0.), glm::vec3(1.0, 0.0, 0.0), glm::vec2(0.0));
				mesh.vertices[i + resolution]     = Vertex3D(glm::vec3(base_vertices[i], 0.5), glm::vec3(1.0, 0.0, 0.0), glm::vec2(0.0));
				mesh.vertices[i + resolution * 2] = Vertex3D(glm::vec3(base_vertices[i], 0.5), glm::vec3(0.0, 1.0, 0.0), glm::vec2(0.0));
				mesh.vertices[i + resolution * 3] = Vertex3D(glm::vec3(base_vertices[i], 1.), glm::vec3(0.0, 1.0, 0.0), glm::vec2(0.0));

				mesh.indices[i * 12]      = i;
				mesh.indices[i * 12 + 1]  = (i + 1) % resolution;
				mesh.indices[i * 12 + 2]  = i + resolution;
				mesh.indices[i * 12 + 3]  = (i + 1) % resolution;
				mesh.indices[i * 12 + 4]  = (i + 1) % resolution + resolution;
				mesh.indices[i * 12 + 5]  = i + resolution;
				mesh.indices[i * 12 + 6]  = i + resolution * 2;
				mesh.indices[i * 12 + 7]  = (i + 1) % resolution + resolution * 2;
				mesh.indices[i * 12 + 8]  = i + resolution * 3;
				mesh.indices[i * 12 + 9]  = (i + 1) % resolution + resolution * 2;
				mesh.indices[i * 12 + 10] = (i + 1) % resolution + resolution * 3;
				mesh.indices[i * 12 + 11] = i + resolution * 3;
			}
		}
		else {
			mesh.indices.resize(resolution * 6);
			mesh.vertices.resize(resolution * 2);
			for (int i = 0; i < resolution; ++i) {
				mesh.vertices[i] = Vertex3D(glm::vec3(base_vertices[i], 0.), glm::vec3(1.0, 0.0, 0.0), glm::vec2(0.0));
				mesh.vertices[i + resolution] = Vertex3D(glm::vec3(base_vertices[i], 1.0), glm::vec3(0.0, 1.0, 0.0), glm::vec2(0.0));

				mesh.indices[i * 6]     = i;
				mesh.indices[i * 6 + 1] = (i + 1) % resolution;
				mesh.indices[i * 6 + 2] = i + resolution;
				mesh.indices[i * 6 + 3] = (i + 1) % resolution;
				mesh.indices[i * 6 + 4] = (i + 1) % resolution + resolution;
				mesh.indices[i * 6 + 5] = i + resolution;
			}
		}

		mesh.generateNormals();
		mesh.update();
	}
}