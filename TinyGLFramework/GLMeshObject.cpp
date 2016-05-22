#include "GLMeshObject.h"

#define MATERIAL_NUM_PROPS 4
#define POS_MAT_AMBIENT 0
#define POS_MAT_DIFFUSE 1
#define POS_MAT_SPECULAR 2
#define POS_MAT_SHININESS 3

#define VERTEX_ATTR_POS 0
#define VERTEX_ATTR_NORMAL 1
#define VERTEX_ATTR_TEXCOORD 2

tiny_gl::GLMeshObject::GLMeshObject()
{
}


tiny_gl::GLMeshObject::~GLMeshObject()
{
}

void tiny_gl::GLMeshObject::Load(std::string filepath)
{
	mMesh.Load(filepath);
}

void tiny_gl::GLMeshObject::Render(GLShaderProgram & program)
{
	glm::mat4 &m = mTransform.GetTransformMatrix();

	glUniformMatrix4fv(program.GetLocation("M"), 1, GL_FALSE, glm::value_ptr(m));

	glEnableVertexAttribArray(VERTEX_ATTR_POS);
	glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
	glEnableVertexAttribArray(VERTEX_ATTR_TEXCOORD);

	for (int j = 0; j < mMesh.mGroups.size(); j++)
	{
		tiny_gl::GLMesh::group_t group = mMesh.mGroups[j];

		glBindVertexArray(group.vaoid);
		for (int k = 0; k < group.indexGroups.size(); k++)
		{
			GLint matid = group.indexGroups[k].matid;
			GLuint ibo = group.indexGroups[k].ibo;
			tinyobj::material_t mat = mMesh.mMaterials[matid];

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mMesh.mTextureGroups[matid].diffuseid);

			glUniform4fv(program.GetLocation("Material.ambient"), 1, mat.ambient);
			glUniform4fv(program.GetLocation("Material.diffuse"), 1, mat.diffuse);
			glUniform4fv(program.GetLocation("Material.specular"), 1, mat.specular);
			glUniform1f(program.GetLocation("Material.shininess"), mat.shininess);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			glDrawElements(GL_TRIANGLES, group.indexGroups[k].numIndices, GL_UNSIGNED_INT, 0);
		}
	}
}
