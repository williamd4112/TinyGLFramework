#pragma once

#include "tiny_glframework.h"

namespace tiny_gl
{
	class GLFrameBuffer
	{
	public:
		enum BufferType
		{
			COLOR,
			DEPTH,
			DEPTH_CUBE
		};

		GLFrameBuffer() {}
		~GLFrameBuffer() {}

		void Init(BufferType type, int numChannel, int w, int h);
		void Bind();
		void Unbind();
		GLuint GetTexture(int i) { return mTextureIDs.at(i); }
	private:
		BufferType mType;
		int mWidth;
		int mHeight;
		GLuint mFrameBufferID;
		GLuint mDepthRbo; // For color buffer
		std::vector<GLuint> mTextureIDs;

		void init_depth();
		void init_depth_cube();
		void init_color();
	};
}