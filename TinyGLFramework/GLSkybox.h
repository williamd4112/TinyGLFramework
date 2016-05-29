#pragma once

#include "tiny_glframework.h"
#include "GLShader.h"
#include "GLShaderProgram.h"

namespace tiny_gl
{
	class GLSkybox
	{
	public:
		GLSkybox() {}
		~GLSkybox() {}

		void Load(std::vector<std::string> & filenames);
		void Load(std::string merged_filename);
		void Render(glm::mat4 & viewMatrix);
		void initShader();
	private:
		GLuint mTexture;
		GLuint mLocViewMatrix;
		GLShaderProgram mProgram;
	};
}

