#include "GLMeshObject.h"

#define MATERIAL_NUM_PROPS 4
#define POS_MAT_AMBIENT 0
#define POS_MAT_DIFFUSE 1
#define POS_MAT_SPECULAR 2
#define POS_MAT_SHININESS 3

#define VERTEX_ATTR_POS 0
#define VERTEX_ATTR_NORMAL 1
#define VERTEX_ATTR_TEXCOORD 2

tiny_gl::GLMeshObject::GLMeshObject(): 
	GLObject(TYPE_MESHOBJ)
{
}

tiny_gl::GLMeshObject::~GLMeshObject()
{
	delete mMesh;
}

void tiny_gl::GLMeshObject::Load(std::string filepath)
{
	mMesh = new GLMesh;
	mMesh->Load(filepath);
}

void tiny_gl::GLMeshObject::SetIndirect()
{

}

void tiny_gl::GLMeshObject::Render(GLShaderProgram & program)
{

}

bool tiny_gl::GLMeshObject::HasIndirect()
{
	return false;
}
