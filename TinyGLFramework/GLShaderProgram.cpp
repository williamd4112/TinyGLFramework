#include "GLShaderProgram.h"

#include "GLShader.h"
#include "GLScene.h"
#include "GLMeshObject.h"
#include "GLFrameBufferObject.h"

#include <cassert>

using namespace glm;

tiny_gl::GLShaderProgram::GLShaderProgram()
{
}


tiny_gl::GLShaderProgram::~GLShaderProgram()
{
}

void tiny_gl::GLShaderProgram::Init()
{
	Init(SCENE);
}

void tiny_gl::GLShaderProgram::Init(ShaderProgramType type)
{
	mType = type;
	mProgramId = glCreateProgram();
}

void tiny_gl::GLShaderProgram::SetRenderFunction(GLRenderFunc func)
{
	if (mType != SCENE)
		throw tiny_gl::tiny_gl_exception_t("Wrong shader type assignment.");
	mFunc = func;
}

void tiny_gl::GLShaderProgram::SetFrameBufferRenderFunction(GLFBORenderFunc func)
{
	if (mType != FBO)
		throw tiny_gl::tiny_gl_exception_t("Wrong shader type assignment.");
	mFboFunc = func;
}

void tiny_gl::GLShaderProgram::AttachShader(GLShader & shader)
{
	glAttachShader(mProgramId, shader.mShaderId);
}

void tiny_gl::GLShaderProgram::Link() const
{
	glLinkProgram(mProgramId);
}

void tiny_gl::GLShaderProgram::Use() const
{
	glUseProgram(mProgramId);
}

void tiny_gl::GLShaderProgram::Render(const GLScene & scene)
{
	if (mFunc == NULL)
		throw tiny_gl::tiny_gl_exception_t("Scene Rendering function not set.");
	Use();
	mFunc(*this, scene);
}

void tiny_gl::GLShaderProgram::Render(const GLFrameBufferObject & fbo)
{
	if (mFboFunc == NULL)
		throw tiny_gl::tiny_gl_exception_t("FBO Rendering function not set.");
	Use();
	mFboFunc(*this, fbo);
}

