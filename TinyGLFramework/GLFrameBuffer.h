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
			DEPTH
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
		std::vector<GLuint> mTextureIDs;

		void init_depth();
		void init_color();
	};
}