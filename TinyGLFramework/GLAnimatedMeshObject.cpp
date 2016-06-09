#include "GLAnimatedMeshObject.h"



tiny_gl::GLAnimatedMeshObject::GLAnimatedMeshObject(): GLObject(TYPE_ANIMMESHOBJ)
{
}


tiny_gl::GLAnimatedMeshObject::~GLAnimatedMeshObject()
{
}

void tiny_gl::GLAnimatedMeshObject::Update(float t)
{
	std::vector<tinyobj::shape_t> new_shapes;
	GetFbxAnimation(mAnimMesh.Fbx(), new_shapes, t);
	for (int j = 0; j < new_shapes.size(); j++)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mAnimMesh.Groups()[j].vboid[VBO_POS]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, new_shapes[j].mesh.positions.size() * sizeof(float), &new_shapes[j].mesh.positions[0]);
	}
}

void tiny_gl::GLAnimatedMeshObject::Load(std::string filepath)
{
	mAnimMesh.Load(filepath);
}

void tiny_gl::GLAnimatedMeshObject::Render(GLShaderProgram & program)
{
}
