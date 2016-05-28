#pragma once

#include "tiny_glframework.h"

namespace tiny_gl
{
	class GLFrameBufferObject
	{
	public:
		GLFrameBufferObject() : mWidth(0), mHeight(0), mNumPass(1), mVao(0), mVbo(0), mFbo(0), mDepthRbo(0) {}
		GLFrameBufferObject(int w, int h) : mWidth(w), mHeight(h), mNumPass(1), mVao(0), mVbo(0), mFbo(0), mDepthRbo(0) {}
		~GLFrameBufferObject() {}

		void Init(int _numPass, int w, int h);
		void Reshape(int w, int h);
		void BindFBO();
		void BindVAO() const { glBindVertexArray(mVao); }

		GLuint GetFrameBufferID() { return mFbo; }
		GLuint GetVAO() const { return mVao; }
		const std::vector<GLuint> & GetTextures() const { return mTextures; }
	private:
		int mWidth;
		int mHeight;
		int mNumPass;

		GLuint mVao;
		GLuint mVbo;
		GLuint mFbo;
		GLuint mDepthRbo;
		std::vector<GLuint> mTextures;

		void genPass();
	};
}

