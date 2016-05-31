#include "Transform.h"

#include "tiny_glframework.h"

using namespace glm;

tiny_gl::Transform::Transform() :
	position(0), scale(glm::vec3(1, 1, 1)), forward(0, 0, 1), up(0, 1, 0), right(1, 0, 0), eulerAngle(0, 0, 0)
{

}

tiny_gl::Transform::Transform(
	const vec3 _position = vec3(0), 
	const vec3 _rotation = vec3(0), 
	const vec3 _scale = vec3(1, 1, 1)) :
	position(_position), 
	rotation(_rotation), 
	scale(_scale), 
	transformMatrix(mat4(1.0f)), forward(0, 0, 1), up(0, 1, 0), right(1, 0, 0), eulerAngle(0, 0, 0)
{

}

tiny_gl::Transform::~Transform()
{
}

glm::mat4 tiny_gl::Transform::GetTransformMatrix()
{
	return transformMatrix;
}

glm::mat4 tiny_gl::Transform::GetInverseTransformMatrix()
{
	return glm::inverse(transformMatrix);
}

void tiny_gl::Transform::Translate(const vec3 v)
{
	position += v;
	updateTransformMatrix();
}

void tiny_gl::Transform::Rotate(const vec3 eulerAngles)
{
	glm::quat qPitch = glm::angleAxis(radians(eulerAngles.x), glm::vec3(1, 0, 0));
	forward = glm::normalize(vec3(glm::mat4_cast(qPitch) * vec4(forward, 0.0)));
	up = glm::normalize(vec3(glm::mat4_cast(qPitch) * vec4(up, 0.0)));
	right = glm::normalize(vec3(glm::mat4_cast(qPitch) * vec4(right, 0.0)));

	glm::quat qYaw = glm::angleAxis(radians(eulerAngles.y), glm::vec3(0, 1, 0));
	forward = glm::normalize(vec3(glm::mat4_cast(qYaw) * vec4(forward, 0.0)));
	up = glm::normalize(vec3(glm::mat4_cast(qYaw) * vec4(up, 0.0)));
	right = glm::normalize(vec3(glm::mat4_cast(qYaw) * vec4(right, 0.0)));

	glm::quat qRoll = glm::angleAxis(radians(eulerAngles.z), glm::vec3(0, 0, 1));
	forward = glm::normalize(vec3(glm::mat4_cast(qRoll) * vec4(forward, 0.0)));
	up = glm::normalize(vec3(glm::mat4_cast(qRoll) * vec4(up, 0.0)));
	right = glm::normalize(vec3(glm::mat4_cast(qRoll) * vec4(right, 0.0)));

	rotation = qRoll * qYaw * qPitch * rotation;
	eulerAngle = glm::eulerAngles(rotation);

	updateTransformMatrix();
}

void tiny_gl::Transform::Rotate(const glm::vec3 axis, float angle)
{
	rotation = glm::normalize(glm::angleAxis(angle, axis) * rotation);
	updateTransformMatrix();
	forward = glm::vec3(glm::mat4_cast(rotation) * glm::vec4(forward, 0.0));
}

void tiny_gl::Transform::RotateEuler(const glm::vec3 eulerAngles)
{
	eulerAngle += vec3(radians(eulerAngles.x), radians(eulerAngles.y), radians(eulerAngles.z));
	glm::mat4 mPitch = glm::rotate(mat4(1.0), eulerAngle.x, vec3(1, 0, 0));
	glm::mat4 mYaw = glm::rotate(mat4(1.0), eulerAngle.y, vec3(0, 1, 0));
	glm::mat4 mRoll = glm::rotate(mat4(1.0), eulerAngle.z, vec3(0, 0, 1));
	glm::mat4 mRot = mPitch * mYaw;

	forward = glm::normalize(vec3(mRot  * vec4(forward, 0.0)));
	up = glm::normalize(vec3(mRot  * vec4(up, 0.0)));
	right = glm::normalize(vec3(mRot  * vec4(right, 0.0)));

	rotation = glm::quat(mRot);

	updateTransformMatrix();
}

void tiny_gl::Transform::Scale(const glm::vec3 v)
{
	scale += v;
	updateTransformMatrix();
}

void tiny_gl::Transform::TranslateTo(const glm::vec3 v)
{
	position = v;
	updateTransformMatrix();
}

void tiny_gl::Transform::RotateTo(const glm::vec3 eulerAngles)
{
	Rotate(eulerAngles - glm::eulerAngles(rotation));
}

void tiny_gl::Transform::ScaleTo(const glm::vec3 v)
{
	scale = v;
	updateTransformMatrix();
}

const glm::vec3 tiny_gl::Transform::Orientation()
{
	return forward;
}

void tiny_gl::Transform::updateTransformMatrix()
{
	// Translation matrix
	glm::mat4 translateMatrix = glm::translate(mat4(1.0f), position);

	// Rotation matrix
	glm::mat4 rotateMatrix = glm::mat4_cast(rotation);

	// Scale matrix
	glm::mat4 scaleMatrix = glm::scale(mat4(1.0f), scale);

	transformMatrix = translateMatrix * rotateMatrix * scaleMatrix;
}

glm::quat tiny_gl::Transform::eulerToQuat(const vec3 &eulerAngle)
{
	glm::quat qPitch = glm::angleAxis(radians(eulerAngle.x), glm::vec3(1, 0, 0));
	glm::quat qYaw = glm::angleAxis(radians(eulerAngle.y), glm::vec3(0, 1, 0));
	glm::quat qRoll = glm::angleAxis(radians(eulerAngle.z), glm::vec3(0, 0, 1));
	
	return qPitch * qYaw * qRoll;
}
