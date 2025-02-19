#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../logic/SpriteSheet.h"
#include "../logic/Types.h"
#include "../logic/Pointer.h"

#include "VertexArray.h"

namespace fgr {
	///<summary>
	///Texture units that can be bound to. "dither_texture" is used for dithering and "misc" should not be used.
	///</summary>
	enum TextureUnit {
		texture0 = 0,
		texture1 = 1,
		texture2 = 2,
		texture3 = 3,
		texture4 = 7,
		texture5 = 8,
		texture6 = 9,
		texture7 = 10,
		texture8 = 11,
		texture9 = 12,
		texture10 = 13,
		texture11 = 14,
		texture12 = 15,
		texture13 = 16,
		texture14 = 17,
		texture15 = 18,
		texture16 = 19,
		texture17 = 20,
		texture18 = 21,
		texture19 = 22,
		texture20 = 23,
		texture21 = 24,
		texture22 = 25,
		texture23 = 26,
		dither_texture = 4,
		misc = 5,
		misc2 = 6,
	};

	///<summary>
	///A struct for handling OpenGL textures. One handle may be used for multiple textures throughout its lifetime.
	///</summary>
	struct TextureHandle {
		///<summary>
		///The dimensions of the texture. WARNING: read-only!
		///</summary>
		int width = 0, height = 0;
		
		///<summary>
		///The ID of the texture. WARNING: read-only!
		///</summary>
		uint id = 0;
		
		///<summary>
		///Specifies the pixel format (e.g. GL_RGBA16). 
		///</summary>
		int texture_format = GL_RGBA;
		
		///<summary>
		///The bytes required for each pixel in memory. WARNING: read-only!
		///</summary>
		int bytes_per_pixel = 4;

		///<summary>
		///The data within the texture. May be changed, so lang as "synchTexture()" is subsequently called.
		///</summary>
		flo::Array<u8> data;

		TextureHandle() = default;

		TextureHandle(int width, int height, int bytes_per_pixel = 4, u8* data = nullptr);

		///<summary>
		///No new OpenGL texture object is created when copying.
		///</summary>
		TextureHandle(const TextureHandle& copy);

		void operator=(const TextureHandle& other);

		///<summary>
		///Load image data from a file. This does not create an OpenGL texture.
		///</summary>
		///<param name="path">The path of the image file.</param>
		void loadFromFile(const std::string& path, bool flip_vertically = true);

		void saveFile(const std::string& path, bool flip_vertically = true);

		///<summary>
		///Load a handle from an ID.
		///</summary>
		///<param name="id">The ID to be loaded from.</param>
		///<param name="texture_format">Specifies the pixel format (e.g. GL_RGBA16).</param>
		void loadFromID(const int id, int texture_format = GL_RGBA);

		///<summary>
		///Create a new OpenGL texture object from the present data.
		///</summary>
		///<param name="wrap">The OpenGL wrap mode to be applied.</param>
		///<param name="filter">The OpenGL filter mode to be applied.</param>
		///<param name="texture_format">Specifies the pixel format (e.g. GL_RGBA16).</param>
		void createBuffer(int wrap, int filter, int texture_format = GL_RGBA);
		
		///<summary>
		///Copy the present data to the current ID that is being handled.
		///</summary>
		void syncTexture();

		///<summary>
		///Bind the texture for rendering.
		///</summary>
		///<param name="unit">The unit to be bound to.</param>
		void bindToUnit(const TextureUnit unit);

		///<summary>
		///Retrieve the image data (from the OpenGL buffer) within a given rectangle.
		///</summary>
		///<param name="x">The x coordinate of the rectangle's corner.</param>
		///<param name="y">The y coordinate of the rectangle's corner.</param>
		///<param name="width">The width of the rectangle.</param>
		///<param name="height">The height of the rectangle.</param>
		///<param name="data">A pointer to be assigned the raw image data. Its length is calculated as width*height*bytes_per_pixel.</param>
		///<param name="format">The format to be retrieved.</param>
		///<returns>The success, true being a failure and false a success.</returns>
		bool readRect(int x, int y, int width, int height, int format, u8* data);

		///<summary>
		///Set the image data (in the OpenGL buffer) within a given rectangle. The formats must be compatible.
		///</summary>
		///<param name="x">The x coordinate of the rectangle's corner.</param>
		///<param name="y">The y coordinate of the rectangle's corner.</param>
		///<param name="width">The width of the rectangle.</param>
		///<param name="height">The height of the rectangle.</param>
		///<param name="data">The data within the rectangle.</param>
		void setRect(int x, int y, int width, int height, u8* data);

