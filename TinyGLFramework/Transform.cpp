#include "Transform.h"

#include "tiny_glframework.h"

using namespace glm;

tiny_gl::Transform::Transform() :
	position(0), scale(glm::vec3(1, 1, 1))
{

}

tiny_gl::Transform::Transform(
	const vec3 _position = vec3(0), 
	const vec3 _rotation = vec3(0), 
	const vec3 _scale = vec3(1, 1, 1)) :
	position(_position), 
	rotation(_rotation), 
	scale(_scale), 
	transformMatrix(mat4(1.0f))
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
	glm::quat key_quat = glm::quat(vec3(radians(eulerAngles.x), radians(eulerAngles.y), radians(eulerAngles.z)));
	rotation = key_quat * rotation;
	rotation = glm::normalize(rotation);
	updateTransformMatrix();

	glm::vec3 a = glm::eulerAngles(rotation);
	printf("angle(%f, %f, %f)\n", glm::degrees(a.x), glm::degrees(a.y), glm::degrees(a.z));
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
	vec3 result;
	float y = Rotation().y;
	float p = Rotation().x;

	result.z = cos(y)*cos(p);
	result.y = sin(p);
	result.x = -sin(y)*cos(p);

	return result;
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
