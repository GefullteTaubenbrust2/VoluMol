#pragma once
#include "../../logic/Types.h"
#include "../../logic/Pointer.h"
#include "../../graphics/Texture.h"

namespace fgr {
	struct RenderTarget;

	///<summary>
	///A struct for handling OpenGL 3D textures. One handle may be used for multiple textures throughout its lifetime.
	///</summary>
	struct TextureHandle3D {
		///<summary>
		///The dimensions of the texture. WARNING: read-only!
		///</summary>
		int width = 0, height = 0, depth = 0;

		///<summary>
		///The ID of the texture. WARNING: read-only!
		///</summary>
		uint id = 0;

		///<summary>
		///FBO id. WARNING: read-only!
		///</summary>
		uint fbo = 0;

		///<summary>
		///The data within the texture. May be changed, so lang as "synchTexture()" is subsequently called.
		///</summary>
		flo::Array<float> data;

		TextureHandle3D() = default;

		TextureHandle3D(uint width, uint height, uint depth, float* data = nullptr);

		///<summary>
		///No new OpenGL texture object is created when copying.
		///</summary>
		TextureHandle3D(const TextureHandle3D& copy);

		void operator=(const TextureHandle3D& other);
		
		///<summary>
		///Resize the texture.
		///</summary>
		///<param name="width">New width.</param>
		///<param name="height">New height.</param>
		///<param name="depth">New depth.</param>
		void resize(uint width, uint height, uint depth);

		///<summary>
		///Load a handle from an ID.
		///</summary>
		///<param name="id">The ID to be loaded from.</param>
		void loadFromID(const int id);

		///<summary>
		///Create a new OpenGL texture object from the present data.
		///</summary>
		///<param name="wrap">The OpenGL wrap mode to be applied.</param>
		///<param name="filter">The OpenGL filter mode to be applied.</param>
		void createBuffer(int wrap, int filter);

		///<summary>
		///Copy the present data to the current ID that is being handled.
		///</summary>
		void syncTexture();

		///<summary>
		///Create a FrameBuffer for rendering to the texture.
		///</summary>
		RenderTarget createFrameBuffer();

		///<summary>
		///Bind the texture for rendering.
		///</summary>
		///<param name="unit">The unit to be bound to.</param>
		void bindToUnit(const TextureUnit unit);

		///<summary>
		///Set the image data (in the OpenGL buffer) within a given cuboid.
		///</summary>
		///<param name="x">The x coordinate of the cuboid's corner.</param>
		///<param name="y">The y coordinate of the cuboid's corner.</param>
		///<param name="z">The z coordinate of the cuboid's corner.</param>
		///<param name="width">The width of the cuboid.</param>
		///<param name="height">The height of the cuboid.</param>
		///<param name="depth">The depth of the cuboid.</param>
		/// ///<param name="data">A pointer to read data from.</param>
		void setCuboid(int x, int y, int z, int width, int height, int depth, float* data);

		///<summary>
		///Destroy present data. The OpenGL texture itself will be destroyed too.
		///</summary>
		void dispose();

		~TextureHandle3D();
	};
}