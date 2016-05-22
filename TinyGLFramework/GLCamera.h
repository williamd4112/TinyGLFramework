#pragma once

#include <glm\glm.hpp>

#include "Transform.h"

namespace tiny_gl
{
	class GLCamera
	{
	public:
		GLCamera();
		~GLCamera();

		virtual glm::mat4 GetViewProjectionMatrix() = 0;
		virtual glm::mat4 GetProjectionMatrix() = 0;

		Transform &GetTransform() { return transform; }
		glm::mat4 GetViewMatrix();
		glm::vec3 Forward();
		glm::vec3 Right();
	protected:
		Transform transform;

	};
}

