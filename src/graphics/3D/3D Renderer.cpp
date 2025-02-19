#include "3D Renderer.h"

#include "Meshloader.h"

#include "../GErrorHandler.h"
#include "../Window.h"

#include <fstream>

namespace fgr {
	Vertex3D::Vertex3D(const glm::vec3& position, const glm::vec3& color, const glm::vec2& texCoord, const glm::vec3& normal, const glm::vec3& tangent) : 
	position(position), color(color), tex_coord(texCoord), normal(normal), tangent(tangent) {

	}

	void View::setPerspective(const float fov, const float screen_width, const float screen_height, const float near, const float far) {
		projection = glm::perspective(fov, screen_width / screen_height, near, far);
	}

	void View::setOrthographic(const float width, const float height, const float zNear, const float zFar) {
		projection = glm::ortho(-0.5f * width, 0.5f * width, -0.5f * height, 0.5f * height, zNear, zFar);
	}

	void View::setOrientation(const glm::vec3& camera_pos, const glm::vec3& camera_front, const glm::vec3& camera_up) {
		view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
	}

	Mesh::Mesh(const Mesh& copy) {
		*this = copy;
	}

	void Mesh::operator=(const Mesh& other) {
		dispose();
		if (other.VAO) {
			init();
		}
		setMesh(other.vertices, other.indices);
		model_matrix = other.model_matrix;
	}

	void Mesh::init() {
		graphics_check_external();

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, color));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, tangent));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, tex_coord));
		glEnableVertexAttribArray(4);

		graphics_check_error();
	}

	void Mesh::setMesh(const std::vector<Vertex3D>& vertices, const std::vector<uint>& indices) {
		Mesh::vertices = vertices;
		Mesh::indices = indices;
		update();
	}

	void Mesh::mergeMesh(const Mesh& other, const glm::mat4& transform) {
		const uint start = vertices.size();

		vertices.reserve(vertices.size() + other.vertices.size());
		for (Vertex3D v : other.vertices) {
			v.position = (transform * glm::vec4(v.position, 1.0));
			vertices.push_back(v);
		}

		indices.reserve(indices.size() + other.indices.size());
		for (uint i : other.indices) {
			i += start;
			indices.push_back(i);
		}

		update();
	}

	void Mesh::loadFromOBJ(const std::string& path) {
		std::ifstream input;
		input.open(path, std::ios::ate | std::ios::binary);

		if (!input.is_open()) {
			std::cerr << "Could not load object file\n";
			return;
		}

		input.seekg(0, std::ios::end);
		uint size = input.tellg();
		input.seekg(0, std::ios::beg);
		flo::Array<char> data = (size);
		input.read(data.getPtr(), size);

		input.close();

		std::string str = std::string(data.getPtr(), size);
		meshloader::loadMesh(*this, str);

		update();
	}

	void Mesh::generateNormals(bool hard_shading) {
		for (int i = 0; i < indices.size(); ++i) {
			indices[i] %= vertices.size();
		}

		if (hard_shading) {
			std::vector<bool> vertex_used(vertices.size());

			for (int i = 0; i < indices.size(); ++i) {
				uint index = indices[i];
				if (vertex_used[index]) {
					indices[i] = vertices.size();
					vertices.push_back(vertices[index]);
				}
				else {
					vertex_used[index] = true;
				}
			}
		}

		for (Vertex3D& v : vertices) {
			v.normal = glm::vec3(0.);
			v.tangent = glm::vec3(0.);
		}

		for (int t = 0, i = 0; t < (int)indices.size() - 2; t += 3, ++i) {
			Vertex3D& v0 = vertices[indices[t]];
			Vertex3D& v1 = vertices[indices[t + 1]];
			Vertex3D& v2 = vertices[indices[t + 2]];

			glm::vec3 normal = glm::normalize(glm::cross(v1.position - v0.position, v2.position - v0.position));
			glm::mat2 texture_matrix = glm::mat2(
				v1.tex_coord - v0.tex_coord,
				v2.tex_coord - v0.tex_coord
			);
			glm::vec2 T = glm::inverse(texture_matrix) * glm::vec2(1.0, 0.0);
			glm::vec3 tangent = glm::normalize((v1.position - v0.position) * T.x + (v2.position - v0.position) * T.y);

			float weight0 = glm::acos(glm::dot(glm::normalize(v1.position - v0.position), glm::normalize(v2.position - v0.position)));
			v0.normal  += normal * weight0;
			v0.tangent += tangent * weight0;
			float weight1 = glm::acos(glm::dot(glm::normalize(v2.position - v1.position), glm::normalize(v0.position - v1.position)));
			v1.normal  += normal * weight1;
			v1.tangent += tangent * weight1;
			float weight2 = glm::acos(glm::dot(glm::normalize(v0.position - v2.position), glm::normalize(v1.position - v2.position)));
			v2.normal  += normal * weight2;
			v2.tangent += tangent * weight2;
		}

		for (Vertex3D& v : vertices) {
			v.normal = glm::normalize(v.normal);
			v.tangent = glm::normalize(v.tangent);
		}
	}

	void Mesh::update() {
		graphics_check_external();

		if (!vertices.size() || !indices.size() || !VAO) return;

		glBindVertexArray(VAO);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		if (vertices.size() > vertices_allocated) {
			vertices_allocated = (vertices.size() & ~1023) + 1024;
			glBufferData(GL_ARRAY_BUFFER, vertices_allocated * sizeof(Vertex3D), NULL, GL_DYNAMIC_DRAW);
		}
		if (indices.size() > indices_allocated) {
			indices_allocated = (indices.size() & ~1023) + 1024;
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_allocated * sizeof(Vertex3D), NULL, GL_DYNAMIC_DRAW);
		}

		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(uint), &indices[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex3D), &vertices[0]);

		glBindVertexArray(0);

		graphics_check_error();
	}

	void Mesh::render(Shader& shader, bool back_culling, bool front_culling) {
		graphics_check_external();

		glEnable(GL_DEPTH_TEST);
		if (front_culling || back_culling) {
			glEnable(GL_CULL_FACE);
			if (back_culling) glCullFace(front_culling ? GL_FRONT_AND_BACK : GL_BACK);
			else glCullFace(GL_BACK);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glUseProgram(shader.shader_program);

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glBindVertexArray(0);

		graphics_check_error();
	}

	void Mesh::dispose() {
		if (!window::graphicsInitialized()) return;

		graphics_check_external();

		if (!(VAO && EBO && VBO)) return;
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		glDeleteVertexArrays(1, &VAO);
		VBO = 0;
		EBO = 0;
		VAO = 0;

		graphics_check_error();
	}

	Mesh::~Mesh() {
		dispose();
	}
}
