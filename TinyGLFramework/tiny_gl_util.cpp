#include "tiny_gl_util.h"

void tiny_gl::CheckError(const char *functionName)
{
	GLenum error;

	while ((error = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "GL error 0x%X detected in %s\n", error, functionName);
	}
}

void tiny_gl::DumpInfo(void)
{
	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

char** tiny_gl::LoadShaderSource(const char* file)
{
	FILE* fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	char **srcp = new char*[1];
	srcp[0] = src;
	return srcp;
}

void tiny_gl::FreeShaderSource(char** srcp)
{
	delete srcp[0];
	delete srcp;
}

void tiny_gl::ShaderLog(GLuint shader)
{
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		char* errorLog = new char[maxLength];
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		printf("%s\n", errorLog);
		delete errorLog;
	}
}

void tiny_gl::SetLightSource(const GLShaderProgram & program, std::string prefix, int lightID, const GLLight & light)
{
	static const char *lightsource_loc_str[] =
	{
		"type",
		"ambient",
		"diffuse",
		"specular",
		"position",
		"halfVector",
		"spotDirection",
		"spotExponent",
		"spotCutoff", // (range: [0.0,90.0], 180.0)
		"spotCosCutoff", // (range: [1.0,0.0],-1.0)
		"constantAttenuation",
		"linearAttenuation",
		"quadraticAttenuation"
	};

	GLuint iLocLightsource[LIGHTSOURCE_NUM_PROPS];
	const GLLight::LightSourceParameters & lightsource = light.GetParms();

	char str[256];
	for (int i = 0; i < LIGHTSOURCE_NUM_PROPS; i++)
	{
		sprintf(str, "%s[%d].%s", prefix.c_str(), lightID, lightsource_loc_str[i]);
		iLocLightsource[i] = program.GetLocation(str);
	}
	glUniform1i(iLocLightsource[POS_TYPE], lightsource.type);
	glUniform4fv(iLocLightsource[POS_AMBIENT], 1, lightsource.ambient);
	glUniform4fv(iLocLightsource[POS_DIFFUSE], 1, lightsource.diffuse);
	glUniform4fv(iLocLightsource[POS_SPECULAR], 1, lightsource.specular);
	glUniform4fv(iLocLightsource[POS_POSITION], 1, lightsource.position);
	glUniform4fv(iLocLightsource[POS_HALFVECTOR], 1, lightsource.halfVector);
	glUniform4fv(iLocLightsource[POS_SPOT_DIR], 1, lightsource.spotDirection);
	glUniform1fv(iLocLightsource[POS_SPOT_EXP], 1, &lightsource.spotExponent);
	glUniform1fv(iLocLightsource[POS_SPOT_CUTOFF], 1, &lightsource.spotCutoff);
	glUniform1fv(iLocLightsource[POS_SPOT_COSTCUTOFF], 1, &lightsource.spotCosCutoff);
	glUniform1fv(iLocLightsource[POS_CONSTANT_ATTENUATION], 1, &lightsource.constantAttenuation);
	glUniform1fv(iLocLightsource[POS_CONSTANT_LINEAR_ATTENUATION], 1, &lightsource.linearAttenuation);
	glUniform1fv(iLocLightsource[POS_CONSTANT_QUADRATIC_ATTENUATION], 1, &lightsource.quadraticAttenuation);
}
