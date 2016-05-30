#pragma once

#include "tiny_glframework.h"
#include "GLShaderProgram.h"
#include "GLLight.h"

namespace tiny_gl
{
	typedef struct _texture_data
	{
		_texture_data() : width(0), height(0), data(0) {}
		int width;
		int height;
		unsigned char* data;
	} texture_data;

	void CheckError(const char * functionName);
	void DumpInfo(void);
	char ** LoadShaderSource(const char * file);
	void FreeShaderSource(char ** srcp);
	void ShaderLog(GLuint shader);

	void SetLightSource(const GLShaderProgram & program, std::string prefix, int lightID, const GLLight & light);
	texture_data load_png(const char* path);

	glm::vec3 colorRGB(float r, float g, float b);
}
