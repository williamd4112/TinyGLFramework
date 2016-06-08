#include "GLSkybox.h"

#include "tiny_gl_util.h"

static GLfloat skyboxVertices[] = {
	// Positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

void tiny_gl::GLSkybox::Load(std::vector<std::string> & filenames)
{
	assert(filenames.size() == 6);

	initShader();

	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
	for (GLuint i = 0; i < 6; i++)
	{
		ILuint ilTexName;
		ilGenImages(1, &ilTexName);
		ilBindImage(ilTexName);

		if (ilLoadImage(filenames[i].c_str()) && ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE))
		{
			int width = ilGetInteger(IL_IMAGE_WIDTH);
			int height = ilGetInteger(IL_IMAGE_HEIGHT);
			unsigned char *data = new unsigned char[ilGetInteger(IL_IMAGE_WIDTH) * ilGetInteger(IL_IMAGE_HEIGHT) * 3];
			ilCopyPixels(0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 1, IL_RGB, IL_UNSIGNED_BYTE, data);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

			delete[] data;
		}
		ilDeleteImages(1, &ilTexName);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindVertexArray(0);
}

void tiny_gl::GLSkybox::Render(glm::vec3 cameraPosition, glm::mat4 & viewMatrix, glm::mat4 & projMatrix)
{
	mProgram.Use();
	glDepthMask(GL_FALSE);
	
	glBindVertexArray(skyboxVAO);
	glUniform3fv(mProgram.GetLocation("camera_position"), 1, glm::value_ptr(cameraPosition));
	glUniformMatrix4fv(mProgram.GetLocation("view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(mProgram.GetLocation("projection"), 1, GL_FALSE, glm::value_ptr(projMatrix));
	
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(mProgram.GetLocation("skybox"), 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindVertexArray(0);
}

void tiny_gl::GLSkybox::initShader()
{
	mProgram = GLShaderProgram::LoadWithSeries(SCENE, "skybox");
	mLocViewMatrix = mProgram.GetLocation("view");
	mLocProjMatrix = mProgram.GetLocation("projection");
}
