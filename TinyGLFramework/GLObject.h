#pragma once

#include "tiny_glframework.h"

#include "Transform.h"
#include "GLShaderProgram.h"

#define TYPE_MESHOBJ 1
#define TYPE_ANIMMESHOBJ 2

namespace tiny_gl
{
	class GLObject
	{
	public:
		GLObject();
		GLObject(int type);
		~GLObject();

		virtual void Update(float t) {}
		virtual void Render(GLShaderProgram & program) = 0;

		Transform & GetTransform() { return mTransform; }
		int GetType() { return mObjectType; }
		int GetVariant() { return mVariant; }
		void SetVariant(int var) { mVariant = var; }
	protected:
		int mObjectType;
		int mVariant;
		Transform mTransform;
	};

}
