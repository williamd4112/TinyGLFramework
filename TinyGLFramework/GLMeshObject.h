#pragma once

#include "tiny_glframework.h"
#include "GLObject.h"
#include "GLMesh.h"

namespace tiny_gl
{
	class GLMeshObject
		: public GLObject
	{
	public:
		GLMeshObject();
		~GLMeshObject();

		void Load(std::string filepath);
		void Render(GLShaderProgram & program);
		const GLMesh & Mesh() { return mMesh; }
	private:
		GLMesh mMesh;		
	};
}

