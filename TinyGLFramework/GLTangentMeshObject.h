#pragma once

#include "tiny_glframework.h"
#include "GLMeshObject.h"
#include "GLTangentMesh.h"

namespace tiny_gl
{
	class GLTangentMeshObject
		: public GLMeshObject
	{
	public:
		GLTangentMeshObject();
		~GLTangentMeshObject();

		void Load(std::string filepath);
	};
}

