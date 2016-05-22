#pragma once

#include "tiny_glframework.h"

namespace tiny_gl
{
	void checkError(const char * functionName);
	void dumpInfo(void);
	char ** loadShaderSource(const char * file);
	void freeShaderSource(char ** srcp);
	void shaderLog(GLuint shader);
}
