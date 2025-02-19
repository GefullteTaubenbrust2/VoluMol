#include "VertexArray.h"

#include "Window.h"

#include "GErrorHandler.h"

namespace fgr {
	fgr::VertexArray default_rectangle_va;

	Vertex::Vertex(glm::vec3 position, glm::vec2 tex_coords, glm::vec4 color) : 
		position(position), tex_coords(tex_coords), color(color) {

	}

	Instance::Instance(glm::mat3 transformations, glm::mat3 tex_transform, glm::vec4 color) : 
		transformations(transformations), tex_transform(tex_transform), color(color) {

	}

	VertexArray::VertexArray(const VertexArray& copy) {
		*this = copy;
	}

	void VertexArray::operator=(const VertexArray& other) {
		dispose();

		dynamic_allocation = other.dynamic_allocation;
		transform = other.transform;
		vertices = other.vertices;
		vertices_allocated = 0;

		if (other.VAO) {
			init();
			update();
		}
	}

	void VertexArray::init() {
		graphics_check_external();

		if (VBO) return;

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
		glEnableVertexAttribArray(2);

		transform = glm::mat3(1.0);

		graphics_check_error();
	}

	void VertexArray::draw(const Shader& shader, const uint mode) {
		graphics_check_external();

		glBindVertexArray(VAO);
		glUseProgram(shader.shader_program);
		glUniformMatrix3fv(shader.transformations_uniform, 1, false, glm::value_ptr(transform));

		glDrawArrays(mode, 0, vertices.size());

		glBindVertexArray(0);

		graphics_check_error();
	}

	void VertexArray::update() {
		graphics_check_external();

		if (!vertices.size()) return;

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		if (dynamic_allocation) {
			if (vertices.size() > vertices_allocated) {
				if (!vertices_allocated) vertices_allocated = 1;
				while (vertices_allocated < vertices.size()) vertices_allocated *= 2;
				glBufferData(GL_ARRAY_BUFFER, vertices_allocated * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
			}
		}
		else {
			vertices_allocated = vertices.size();
			glBufferData(GL_ARRAY_BUFFER, vertices_allocated * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
		}

		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), &vertices[0]);

		glBindVertexArray(0);

		graphics_check_error();
	}

	void VertexArray::setTransformations(const glm::mat3& transform) {
		VertexArray::transform = transform;
	}

	void VertexArray::dispose() {
		if (!window::graphicsInitialized()) return;

		graphics_check_external();

		if (!(VAO && VBO)) return;
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
		VBO = 0;
		VAO = 0;

		graphics_check_error();
	}

	VertexArray::~VertexArray() {
		dispose();
	}

	InstanceArray::InstanceArray(const InstanceArray& copy) {
		*this = copy;
	}

	void InstanceArray::operator=(const InstanceArray& other) {
		dispose();

		instances = other.instances;
		dynamic_allocation = other.dynamic_allocation;
		instances_allocated = 0;
		va = other.va;

		if (other.VBO) {
			init();
			update();
		}
	}

	void InstanceArray::init() {
		graphics_check_external();

		if (VBO) return;

		va.init();

		glGenBuffers(1, &VBO);
		glBindVertexArray(va.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		const int size = sizeof(Instance);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, size, (void*)(offsetof(Instance, transformations)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, size, (void*)(offsetof(Instance, transformations) + sizeof(glm::vec3)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, size, (void*)(offsetof(Instance, transformations) + 2 * sizeof(glm::vec3)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, size, (void*)(offsetof(Instance, tex_transform)));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, size, (void*)(offsetof(Instance, tex_transform) + sizeof(glm::vec3)));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, size, (void*)(offsetof(Instance, tex_transform) + 2 * sizeof(glm::vec3)));
		glEnableVertexAttribArray(9);
		glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, size, (void*)(offsetof(Instance, color)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);
		glVertexAttribDivisor(9, 1);

		glBindVertexArray(0);

		graphics_check_error();
	}

	void InstanceArray::draw(const Shader& shader, const uint mode) {
		graphics_check_external();

		glBindVertexArray(va.VAO);
		glUseProgram(shader.shader_program);
		const glm::mat3 trans = va.transform;
		glUniformMatrix3fv(shader.transformations_uniform, 1, false, glm::value_ptr(va.transform));

		glDrawArraysInstanced(mode, 0, va.vertices.size(), glm::min((uint)instances.size(), instances_allocated));

		glBindVertexArray(0);

		graphics_check_error();
	}

	void InstanceArray::update() {
		graphics_check_external();

		if (!instances.size()) return;

		glBindVertexArray(va.VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		if (dynamic_allocation) {
			if (instances.size() > instances_allocated) {
				if (!instances_allocated) instances_allocated = 1;
				while (instances_allocated < instances.size()) instances_allocated *= 2;
				glBufferData(GL_ARRAY_BUFFER, instances_allocated * sizeof(Instance), NULL, GL_DYNAMIC_DRAW);
			}
		}
		else {
			instances_allocated = instances.size();
			glBufferData(GL_ARRAY_BUFFER, instances_allocated * sizeof(Instance), NULL, GL_DYNAMIC_DRAW);
		}

		glBufferSubData(GL_ARRAY_BUFFER, 0, instances.size() * sizeof(Instance), instances.data());

		glBindVertexArray(0);

		graphics_check_error();
	}


	void InstanceArray::dispose() {
		if (!window::graphicsInitialized()) return;

		graphics_check_external();
		va.dispose();
		if (VBO) glDeleteBuffers(1, &VBO);
		VBO = 0;
		graphics_check_error();
		instances.clear();
	}
	
	void drawRectangle(const glm::mat3& transform, fgr::Shader& shader) {
		default_rectangle_va.setTransformations(transform);
		default_rectangle_va.draw(shader, fgr::RendeMode::triangle_strip);
	}

	InstanceArray::~InstanceArray() {
		dispose();
	}
}
