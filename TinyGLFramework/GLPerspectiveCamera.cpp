#include "GLPerspectiveCamera.h"

using namespace glm;

tiny_gl::GLPerspectiveCamera::GLPerspectiveCamera(GLfloat _fov, GLfloat _zNear, GLfloat _zFar, GLfloat _aspect)
	: GLCamera(), config(_fov, _zNear, _zFar, _aspect)
{
}

tiny_gl::GLPerspectiveCamera::~GLPerspectiveCamera()
{
}

glm::mat4 tiny_gl::GLPerspectiveCamera::GetViewProjectionMatrix()
{
	return GetProjectionMatrix() * GetViewMatrix() * mat4(1.0f);
}

glm::mat4 tiny_gl::GLPerspectiveCamera::GetProjectionMatrix()
{
	return glm::perspective(
		glm::radians(config.fov), config.aspect, config.zNear, config.zFar);
}

tiny_gl::GLPerspectiveCamera::PerspectiveConfig::PerspectiveConfig(GLfloat _fov, GLfloat _zNear, GLfloat _zFar, GLfloat _aspect)
	: fov(_fov), zNear(_zNear), zFar(_zFar), aspect(_aspect)
{
}
