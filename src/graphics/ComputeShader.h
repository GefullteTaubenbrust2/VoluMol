#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "../logic/Types.h"

namespace fgr {
	///<summary>
	///A struct for creating and handling OpenGL compute shaders.
	///</summary>
	struct ComputeShader {
		///<summary>
		///The ID of the named program object. WARNING: read-only!
		///</summary>
		uint shader_program = 0, compute_shader = 0;

		std::string source;

		///<summary>
		///Is the Shader loaded? WARNING: read-only!
		///</summary>
		bool loaded = false;

		///<summary>
		///A vector containing the uniform locations within the shader, the index is equal to the index of the appropriate name given on construction. 
		///WARNING: read-only
		///</summary>
		std::vector<uint> uniform_locations;

		///<summary>
		///The names of all uniforms.
		///</summary>
		std::vector<std::string> uniform_names;

		///<summary>
		///The number of workgroups for all axes. 
		///WARNING can only be set before!
		///</summary>
		glm::uvec3 work_group_count;

		///<summary>
		///Settings for the shader code. Stored only for the = operator.
		///</summary>
		std::string settings;

		ComputeShader() = default;

		///<summary>
		///Copying and assignment not possible.
		///</summary>
		ComputeShader(const ComputeShader& copy);

		///<summary>
		///Load a shader with a vertex and fragment shader.
		///</summary>
		///<param name="path">The path of the shader source file.</param>
		///<param name="fragment_path">The path of the fragment shader source file.</param>
		///<param name="uniforms">Names of the shader uniforms.</param>
		ComputeShader(const std::string& path, const std::vector<std::string>& uniforms);

		///<summary>
		///Copying and assignment not possible.
		///</summary>
		void operator=(const ComputeShader& other);

		///<summary>
		///Compile the shader program.
		///</summary>
		///<param name="settings">Settings to add to the shader code.</param>
		///<returns>Success of the operation, false being successful.</returns>
		bool compile(const std::string& settings = "");

		///<summary>
		///Destroy the shader programs.
		///</summary>
		void dispose();

		///<summary>
		///Run the shader program.
		///</summary>
		void dispatch();

		///<summary>
		///Bind a texture as an image to the compute shader.
		///</summary>
		void bindImage(uint unit, uint texture_id, bool write, bool read, bool layered, uint format);

		///<summary>
		///Set a uniform int.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">The desired value to set the uniform to.</param>
		void setInt(const int location, const int value);

		///<summary>
		///Set a uniform float.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">The desired value to set the uniform to.</param>
		void setFloat(const int location, const float value);

		///<summary>
		///Set a uniform vec2.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">The desired value to set the uniform to.</param>
		void setVec2(const int location, const glm::vec2 value);

		///<summary>
		///Set a uniform vec3.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">The desired value to set the uniform to.</param>
		void setVec3(const int location, const glm::vec3 value);

		///<summary>
		///Set a uniform vec4.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">The desired value to set the uniform to.</param>
		void setVec4(const int location, const glm::vec4 value);

		///<summary>
		///Set a uniform 3x3 matrix.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">The desired value to set the uniform to.</param>
		///<param name="transpose">Transpose the matrix before setting the value?.</param>
		void setMat3(const int location, const glm::mat3 value, const bool transpose = false);

		///<summary>
		///Set a uniform 4x4 matrix.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">The desired value to set the uniform to.</param>
		///<param name="transpose">Transpose the matrix before setting the value?.</param>
		void setMat4(const int location, const glm::mat4 value, const bool transpose = false);

		///<summary>
		///Set a uniform array of ints.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">A pointer to the desired values.</param>
		///<param name="value">The amount of elements in the array. Should be equal for "value" and the in-shader uniform variable.</param>
		void setIntArray(const int location, const int* value, const int count);

		///<summary>
		///Set a uniform array of floats.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">A pointer to the desired values.</param>
		///<param name="value">The amount of elements in the array. Should be equal for "value" and the in-shader uniform variable.</param>
		void setFloatArray(const int location, const float* value, const int count);

		///<summary>
		///Set a uniform array of 2D vectors.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">A pointer to the desired values.</param>
		///<param name="value">The amount of elements in the array. Should be equal for "value" and the in-shader uniform variable.</param>
		void setVec2Array(const int location, const glm::vec2* value, const int count);

		///<summary>
		///Set a uniform array of 3D vectors.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">A pointer to the desired values.</param>
		///<param name="value">The amount of elements in the array. Should be equal for "value" and the in-shader uniform variable.</param>
		void setVec3Array(const int location, const glm::vec3* value, const int count);

		///<summary>
		///Set a uniform array of 4D vectors.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">A pointer to the desired values.</param>
		///<param name="value">The amount of elements in the array. Should be equal for "value" and the in-shader uniform variable.</param>
		void setVec4Array(const int location, const glm::vec4* value, const int count);

		///<summary>
		///Set a uniform array of 3x3 matrices.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">A pointer to the desired values.</param>
		///<param name="value">The amount of elements in the array. Should be equal for "value" and the in-shader uniform variable.</param>
		///<param name="transpose">Transpose the matrices before setting the values?.</param>
		void setMat3Array(const int location, const glm::mat3* value, const int count, const bool transpose = false);

		///<summary>
		///Set a uniform array of 4x4 matrices.
		///</summary>
		///<param name="location">The index of the location in the "uniform_locations" vector. Assume the uniform "foo" has been named as the first element for the vector, then location shall be 0 for setting the uniform.</param>
		///<param name="value">A pointer to the desired values.</param>
		///<param name="value">The amount of elements in the array. Should be equal for "value" and the in-shader uniform variable.</param>
		///<param name="transpose">Transpose the matrices before setting the values?.</param>
		void setMat4Array(const int location, const glm::mat4* value, const int count, const bool transpose = false);

		~ComputeShader();
	};
}
