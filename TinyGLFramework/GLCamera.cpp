#include "GLCamera.h"

#include "tiny_glframework.h"

tiny_gl::GLCamera::GLCamera()
{
}


tiny_gl::GLCamera::~GLCamera()
{
}

glm::mat4 tiny_gl::GLCamera::GetViewMatrix()
{
	glm::vec3 m_eye = -transform.Position();
	
	return glm::mat4_cast(transform.Quat()) * glm::translate(glm::mat4(1.0f), m_eye);
}

glm::vec3 tiny_gl::GLCamera::Forward()
{
	glm::vec3 direction;
	glm::vec3 rot = transform.Rotation();

	glm::vec4 t = (glm::mat4_cast(transform.Quat()) * glm::vec4(0, 0, 1, 0));

	glm::mat4 v = GetViewMatrix();

	return glm::normalize(glm::vec3(v[0][2], v[1][2], v[2][2]));
}

glm::vec3 tiny_gl::GLCamera::Right()
{
	return glm::cross(Forward(), glm::vec3(0, 1, 0));
}
