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

		virtual void Load(std::string filepath);
		void SetIndirect();
		void Render(GLShaderProgram & program);
		bool HasIndirect();
		const GLMesh & Mesh() { if (mMesh == nullptr) throw tiny_gl_exception_t("Mesh not loaded");  return *mMesh; }
	protected:
		GLMesh *mMesh;		
		bool mHasIndirect;
	};
}

