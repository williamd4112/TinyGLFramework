#include "GLFrameBuffer.h"

void tiny_gl::GLFrameBuffer::Init(BufferType type, int numChannel, int w, int h)
{
	mWidth = w;
	mHeight = h;
	mType = type;

	glGenFramebuffers(1, &mFrameBufferID);
	
	mTextureIDs.resize(numChannel);
	glGenTextures(numChannel, mTextureIDs.data());
	switch (mType)
	{
	case DEPTH:
		init_depth();
		break;
	case COLOR:
		init_color();
		break;
	default:
		break;
	}
}

void tiny_gl::GLFrameBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferID);
	
	switch (mType)
	{
	case DEPTH:
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mTextureIDs.front(), 0);
		break;
	case COLOR:
		break;
	default:
		break;
	}
}

void tiny_gl::GLFrameBuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void tiny_gl::GLFrameBuffer::init_depth()
{
	assert(mTextureIDs.size() == 1);

	glBindTexture(GL_TEXTURE_2D, mTextureIDs.front());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		mWidth, mHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mTextureIDs.front(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void tiny_gl::GLFrameBuffer::init_color()
{
}
