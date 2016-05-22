#pragma once

#include "tiny_glframework.h"
#include "fbxloader.h"
#include "GLMeshObject.h"
#include "GLAnimatedMesh.h"

namespace tiny_gl
{
	class GLAnimatedMeshObject
		: public GLObject
	{
	public:
		GLAnimatedMeshObject();
		~GLAnimatedMeshObject();
		
		void Update(float t);
		void Load(std::string filepath);
		void Render(GLShaderProgram & program);
	private:
		GLAnimatedMesh mAnimMesh;
	};
}

