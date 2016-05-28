#pragma once

#include "GLCamera.h"
#include "GLPerspectiveCamera.h"
#include "GLObject.h"
#include "GLLight.h"

#include "tiny_glframework.h"


namespace tiny_gl
{
	class GLObject;
	class GLLight;

	class GLScene
	{
		friend class GLShaderProgram;
	public:
		GLScene();
		~GLScene();

		GLCamera & GetCamera() const { if (mCamera == nullptr) throw tiny_gl_exception_t("Camera not set");  return *mCamera; }
		
		template <class T>
		T & GetCamera() const { if (mCamera == nullptr) throw tiny_gl_exception_t("Camera not set");  return *static_cast<T*>(mCamera); }

		GLObject & GetGLObject(int32_t id) 
		{ 
			if (id >= 0 && id < mObjects.size())
				return *mObjects[id];
			else
				throw tiny_gl::tiny_gl_exception_t("Id out of bound.");
		}

		const std::vector<GLObject *> & GetObjects() const { return mObjects; }
		const std::vector<GLLight *> & GetLights() const { return mLights; }

		GLLight & GetGLLight(int32_t id) const 
		{
			if (id >= 0 && id < mLights.size())
				return *mLights[id];
			else
				throw tiny_gl::tiny_gl_exception_t("Id out of bound.");
		}

		int32_t CreateMeshObject(std::string filepath);
		int32_t CreateAnimateMeshObject(std::string filepath);
		int32_t CreateTangentMeshObject(std::string filepath);
		void CreatePerspectiveCamera(float fov, float znear, float zfar, float aspect);

		int32_t CreateDirectionalLight(
			glm::vec3 dir, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);

		int32_t CreatePointLight(
			glm::vec3 pos, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, 
			float constantAtt, float linearAtt, float quadraticAtt);

		int32_t CreateSpotLight(
			glm::vec3 pos, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, 
			float constantAtt, float linearAtt, float quadraticAtt, float cutoff);
		void Update(float t);
	private:
		GLCamera * mCamera;
		std::vector<GLLight *> mLights;
		std::vector<GLObject*> mObjects;
	};

}
