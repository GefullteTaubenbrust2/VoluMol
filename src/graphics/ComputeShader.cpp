#include "ComputeShader.h"

#include "GErrorHandler.h"

#include "Window.h"

#include "../logic/TextReading.h"

extern std::string executable_path;

namespace fgr {
	ComputeShader::ComputeShader(const ComputeShader& copy) {
		*this = copy;
	}

	void ComputeShader::operator=(const ComputeShader& other) {
		if (&other == this) return;

		dispose();

		source = other.source;

		uniform_names = other.uniform_names;

		if (other.loaded) compile(other.settings);
	}

	ComputeShader::ComputeShader(const std::string& path, const std::vector<std::string>& uniforms) : uniform_names(uniforms) {
		source = flo::readFullFile(executable_path + path);
		if (!source.size()) {
			std::cerr << "Compute shader '" + executable_path + path + "' not found\n";
			return;
		}
		source += '\0';
	}

	bool ComputeShader::compile(const std::string& _settings) {
		graphics_check_external();

		settings = _settings;

		if (shader_program) glDeleteProgram(shader_program);
		shader_program = 0;

		std::string code = source;
		for (int i = 0; i < code.size(); ++i) {
			if (code[i] == '\n') {
				code.insert(code.begin() + i + 1, settings.begin(), settings.end());
				break;
			}
		}
		char* code_ptr = code.data();
		compute_shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute_shader, 1, &code_ptr, NULL);
		glCompileShader(compute_shader);
		int success;
		char infoLog[512];
		glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(compute_shader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;
			return true;
		}

		shader_program = glCreateProgram();
		glAttachShader(shader_program, compute_shader);
		glLinkProgram(shader_program);
		glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
			std::cerr << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
			return true;
		}

		glDeleteShader(compute_shader);

		uniform_locations.resize(uniform_names.size());
		for (int i = 0; i < uniform_names.size(); ++i) {
			uniform_locations[i] = glGetUniformLocation(shader_program, uniform_names[i].data());
		}

		loaded = true;

		graphics_check_error();

		return false;
	}

	void ComputeShader::dispose() {
		if (!window::graphicsInitialized()) return;

		graphics_check_external();
		if (shader_program) glDeleteProgram(shader_program);
		shader_program = 0;
		graphics_check_error();

		loaded = false;
	}

	void ComputeShader::dispatch() {
		graphics_check_external();

		int x_max = 0, y_max = 0, z_max = 0;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &x_max);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &y_max);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &z_max);

		work_group_count = glm::min(work_group_count, glm::uvec3(x_max, y_max, z_max));

		glUseProgram(shader_program);
		glDispatchCompute(work_group_count.x, work_group_count.y, work_group_count.z);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		graphics_check_error();
	}

	void ComputeShader::bindImage(uint unit, uint texture_id, bool write, bool read, bool layered, uint format) {
		graphics_check_external();
		glBindImageTexture(unit, texture_id, 0, layered, 0, write ? (read ? GL_READ_WRITE : GL_WRITE_ONLY) : GL_READ_ONLY, format);
		graphics_check_error();
	}

	void ComputeShader::setInt(const int location, const int value) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform1i(uniform_locations[location], value);
	}

	void ComputeShader::setFloat(const int location, const float value) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform1f(uniform_locations[location], value);
	}

	void ComputeShader::setVec2(const int location, const glm::vec2 value) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform2fv(uniform_locations[location], 1, glm::value_ptr(value));
	}

	void ComputeShader::setVec3(const int location, const glm::vec3 value) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform3fv(uniform_locations[location], 1, glm::value_ptr(value));
	}

	void ComputeShader::setVec4(const int location, const glm::vec4 value) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform4fv(uniform_locations[location], 1, glm::value_ptr(value));
	}

	void ComputeShader::setMat3(const int location, const glm::mat3 value, const bool transpose) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniformMatrix3fv(uniform_locations[location], 1, transpose, glm::value_ptr(value));
	}

	void ComputeShader::setMat4(const int location, const glm::mat4 value, const bool transpose) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniformMatrix4fv(uniform_locations[location], 1, transpose, glm::value_ptr(value));
	}

	void ComputeShader::setIntArray(const int location, const int* value, const int count) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform1iv(uniform_locations[location], count, value);
	}

	void ComputeShader::setFloatArray(const int location, const float* value, const int count) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform1fv(uniform_locations[location], count, value);
	}

	void ComputeShader::setVec2Array(const int location, const glm::vec2* value, const int count) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform2fv(uniform_locations[location], count, glm::value_ptr(value[0]));
	}

	void ComputeShader::setVec3Array(const int location, const glm::vec3* value, const int count) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform3fv(uniform_locations[location], count, glm::value_ptr(value[0]));
	}

	void ComputeShader::setVec4Array(const int location, const glm::vec4* value, const int count) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniform4fv(uniform_locations[location], count, glm::value_ptr(value[0]));
	}

	void ComputeShader::setMat3Array(const int location, const glm::mat3* value, const int count, const bool transpose) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniformMatrix3fv(uniform_locations[location], count, transpose, glm::value_ptr(value[0]));
	}

	void ComputeShader::setMat4Array(const int location, const glm::mat4* value, const int count, const bool transpose) {
		glUseProgram(shader_program);
		if (location >= uniform_locations.size() || location < 0) {
			graphics_throw_error("An out of bounds exception");
			return;
		}
		glUniformMatrix4fv(uniform_locations[location], count, transpose, glm::value_ptr(value[0]));
	}

	ComputeShader::~ComputeShader() {
		dispose();
	}
}
