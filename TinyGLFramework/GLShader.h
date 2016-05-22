#pragma once

#include "tiny_glframework.h"

namespace tiny_gl
{
	class GLShader
	{
		friend class GLShaderProgram;
	public:
		GLShader();
		~GLShader();

		void Load(std::string filepath, GLint type);
	private:
		GLuint mShaderId;
	};
}

