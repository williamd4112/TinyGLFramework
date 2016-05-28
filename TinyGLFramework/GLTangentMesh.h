#pragma once

#include "tiny_glframework.h"
#include "GLMesh.h"

namespace tiny_gl
{
	class GLTangentMesh :
		public GLMesh
	{
		friend class GLTangentMeshObject;
	public:
		GLTangentMesh();
		~GLTangentMesh();
	protected:
		void Load(std::string filepath);
		void load_texture();
	};
}

