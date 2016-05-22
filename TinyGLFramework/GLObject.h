#pragma once

#include "tiny_glframework.h"

#include "Transform.h"
#include "GLShaderProgram.h"

namespace tiny_gl
{
	class GLObject
	{
	public:
		GLObject();
		~GLObject();

		virtual void Update(float t) {}
		virtual void Render(GLShaderProgram & program) = 0;

		Transform & GetTransform() { return mTransform; }
	protected:
		Transform mTransform;
	};

}
