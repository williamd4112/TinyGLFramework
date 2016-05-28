#include "GLFrameBufferObject.h"

#define MAX_PASS 4

static const GLfloat window_vertex[] = {
	//vec2 position vec2 texture_coord 
	1.0f,-1.0f,1.0f,0.0f,
	-1.0f,-1.0f,0.0f,0.0f,
	-1.0f,1.0f,0.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f
};

static GLuint draw_buffers[MAX_PASS] = {
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3,
};

void tiny_gl::GLFrameBufferObject::Init(int _numPass, int w, int h)
{
	mNumPass = _numPass;
	mWidth = w;
	mHeight = h;

	mTextures.resize(mNumPass);

	glGenVertexArrays(1, &mVao);
	glBindVertexArray(mVao);

	glGenBuffers(1, &mVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(window_vertex), window_vertex, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, (const GLvoid*)(sizeof(GL_FLOAT) * 2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	glGenFramebuffers(1, &mFbo); //Create FBO
	glGenRenderbuffers(1, &mDepthRbo); //Create Depth RBO 
	glBindRenderbuffer(GL_RENDERBUFFER, mDepthRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, w, h);

	genPass();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void tiny_gl::GLFrameBufferObject::Reshape(int w, int h)
{
	mWidth = w;
	mHeight = h;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFbo);
	glDeleteTextures(mNumPass, mTextures.data());

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, mWidth, mHeight);

	genPass();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void tiny_gl::GLFrameBufferObject::BindFBO()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFbo);
	glDrawBuffers(mNumPass, draw_buffers);
	glViewport(0, 0, mWidth, mHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void tiny_gl::GLFrameBufferObject::genPass()
{
	glGenTextures(mNumPass, mTextures.data()); //Create fobDataTexture
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFbo);
	//Set depthrbo to current fbo 
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRbo);
	for (int i = 0; i < mNumPass; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, mTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Set buffertexture to current fbo
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, draw_buffers[i], GL_TEXTURE_2D, mTextures[i], 0);
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