		///<summary>
		///Destroy present data. The OpenGL texture itself will be destroyed too.
		///</summary>
		void dispose();

		///<summary>
		///- deprecated -
		///</summary>
		static int splitTexture(const TextureHandle& texture, int* output, const int width, const int height);

		~TextureHandle();
	};

	namespace Texture {
		///<summary>
		///Textures for dithering. WARNING: read-only!
		///</summary>
		extern TextureHandle dither0, dither1;

		///<summary>
		///Bind the texture for rendering.
		///</summary>
		///<param name="unit">The unit to be bound to.</param>
		void bindTextureID(uint id, TextureUnit unit);
	}

#define UNIT_ENUM_TO_GL_UNIT(UNIT) (int)UNIT + GL_TEXTURE0

	struct FrameBuffer;

	///<summary>
	///A struct for handling OpengGL 2D array textures. One handle may be used for multiple textures throughout its lifetime.
	///</summary>
	struct ArrayTexture {
		///<summary>
		///The ID of the texture. WARNING: read-only!
		///</summary>
		uint id = 0;

		uint fbo_id = 0, depth_rbo = 0;

		int fbo_layer = 0;
		
		///<summary>
		///The dimensions of the texture. WARNING: read-only!
		///</summary>
		int width = 0, height = 0, layer_count = 0;
		
		///<summary>
		///Specifies the pixel format (e.g. GL_RGBA16). 
		///</summary>
		int texture_format = GL_RGBA;
		
		///<summary>
		///The image data. WARNING: read-only!
		///</summary>
		flo::Array<u8> data;
		
		///<summary>
		///The amount of data per pixel. WARNING: read-only!
		///</summary>
		int bytes_per_pixel = 4;

		ArrayTexture() = default;

		ArrayTexture(int width, int height, int layer_count, int bytes_per_pixel = 4);

		///<summary>
		///No new OpenGL texture object is created when copying.
		///</summary>
		ArrayTexture(const ArrayTexture& copy);

		void operator=(const ArrayTexture& other);

		///<summary>
		///Load image data from a file. This does not create an OpgenGL texture. The width and height must be compatible whith the texture.
		///</summary>
		///<param name="path">The path of the image file.</param>
		///<param name="layer">The layer to load to.</param>
		void loadFromFile(const std::string& path, int layer, bool flip_vertically = true);

		///<summary>
		///Load a handle from an ID.
		///</summary>
		///<param name="id">The ID to be loaded from.</param>
		///<param name="texture_format">Specifies the pixel format (e.g. GL_RGBA16).</param>
		void loadFromID(const int id, int texture_format = GL_RGBA);

		///<summary>
		///Create a new OpenGL texture object from the present data.
		///</summary>
		///<param name="wrap">The OpenGL wrap mode to be applied.</param>
		///<param name="filter">The OpenGL filter mode to be applied.</param>
		///<param name="texture_format">Specifies the pixel format (e.g. GL_RGBA16).</param>
		void createBuffer(int wrap, int filter, int texture_format = GL_RGBA);

		///<summary>
		/// Create an FBO for a layer of the texture.
		///</summary>
		///<param name="layer">The targeted layer.</param>
		FrameBuffer createFBO(int layer);

		void destroyFBO();

		///<summary>
		///Copy the present data to the current ID that is being handled. 
		///WARNING: If there is something that uses the texture to draw into it, call loadFromID with the texture's own ID first, else rendered content will be deleted.
		///</summary>
		void syncTexture();

		///<summary>
		///Bind the texture for rendering.
		///</summary>
		///<param name="unit">The unit to be bound to.</param>
		void bindToUnit(TextureUnit unit);

		///<summary>
		///Retrieve the image data (from the handle's data itself) within a given rectangle.
		///</summary>
		///<param name="x">The x coordinate of the rectangle's corner.</param>
		///<param name="y">The y coordinate of the rectangle's corner.</param>
		///<param name="width">The width of the rectangle.</param>
		///<param name="height">The height of the rectangle.</param>
		///<param name="layer">The affected layer.</param>
		///<param name="data">A pointer to be assigned the raw image data. Its length is calculated as width*height*bytes_per_pixel.</param>
		///<returns>The success, true being a failure and false a success.</returns>
		bool readRect(int x, int y, int width, int height, int layer, u8* data);

