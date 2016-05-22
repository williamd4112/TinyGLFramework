#include "GLAnimatedMesh.h"



tiny_gl::GLAnimatedMesh::GLAnimatedMesh()
{
}


tiny_gl::GLAnimatedMesh::~GLAnimatedMesh()
{
}

void tiny_gl::GLAnimatedMesh::load_file(std::string filepath)
{
	std::string err;
	if (!LoadFbx(mFbx, mShapes, mMaterials, err, filepath.c_str()))
		throw tiny_gl::tiny_gl_exception_t("FBX load error.");
}
