#include "GLFrameBuffer.h"

#define MAX_PASS 4

static GLuint draw_buffers[MAX_PASS] = {
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3,
};

void tiny_gl::GLFrameBuffer::Init(BufferType type, int numChannel, int w, int h)
{
	mWidth = w;
	mHeight = h;
	mType = type;

	glGenFramebuffers(1, &mFrameBufferID);

	switch (mType)
	{
	case DEPTH:
		mTextureIDs.resize(numChannel);
		glGenTextures(numChannel, mTextureIDs.data());
		init_depth();
		break;
	case DEPTH_CUBE:
		mTextureIDs.resize(numChannel + 1);
		glGenTextures(numChannel + 1, mTextureIDs.data());
		init_depth_cube();
		break;
	case COLOR:
		mTextureIDs.resize(numChannel);
		glGenTextures(numChannel, mTextureIDs.data());
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
	case DEPTH_CUBE:
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mTextureIDs[1], 0);	
		break;
	case COLOR:
		glDrawBuffers((mTextureIDs.size() < MAX_PASS) ? mTextureIDs.size() : MAX_PASS, draw_buffers);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
		mWidth, mHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mTextureIDs.front(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void tiny_gl::GLFrameBuffer::init_depth_cube()
{
	mTextureIDs.resize(2);
	assert(mTextureIDs.size() == 2);

	glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureIDs[0]);
	for (GLuint i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			mWidth, mHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	glBindTexture(GL_TEXTURE_2D, mTextureIDs[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
		mWidth, mHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mTextureIDs[1], 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void tiny_gl::GLFrameBuffer::init_color()
{
	glGenTextures(mTextureIDs.size(), mTextureIDs.data()); //Create fobDataTexture
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFrameBufferID);

	//Set depthrbo to current fbo 
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRbo);
	for (int i = 0; i < mTextureIDs.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, mTextureIDs[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Set buffertexture to current fbo
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, draw_buffers[i], GL_TEXTURE_2D, mTextureIDs[i], 0);
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
