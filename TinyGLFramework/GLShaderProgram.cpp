#include "GLShaderProgram.h"

#include "GLShader.h"
#include "GLScene.h"
#include "GLMeshObject.h"

#define MATERIAL_NUM_PROPS 4
#define POS_MAT_AMBIENT 0
#define POS_MAT_DIFFUSE 1
#define POS_MAT_SPECULAR 2
#define POS_MAT_SHININESS 3

using namespace glm;

static const char *material_loc_str[] =
{
	"ambient",
	"diffuse",
	"specular",
	"shininess"
};

tiny_gl::GLShaderProgram::GLShaderProgram()
{
}


tiny_gl::GLShaderProgram::~GLShaderProgram()
{
}

void tiny_gl::GLShaderProgram::Init()
{
	mProgramId = glCreateProgram();
}

void tiny_gl::GLShaderProgram::AttachShader(GLShader & shader)
{
	glAttachShader(mProgramId, shader.mShaderId);
}

void tiny_gl::GLShaderProgram::Link()
{
	glLinkProgram(mProgramId);
}

void tiny_gl::GLShaderProgram::Use()
{
	glUseProgram(mProgramId);
}

void tiny_gl::GLShaderProgram::Render(const GLScene & scene)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(mProgramId);

	GLuint uLocMVP = GetLocation("mvp");
	GLuint uLocM = GetLocation("M");
	GLuint uLocV = GetLocation("V");
	GLuint uLocP = GetLocation("P");
	GLuint uLocN = GetLocation("N");
	GLuint uLocE = GetLocation("E");
	GLuint iLocMaterial[MATERIAL_NUM_PROPS];
	
	char str[256];
	for (int i = 0; i < MATERIAL_NUM_PROPS; i++)
	{
		sprintf(str, "Material.%s", material_loc_str[i]);
		iLocMaterial[i] =
			glGetUniformLocation(mProgramId, str);
	}

	GLCamera & camera = *(scene.mCamera);
	for (int i = 0; i < scene.mObjects.size(); i++)
	{
		GLMeshObject * model = static_cast<GLMeshObject *>((scene.mObjects[i]));

		glm::mat4 &m = model->GetTransform().GetTransformMatrix();
		glm::mat4 v = camera.GetViewMatrix();
		glm::mat4 p = camera.GetProjectionMatrix();
		glm::mat4 &vp = camera.GetViewProjectionMatrix();
		glm::mat4 n = transpose(inverse(v * m));

		glm::mat4 mvp = vp * m * mat4(1.0);
		glUniformMatrix4fv(uLocMVP, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(uLocM, 1, GL_FALSE, glm::value_ptr(m));
		glUniformMatrix4fv(uLocV, 1, GL_FALSE, glm::value_ptr(v));
		glUniformMatrix4fv(uLocP, 1, GL_FALSE, glm::value_ptr(p));
		glUniformMatrix4fv(uLocN, 1, GL_FALSE, glm::value_ptr(n));
		glUniform3fv(uLocE, 1, glm::value_ptr(camera.GetTransform().Position()));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		const GLMesh & mesh = model->Mesh();
		const std::vector<tiny_gl::GLMesh::group_t> & groups = mesh.Groups();
		for (int j = 0; j < groups.size(); j++)
		{
			const tiny_gl::GLMesh::group_t & group = groups[j];

			glBindVertexArray(group.vaoid);
			for (int k = 0; k < group.indexGroups.size(); k++)
			{
				GLint matid = group.indexGroups[k].matid;
				GLuint ibo = group.indexGroups[k].ibo;
				tinyobj::material_t mat = mesh.Materials()[matid];

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, mesh.Textures()[matid].diffuseid);

				glUniform4fv(iLocMaterial[POS_MAT_AMBIENT], 1, mat.ambient);
				glUniform4fv(iLocMaterial[POS_MAT_DIFFUSE], 1, mat.diffuse);
				glUniform4fv(iLocMaterial[POS_MAT_SPECULAR], 1, mat.specular);
				glUniform1f(iLocMaterial[POS_MAT_SHININESS], mat.shininess);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
				glDrawElements(GL_TRIANGLES, group.indexGroups[k].numIndices, GL_UNSIGNED_INT, 0);
			}
		}
	}
}

