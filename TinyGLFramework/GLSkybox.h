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
		void Render(glm::vec3 cameraPosition, glm::mat4 & viewMatrix, glm::mat4 & projMatrix);
		void BindVAO() { glBindVertexArray(skyboxVAO); }
		GLuint GetTextureID() { return mTexture; }
		void initShader();
	private:
		GLuint skyboxVAO, skyboxVBO;
		GLuint mTexture;
		GLuint mLocViewMatrix;
		GLuint mLocProjMatrix;
		GLShaderProgram mProgram;
	};
}

