#pragma once

#include "tiny_glframework.h"
#include "GLCamera.h"

namespace tiny_gl
{
	class GLPerspectiveCamera
		: public GLCamera
	{
	public:
		struct PerspectiveConfig
		{
			GLfloat fov;
			GLfloat zNear;
			GLfloat zFar;
			GLfloat aspect;

			PerspectiveConfig(GLfloat fov,
				GLfloat zNear,
				GLfloat zFar,
				GLfloat aspect);
		};

		GLPerspectiveCamera(GLfloat _fov, GLfloat _zNear, GLfloat _zFar, GLfloat _aspect);
		~GLPerspectiveCamera();

		void SetFov(GLfloat fov) { config.fov = fov; }
		void SetZnear(GLfloat zNear) { config.zNear = zNear; }
		void SetZfar(GLfloat zFar) { config.zFar = zFar; }
		void SetAspect(GLfloat aspect) { config.aspect = aspect; }

		PerspectiveConfig &GetConfig() { return config; }
		glm::mat4 GetViewProjectionMatrix();
		glm::mat4 GetProjectionMatrix();
	private:
		PerspectiveConfig config;
	};
}

