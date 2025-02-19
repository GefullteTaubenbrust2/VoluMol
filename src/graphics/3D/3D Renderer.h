#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Shader.h"

#include <vector>

#include "../Texture.h"

namespace fgr {
	///<summary>
	/// A struct for handling 3D vertices. Used for 3D meshes.
	///</summary>
	struct Vertex3D {
		glm::vec3 position = glm::vec3(0);
		glm::vec3 color = glm::vec3(0);
		glm::vec3 normal = glm::vec3(0);
		glm::vec3 tangent = glm::vec3(0);
		glm::vec2 tex_coord = glm::vec2(0);

		Vertex3D() = default;

		///<summary>
		/// Construct a 3D vertex.
		///</summary>
		Vertex3D(const glm::vec3& position, const glm::vec3& color, const glm::vec2& texCoord, const glm::vec3& normal = glm::vec3(0.), const glm::vec3& tangent = glm::vec3(0.));
	};

	///<summary>
	/// A struct for handling 3D views.
	///</summary>
	struct View {
		///<summary>
		/// The corresponding matrix associated with the view.
		///</summary>
		glm::mat4 view, projection;

		View() = default;

		///<summary>
		/// Set the projection matrix according to screen size.
		///</summary>
		///<param name="fov">The field of view in radiants.</param>
		///<param name="screen_width">The width of the framebuffer to render to.</param>
		///<param name="screen_height">The height of the framebuffer to render to.</param>
		///<param name="near">Near clipping plane depth.</param>
		///<param name="far">Far clipping plane depth.</param>
		void setPerspective(const float fov, const float screen_width, const float screen_height, const float near = 0.3f, const float far = 300.f);

		///<summary>
		/// Set the projection matrix to an orthographic projection matrix.
		///</summary>
		///<param name="width">Width of the view cuboid.</param>
		///<param name="height">Height of the view cuboid.</param>
		///<param name="near">Near clipping plane depth.</param>
		///<param name="far">Far clipping plane depth.</param>
		void setOrthographic(const float width, const float height, const float near, const float far);

		///<summary>
		/// Set the view matrix according to camera parameters.
		///</summary>
		///<param name="camera_pos">Where is the camera located?</param>
		///<param name="camera_front">Where is the camera facing?</param>
		///<param name="camera_up">Where is up for the camera?</param>
		void setOrientation(const glm::vec3& camera_pos, const glm::vec3& camera_front, const glm::vec3& camera_up);
	};

	///<summary>
	/// A struct for rendering 3D meshes.
	///</summary>
	struct Mesh {
	protected:
		uint VAO = 0, VBO = 0, EBO = 0;
		uint vertices_allocated = 0, indices_allocated = 0, model_uniform = 0;

		bool apply_matrices = true;

	public:
		///<summary>
		/// The model matrix. By default this has no effect.
		///</summary>
		glm::mat4 model_matrix = glm::mat4(1.0);

		///<summary>
		/// The vertex pool of the mesh.
		///</summary>
		std::vector<Vertex3D> vertices;

		///<summary>
		/// The indices of the vertices that will make up the mesh. Note that these must make individual triangles!
		///</summary>
		std::vector<uint> indices;

		Mesh() = default;

		Mesh(const Mesh& copy);

		void operator=(const Mesh& other);

		///<summary>
		/// Create all the OpenGL buffers and objects required for rendering.
		///</summary>
		void init();

		///<summary>
		/// Set all vertex data.
		///</summary>
		///<param name="vertices">The vertex pool of the mesh.</param>
		///<param name="indices">The indices of the vertices that will make up the mesh. Note that these must make individual triangles!</param>
		void setMesh(const std::vector<Vertex3D>& vertices, const std::vector<uint>& indices);

		///<summary>
		/// Merge geometry into this mesh.
		///</summary>
		///<param name="other">Mesh to take geometry from.</param>
		///<param name="transform">Transform the other mesh by a matrix before merging.</param>
		void mergeMesh(const Mesh& other, const glm::mat4& transform);

		///<summary>
		/// Load a mesh from a .obj file.
		///</summary>
		///<param name="path">The path of the file.</param>
		void loadFromOBJ(const std::string& path);

		///<summary>
		/// Automatically generate normal and tangent information.
		///</summary>
		///<param name="hard_shading">Generate extra vertices to enable hard shading.</param>
		void generateNormals(bool hard_shading = false);

		///<summary>
		/// Update the vertex data. This must only be called when this data is manually altered.
		///</summary>
		void update();

		///<summary>
		/// Draw the mesh to the framebuffer.
		///</summary>
		///<param name="front_culling">Enable frontface culling.</param>
		///<param name="back_culling">Enable backface culling.</param>
		void render(Shader& shader, bool back_culling = true, bool front_culling = false);

		///<summary>
		/// Destroy all allocated contents.
		///</summary>
		void dispose();

		~Mesh();
	};
}