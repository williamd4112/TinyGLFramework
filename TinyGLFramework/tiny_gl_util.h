#pragma once

#include "tiny_glframework.h"
#include "GLShaderProgram.h"
#include "GLLight.h"

namespace tiny_gl
{
	void CheckError(const char * functionName);
	void DumpInfo(void);
	char ** LoadShaderSource(const char * file);
	void FreeShaderSource(char ** srcp);
	void ShaderLog(GLuint shader);

	void SetLightSource(const GLShaderProgram & program, std::string prefix, int lightID, const GLLight & light);
}
