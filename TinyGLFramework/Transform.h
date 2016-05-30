#pragma once

#include "tiny_glframework.h"

namespace tiny_gl
{
	class Transform
	{
	public:
		Transform(const glm::vec3 _position, const glm::vec3 _rotation, const glm::vec3 _scale);
		Transform();
		~Transform();

		glm::mat4 GetTransformMatrix();
		glm::mat4 GetInverseTransformMatrix();

		void Translate(const glm::vec3 v);
		void Rotate(const glm::vec3 eulerAngles);
		void Rotate(const glm::vec3 axis, float angle);
		void RotateEuler(const glm::vec3 eulerAngles);
		void Scale(const glm::vec3 v);
		void TranslateTo(const glm::vec3 v);
		void RotateTo(const glm::vec3 eulerAngles);
		void ScaleTo(const glm::vec3 v);
		const glm::vec3 &Position() const { return position; }
		const glm::vec3 Up() const { return up; }
		const glm::vec3 Forward() const { return forward; }
		const glm::vec3 Right() const { return right; }
		const glm::quat &Rotation() const { return rotation; }
		const glm::vec3 &Scale() const { return scale; }
		const glm::vec3 Orientation();
		const glm::quat &Quat() const { return rotation; }
	private:
		glm::vec3 forward;
		glm::vec3 right;
		glm::vec3 up;

		glm::vec3 position;

		glm::quat rotation;
		glm::vec3 eulerAngle;

		glm::vec3 scale;
		glm::mat4 transformMatrix; // Update when Translate, Rotate, Scale

		void updateTransformMatrix();
		glm::quat eulerToQuat(const glm::vec3 &eulerAngle);
	};
}

