#include "GLShaderProgram.h"

#include "GLShader.h"
#include "GLScene.h"
#include "GLMeshObject.h"
#include "GLFrameBufferObject.h"

using namespace glm;

tiny_gl::GLShaderProgram tiny_gl::GLShaderProgram::LoadWithSeries(ShaderProgramType type, std::string series)
{
	std::string vName = series + ".vs.glsl";
	std::string fName = series + ".fs.glsl";

	GLShaderProgram prog;
	prog.Init(type);
	GLShader vShader;
	vShader.Load(vName, GL_VERTEX_SHADER);
	GLShader fShader;
	fShader.Load(fName, GL_FRAGMENT_SHADER);
	prog.AttachShader(vShader);
	prog.AttachShader(fShader);
	prog.Link();

	return prog;
}

tiny_gl::GLShaderProgram tiny_gl::GLShaderProgram::LoadWithSeries(ShaderProgramType type, std::string series, unsigned int mode)
{
	std::string vName = series + ".vs.glsl";
	std::string fName = series + ".fs.glsl";
	std::string gName = series + ".gs.glsl";

	GLShaderProgram prog;
	prog.Init(type);

	if (mode & BIT_VS)
	{
		GLShader vShader;
		vShader.Load(vName, GL_VERTEX_SHADER);
		prog.AttachShader(vShader);
	}

	if (mode & BIT_FS)
	{
		GLShader fShader;
		fShader.Load(fName, GL_FRAGMENT_SHADER);
		prog.AttachShader(fShader);
	}

	if (mode & BIT_GS)
	{
		GLShader gShader;
		gShader.Load(gName, GL_GEOMETRY_SHADER);
		prog.AttachShader(gShader);
	}

	prog.Link();

	return prog;
}

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



