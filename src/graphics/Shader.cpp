#include "Shader.h"

#include "GErrorHandler.h"

#include "Window.h"

#include "../logic/TextReading.h"

std::string executable_path = "";

namespace fgr {
	Shader Shader::basic, Shader::basic_instanced, Shader::line, Shader::line_instanced, Shader::textured, Shader::textured_instanced, Shader::sprites_instanced, Shader::sprites_instanced_depth;

	void init_shader_defaults() {
		Shader::basic = Shader("shaders/basic.vert", "shaders/basic.frag", std::vector<std::string>{"tint"});
		//Shader::basic_instanced = Shader("shaders/instanced/basic.vert", "shaders/basic.frag", std::vector<std::string>{});
		//Shader::line = Shader("shaders/line.vert", "shaders/basic.frag", "shaders/line.geom", std::vector<std::string>{"screen_dimensions", "line_thickness"});
		Shader::line_instanced = Shader("shaders/instanced/line.vert", "shaders/basic.frag", "shaders/line.geom", std::vector<std::string>{"screen_dimensions", "line_thickness"});
		//Shader::textured = Shader("shaders/textured.vert", "shaders/textured.frag", std::vector<std::string>{"texture"});
		//Shader::textured_instanced = Shader("shaders/instanced/textured.vert", "shaders/textured.frag", std::vector<std::string>{"texture"});
		//Shader::sprites_instanced = Shader("shaders/instanced/sprite.vert", "shaders/instanced/sprite.frag", std::vector<std::string>{"textures"});
		//Shader::sprites_instanced_depth = Shader("shaders/instanced/sprite.vert", "shaders/instanced/depthsprite.frag", std::vector<std::string>{"textures"});

		Shader::basic.compile();
		//Shader::basic_instanced.compile();
		//Shader::line.compile();
		//Shader::line_instanced.compile();
		Shader::textured.compile();
		//Shader::textured_instanced.compile();
		//Shader::sprites_instanced.compile();
		//Shader::sprites_instanced_depth.compile();
		Shader::basic.setVec4(0, glm::vec4(1.));
	}

	Shader::Shader(const Shader& copy) {
		*this = copy;
	}

	void Shader::operator=(const Shader& other) {
		if (&other == this) return;

		dispose();

		vertex_code = other.vertex_code;
		fragment_code = other.fragment_code;
		geometry_code = other.geometry_code;

		uniform_names = other.uniform_names;

		if (other.loaded) compile(other.settings);
	}

	Shader::Shader(const std::string& vertex_path, const std::string& fragment_path, const std::vector<std::string>& uniforms) : uniform_names(uniforms) {
		vertex_code = flo::readFullFile(executable_path + vertex_path);
		if (!vertex_code.size()) {
			std::cerr << "Vertex shader '" + executable_path + vertex_path + "' not found\n";
			return;
		}
		vertex_code += '\0';
		fragment_code = flo::readFullFile(executable_path + fragment_path);
		if (!fragment_code.size()) {
			std::cerr << "Fragment shader '" + executable_path + fragment_path + "' not found\n";
			return;
		}
		fragment_code += '\0';
	}

	Shader::Shader(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path, const std::vector<std::string>& uniforms) : uniform_names(uniforms) {
		vertex_code = flo::readFullFile(executable_path + vertex_path);
		if (!vertex_code.size()) {
			std::cerr << "Vertex shader '" + executable_path + vertex_path + "' not found\n";
			return;
		}
		vertex_code += '\0';
		fragment_code = flo::readFullFile(executable_path + fragment_path);
		if (!fragment_code.size()) {
			std::cerr << "Fragment shader '" + executable_path + fragment_path + "' not found\n";
			return;
		}
		fragment_code += '\0';
		geometry_code = flo::readFullFile(executable_path + geometry_path);
		if (!geometry_code.size()) {
			std::cerr << "Geometry shader '" + executable_path + geometry_path + "' not found\n";
			return;
		}
		geometry_code += '\0';
	}

