#include "GLTangentMeshObject.h"

tiny_gl::GLTangentMeshObject::GLTangentMeshObject()
{
}

tiny_gl::GLTangentMeshObject::~GLTangentMeshObject()
{
}

void tiny_gl::GLTangentMeshObject::Load(std::string filepath)
{
	GLTangentMesh *tmesh = new GLTangentMesh;
	tmesh->Load(filepath);

	mMesh = tmesh;
}
