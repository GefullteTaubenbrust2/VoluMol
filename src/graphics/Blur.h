#pragma once
#include "FrameBuffer.h"

namespace fgr {
	///<summary>
	/// A struct for blurring images.
	///</summary>
	struct BlurBuffer {
	protected:
		FrameBuffer io_fbo, util_fbo;
	public:
		///<summary>
		/// The radius of the blur in pixels.
		///</summary>
		float blur_radius = 10.;

		///<summary>
		/// When set to true, the quality will be improved. This will slow down the blurring process.
		///</summary>
		bool increased_blur = false;
		
		BlurBuffer() = default;

		///<summary>
		/// Construct a BlurBuffer with a given radius. The default constructor too will suffice.
		///</summary>
		///<param name="blur_radius">The radius of the blur in pixels.</param>
		BlurBuffer(float blur_radius);

		BlurBuffer(const BlurBuffer& copy);

		void operator=(const BlurBuffer& other);

		///<summary>
		/// Must be called before blurring.
		///</summary>
		void init();

		///<summary>
		/// Blur the contents of the fbo. Note that its contents will remain unchanged and the results are instead stored in the blur buffer object.
		///</summary>
		///<param name="fbo">A reference to the fbo to bind.</param>
		void blur(const FrameBuffer& fbo);

		///<summary>
		/// Blur the contents of a texture. Note that its contents will remain unchanged and the results are instead stored in the blur buffer object.
		///</summary>
		///<param name="texture_id">The texture's ID.</param>
		///<param name="tex_size">The texture's size.</param>
		void blur(uint texture_id, const glm::ivec2 tex_size);

		///<summary>
		/// Bind the blurred texture.
		///</summary>
		///<param name="unit">The texture unit to bind to.</param>
		void bindContent(fgr::TextureUnit unit);

		///<summary>
		/// Destroy all allocated contents.
		///</summary>
		void dispose();
	};
}