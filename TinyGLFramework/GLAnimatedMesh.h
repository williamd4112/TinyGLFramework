#pragma once

#include "tiny_glframework.h"
#include "GLMesh.h"
#include "fbxloader.h"

namespace tiny_gl
{
	class GLAnimatedMesh
		: public GLMesh
	{
		friend class GLAnimatedMeshObject;
	public:
		GLAnimatedMesh();
		~GLAnimatedMesh();
		
		fbx_handles & Fbx() { return mFbx; }
	protected:
		fbx_handles mFbx;

		void load_file(std::string filepath);
	};

}
