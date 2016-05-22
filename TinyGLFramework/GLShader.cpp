#include "GLShader.h"

#include "tiny_gl_util.h"

tiny_gl::GLShader::GLShader()
{
}


tiny_gl::GLShader::~GLShader()
{
}

void tiny_gl::GLShader::Load(std::string filepath, GLint type)
{
	mShaderId = glCreateShader(type);
	char** source = tiny_gl::loadShaderSource(filepath.c_str());

	glShaderSource(mShaderId, 1, source, NULL);

	tiny_gl::freeShaderSource(source);

	glCompileShader(mShaderId);

	tiny_gl::shaderLog(mShaderId);
}