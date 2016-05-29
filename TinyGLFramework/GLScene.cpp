#include "GLScene.h"

#include "GLMeshObject.h"
#include "GLAnimatedMeshObject.h"
#include "GLTangentMeshObject.h"

using namespace glm;

tiny_gl::GLScene::GLScene()
	: mCamera(nullptr), mSkybox(nullptr)
{
}


tiny_gl::GLScene::~GLScene()
{
	for (int i = 0; i < mObjects.size(); i++)
		delete mObjects[i];
	for (int i = 0; i < mLights.size(); i++)
		delete mLights[i];
	if(mCamera != nullptr)
		delete mCamera;
	if(mSkybox != nullptr)
		delete mSkybox;
}

int32_t tiny_gl::GLScene::CreateMeshObject(std::string filepath)
{
	GLMeshObject * obj = new GLMeshObject;
	obj->Load(filepath);
	mObjects.push_back(obj);

	return mObjects.size() - 1;
}

int32_t tiny_gl::GLScene::CreateAnimateMeshObject(std::string filepath)
{
	GLAnimatedMeshObject * obj = new GLAnimatedMeshObject;
	obj->Load(filepath);
	mObjects.push_back(obj);

	return mObjects.size() - 1;
}

int32_t tiny_gl::GLScene::CreateTangentMeshObject(std::string filepath)
{
	GLTangentMeshObject * tobj = new GLTangentMeshObject;
	tobj->Load(filepath);
	mObjects.push_back(tobj);

	return mObjects.size() - 1;
}

void tiny_gl::GLScene::CreatePerspectiveCamera(float fov, float znear, float zfar, float aspect)
{
	GLPerspectiveCamera * cam = new GLPerspectiveCamera(fov, znear, zfar, aspect);
	if (mCamera == nullptr)
		mCamera = cam;
	else
	{
		Warning("GLScene::CreatePerspectiveCamera: replacing camera.");
		delete mCamera;
		mCamera = cam;
	}
}

int32_t tiny_gl::GLScene::CreateDirectionalLight(
	glm::vec3 dir, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
{
	GLLight * light = new GLLight(DIRECTIONAL_LIGHT);
	light->SetPosition(glm::vec4(dir.x, dir.y, dir.z, 0.0));
	light->SetAmbient(vec4(ambient, 1.0));
	light->SetDiffuse(vec4(diffuse, 1.0));
	light->SetSpecular(vec4(specular, 1.0));

	mLights.push_back(light);
	
	return mLights.size() - 1;
}

int32_t tiny_gl::GLScene::CreatePointLight(
	glm::vec3 pos, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, 
	float constantAtt, float linearAtt, float quadraticAtt)
{
	GLLight * light = new GLLight(POINT_LIGHT);
	light->SetPosition(glm::vec4(pos, 0.0));
	light->SetAmbient(vec4(ambient, 1.0));
	light->SetDiffuse(vec4(diffuse, 1.0));
	light->SetSpecular(vec4(specular, 1.0));
	light->SetConstantAttenuation(constantAtt);
	light->SetLinearAttenuation(linearAtt);
	light->SetQudraticAttenuation(quadraticAtt);

	mLights.push_back(light);

	return mLights.size() - 1;
}

int32_t tiny_gl::GLScene::CreateSpotLight(
	glm::vec3 pos, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, 
	float constantAtt, float linearAtt, float quadraticAtt, float cutoff)
{
	GLLight * light = new GLLight(SPOT_LIGHT);
	light->SetPosition(glm::vec4(pos, 0.0));
	light->SetAmbient(vec4(ambient, 1.0));
	light->SetDiffuse(vec4(diffuse, 1.0));
	light->SetSpecular(vec4(specular, 1.0));
	light->SetConstantAttenuation(constantAtt);
	light->SetLinearAttenuation(linearAtt);
	light->SetQudraticAttenuation(quadraticAtt);
	light->SetCutoof(cutoff);

	mLights.push_back(light);

	return mLights.size() - 1;
}

void tiny_gl::GLScene::CreateSkybox(std::vector<std::string> & filenames)
{
	mSkybox = new GLSkybox;
	mSkybox->Load(filenames);
}

void tiny_gl::GLScene::CreateSkybox(std::string filename)
{
	mSkybox = new GLSkybox;
	mSkybox->Load(filename);
}

void tiny_gl::GLScene::Update(float t)
{
	for (GLObject * obj : mObjects)
		obj->Update(t);
}
