#include "GLSkybox.h"

#include "tiny_gl_util.h"

GLfloat skyboxVertices[] = {
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

char *skybox_fs_glsl[] =
{
	"#version 410 core                          \n"
	"                                           \n"
	"uniform samplerCube tex_cubemap;           \n"
	"                                           \n"
	"in VS_OUT                                  \n"
	"{                                          \n"
	"    vec3    tc;                            \n"
	"} fs_in;                                   \n"
	"                                           \n"
	"layout (location = 0) out vec4 color;      \n"
	"                                           \n"
	"void main(void)                            \n"
	"{                                          \n"
	"    color = texture(tex_cubemap, fs_in.tc);\n"
	"}                                          \n"
	"                                           \n"
};

char *skybox_vs_glsl[] =
{
	"#version 410 core                                         \n"
	"layout (location = 0) in vec3 position;				   \n"
	"out VS_OUT                                                \n"
	"{                                                         \n"
	"    vec3    tc;                                           \n"
	"} vs_out;                                                 \n"
	"                                                          \n"
	"uniform mat4 view_matrix;                                 \n"
	"                                                          \n"
	"void main(void)                                           \n"
	"{                                                         \n"
	"    vec3[4] vertices = vec3[4](vec3(-1.0, -1.0, 1.0),     \n"
	"                               vec3( 1.0, -1.0, 1.0),     \n"
	"                               vec3(-1.0,  1.0, 1.0),     \n"
	"                               vec3( 1.0,  1.0, 1.0));    \n"
	"                                                          \n"
	"    vs_out.tc = mat3(view_matrix) * vertices[gl_VertexID];\n"
	"                                                          \n"
	"    gl_Position = vec4(vertices[gl_VertexID], 1.0);       \n"
	"}                                                         \n"
	"                                                          \n"
};

void tiny_gl::GLSkybox::Load(std::vector<std::string> & filenames)
{
	assert(filenames.size() == 6);

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	glGenTextures(1, &mTexture);

	glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
	for (int face = 0; face < 6; face++)
	{
		ILuint ilTexName;
		ilGenImages(1, &ilTexName);
		ilBindImage(ilTexName);
		
		if (ilLoadImage(filenames[face].c_str()) && ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE))
		{
			unsigned char *data = new unsigned char[ilGetInteger(IL_IMAGE_WIDTH) * ilGetInteger(IL_IMAGE_HEIGHT) * 4];
			ilCopyPixels(0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 1, IL_RGBA, IL_UNSIGNED_BYTE, data);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
				0,
				GL_RGBA, 
				ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 
				0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			delete[] data;

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
		ilDeleteImages(1, &ilTexName);
	}
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	initShader();
}

void tiny_gl::GLSkybox::Load(std::string merged_filename)
{
	texture_data envmap_data = load_png("mountaincube.png");
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, envmap_data.width, envmap_data.height / 6, 0, GL_RGBA, GL_UNSIGNED_BYTE, envmap_data.data + i * (envmap_data.width * (envmap_data.height / 6) * sizeof(unsigned char) * 4));
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	delete[] envmap_data.data;

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	initShader();
}

void tiny_gl::GLSkybox::Render(glm::mat4 & viewMatrix)
{
	mProgram.Use();
	glDepthMask(GL_FALSE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glUniformMatrix4fv(mLocViewMatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDepthMask(GL_TRUE);
}

void tiny_gl::GLSkybox::initShader()
{
	mProgram.Init(SCENE);
	GLShader vShader;
	vShader.Load(skybox_vs_glsl, GL_VERTEX_SHADER);
	GLShader fShader;
	fShader.Load(skybox_fs_glsl, GL_FRAGMENT_SHADER);
	mProgram.AttachShader(vShader);
	mProgram.AttachShader(fShader);
	mProgram.Link();
	mLocViewMatrix = mProgram.GetLocation("view_matrix");
}
