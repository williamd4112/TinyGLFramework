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
	char** source = tiny_gl::LoadShaderSource(filepath.c_str());

	glShaderSource(mShaderId, 1, source, NULL);

	tiny_gl::FreeShaderSource(source);

	glCompileShader(mShaderId);

	tiny_gl::ShaderLog(mShaderId);
}

void tiny_gl::GLShader::Load(char ** source, GLint type)
{
	mShaderId = glCreateShader(type);

	glShaderSource(mShaderId, 1, source, NULL);
	glCompileShader(mShaderId);

	tiny_gl::ShaderLog(mShaderId);
}
