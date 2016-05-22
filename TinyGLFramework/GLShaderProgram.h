#pragma once

#include "tiny_glframework.h"

namespace tiny_gl
{
	class GLScene;
	class GLShader;

	class GLShaderProgram
	{
	public:
		GLShaderProgram();
		~GLShaderProgram();

		void Init();
		void AttachShader(GLShader & shader);
		void Link();
		void Use();
		virtual void Render(const GLScene & scene);
		GLuint GetLocation(std::string name) { return glGetUniformLocation(mProgramId, name.c_str()); }
	protected:
		GLuint mProgramId;
	};

}
