#pragma once

#include "tiny_glframework.h"
#include "GLFrameBufferObject.h"

namespace tiny_gl
{
	class GLScene;
	class GLShader;
	class GLShaderProgram;

	typedef void(*GLRenderFunc)(const GLShaderProgram &, const GLScene &);
	typedef void(*GLFBORenderFunc)(const GLShaderProgram &, const GLFrameBufferObject &);

	enum ShaderProgramType
	{
		SCENE, FBO
	};

	class GLShaderProgram
	{
	public:
		static GLShaderProgram LoadWithSeries(ShaderProgramType type, std::string series);
	public:
		GLShaderProgram();
		~GLShaderProgram();

		void Init();
		void Init(ShaderProgramType type);
		void SetRenderFunction(GLRenderFunc func);
		void SetFrameBufferRenderFunction(GLFBORenderFunc func);
		void AttachShader(GLShader & shader);
		void Link() const;
		void Use() const;
		

		/*
			Render Scene
		*/
		virtual void Render(const GLScene & scene);
		
		/*
			Render FBO
		*/
		virtual void Render(const GLFrameBufferObject & fbo);
		ShaderProgramType GetType() { return mType; }
		GLuint GetLocation(std::string name) const { return glGetUniformLocation(mProgramId, name.c_str()); }
	protected:
		ShaderProgramType mType;
		GLuint mProgramId;
		GLRenderFunc mFunc;
		GLFBORenderFunc mFboFunc;
	};
}