	bool Shader::compile(const std::string& _settings) {
		graphics_check_external();

		settings = _settings;

		if (shader_program) glDeleteProgram(shader_program);
		shader_program = 0;

		std::string code = vertex_code;
		for (int i = 0; i < code.size(); ++i) {
			if (code[i] == '\n') {
				code.insert(code.begin() + i + 1, settings.begin(), settings.end());
				break;
			}
		}
		char* code_ptr = code.data();
		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, &code_ptr, NULL);
		glCompileShader(vertex_shader);
		int success;
		char infoLog[512];
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			return true;
		}

		code = fragment_code;
		for (int i = 0; i < code.size(); ++i) {
			if (code[i] == '\n') {
				code.insert(code.begin() + i + 1, settings.begin(), settings.end());
				break;
			}
		}
		code_ptr = code.data();
		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &code_ptr, NULL);
		glCompileShader(fragment_shader);
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
			return true;
		}

		if (geometry_code.size()) {
			code = geometry_code;
			for (int i = 0; i < code.size(); ++i) {
				if (code[i] == '\n') {
					code.insert(code.begin() + i + 1, settings.begin(), settings.end());
					break;
				}
			}
			code_ptr = code.data();
			geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry_shader, 1, &code_ptr, NULL);
			glCompileShader(geometry_shader);
			glGetShaderiv(geometry_shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(geometry_shader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
				return true;
			}
		}

		shader_program = glCreateProgram();
		glAttachShader(shader_program, vertex_shader);
		glAttachShader(shader_program, fragment_shader);
		if (geometry_code.size()) glAttachShader(shader_program, geometry_shader);
		glLinkProgram(shader_program);
		glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
			std::cerr << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
			return true;
		}

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		glDeleteShader(geometry_shader);

		uniform_locations.resize(uniform_names.size());
		for (int i = 0; i < uniform_names.size(); ++i) {
			uniform_locations[i] = glGetUniformLocation(shader_program, uniform_names[i].data());
		}
		transformations_uniform = glGetUniformLocation(shader_program, "transformations");

		loaded = true;

		graphics_check_error();

		return false;
	}
	
	void Shader::dispose() {
		if (!window::graphicsInitialized()) return;

		graphics_check_external();
		if (shader_program) glDeleteProgram(shader_program);
		shader_program = 0;
		graphics_check_error();

		loaded = false;
	}

	void Shader::use() {
		graphics_check_external();
		glUseProgram(shader_program);
		graphics_check_error();
	}

	void Shader::setInt(const int location, const int value) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform1i(uniform_locations[location], value);
	}

	void Shader::setFloat(const int location, const float value) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform1f(uniform_locations[location], value);
	}

	void Shader::setVec2(const int location, const glm::vec2 value) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform2fv(uniform_locations[location], 1, glm::value_ptr(value));
	}

	void Shader::setVec3(const int location, const glm::vec3 value) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform3fv(uniform_locations[location], 1, glm::value_ptr(value));
	}

	void Shader::setVec4(const int location, const glm::vec4 value) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform4fv(uniform_locations[location], 1, glm::value_ptr(value));
	}

	void Shader::setMat3(const int location, const glm::mat3 value, const bool transpose) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniformMatrix3fv(uniform_locations[location], 1, transpose, glm::value_ptr(value));
	}

	void Shader::setMat4(const int location, const glm::mat4 value, const bool transpose) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniformMatrix4fv(uniform_locations[location], 1, transpose, glm::value_ptr(value));
	}

	void Shader::setIntArray(const int location, const int* value, const int count) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform1iv(uniform_locations[location], count, value);
	}

	void Shader::setFloatArray(const int location, const float* value, const int count) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform1fv(uniform_locations[location], count, value);
	}

	void Shader::setVec2Array(const int location, const glm::vec2* value, const int count) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform2fv(uniform_locations[location], count, glm::value_ptr(value[0]));
	}

	void Shader::setVec3Array(const int location, const glm::vec3* value, const int count) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform3fv(uniform_locations[location], count, glm::value_ptr(value[0]));
	}

	void Shader::setVec4Array(const int location, const glm::vec4* value, const int count) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform4fv(uniform_locations[location], count, glm::value_ptr(value[0]));
	}

	void Shader::setMat3Array(const int location, const glm::mat3* value, const int count, const bool transpose) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniformMatrix3fv(uniform_locations[location], count, transpose, glm::value_ptr(value[0]));
	}

	void Shader::setMat4Array(const int location, const glm::mat4* value, const int count, const bool transpose) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniformMatrix4fv(uniform_locations[location], count, transpose, glm::value_ptr(value[0]));
	}

	Shader::~Shader() {
		dispose();
	}
}