		///<summary>
		///Set the image data (in the OpenGL buffer) within a given rectangle. The formats must be compatible.
		///</summary>
		///<param name="x">The x coordinate of the rectangle's corner.</param>
		///<param name="y">The y coordinate of the rectangle's corner.</param>
		///<param name="width">The width of the rectangle.</param>
		///<param name="height">The height of the rectangle.</param>
		///<param name="layer">The affected layer.</param>
		///<param name="data">The data within the rectangle.</param>
		void setRect(int x, int y, int width, int height, int layer, u8* data);

		///<summary>
		///Destroy present data. The OpenGL texture itself will be destroyed too.
		///</summary>
		void dispose();

		~ArrayTexture();
	};

	///<summary>
	/// A struct used for adressing textures contained within a TextureStorage. Do not attempt to manually construct an instance of this.
	///</summary>
	struct SubTexture {
		u32 hash = INVALID_INDEX;
		u32 layer = 0;

		SubTexture();
	};

	///<summary>
	/// A struct for conviniently storing multiple textures of the same format. Note that the size of these textures is limited.
	///</summary>
	struct TextureStorage : public ArrayTexture {
	protected:
		std::vector<flo::SpriteSheet> allocators;

	public:
		/// <summary>
		/// Do you wish for there to be free space between textures? This can prevent bleeding artefacts.
		/// </summary>
		bool leave_free_edges = false;

		TextureStorage() = default;

		///<summary>
		/// Construct a TextureStorage.
		///</summary>
		///<param name="width">The maximum width a texture can have.</param>
		///<param name="height">The maximum height a texture can have.</param>
		///<param name="bytes_per_pixel">How many bytes of data one pixel contains. This should probably be left at 4.</param>
		TextureStorage(int width, int height, int bytes_per_pixel = 4);

		///<summary>
		/// Creates a new layer. Do not call this!
		///</summary>
		void addLayer();

		///<summary>
		/// Add a texture to the storage.
		///</summary>
		///<param name="width">The width of the added texture.</param>
		///<param name="height">The height of the added texture.</param>
		///<param name="data">The data contained within the added texture.</param>
		///<returns>The subtexture that is created.</returns>
		SubTexture addTexture(int width, int height, u8* data);

		///<summary>
		/// Load an image from a file and add it.
		///</summary>
		///<param name="path">The path of the file.</param>
		///<param name="flip_vertically">Flip the loaded contents vertically?.</param>
		///<returns>The subtexture that is created. Its hash is invalid if unsuccessfull.</returns>
		SubTexture addImage(const std::string& path, bool flip_vertically = true);

		///<summary>
		/// Get the bounds of a subtexture. When the components are negative, the subtexture is invalid.
		///</summary>
		///<param name="texture_unit">The subtexture.</param>
		///<returns>The bounds in pixel space.</returns>
		glm::ivec4 getTextureBounds(SubTexture texture_unit) const;

		//<summary>
		/// Get the normalized bounds of a subtexture. When the components are negative, the subtexture is invalid.
		///</summary>
		///<param name="texture_unit">The subtexture.</param>
		///<param name="vertically_flipped">Are the contents of the bounds flipped vertically?</param>
		///<returns>The bounds in pixel space.</returns>
		glm::vec4 getTextureBoundsNormalized(SubTexture texture_unit, bool vertically_flipped) const;

		///<summary>
		/// Get the normalized texture coordinated of a set of bounds.
		///</summary>
		///<param name="bounds">The bounds in question.</param>
		///<param name="vertically_flipped">Are the contents of the bounds flipped vertically?</param>
		///<returns>The normalized texture coordinates.</returns>
		glm::vec4 getTextureCoordinates(const glm::ivec4& bounds, bool vertically_flipped) const;

		///<summary>
		/// Free a subtexture from the storage, invalidating it.
		///</summary>
		///<param name="texture_unit">The unit in question.</param>
		void free(SubTexture& texture_unit);

		///<summary>
		/// Create a framebuffer from a subtexture and bind it. [-1.;-1.] and [1.;1.] are the bottom left and top right corners of the subtexture respectively.
		/// Rendering outside of this range will still be possible, so take heed.
		///</summary>
		///<param name="texture">The unit in question.</param>
		void bindFrameBuffer(SubTexture texture);

		///<summary>
		/// Clear a subtexture.
		///</summary>
		///<param name="texture">The unit in question.</param>
		///<param name="color">The clear color.</param>
		void clearFramebuffer(SubTexture texture, const glm::vec4& color = glm::vec4(0.0, 0.0, 0.0, 0.0));
	};
}
