#pragma once

#include "tiny_glframework.h"

namespace tiny_gl
{
#define DIRECTIONAL_LIGHT 0x0
#define POINT_LIGHT 0x1
#define SPOT_LIGHT 0x2

	class GLLight
	{
	public:
		GLLight(uint32_t type) { mLightParams.type = type; }
		~GLLight() {}

		void SetAmbient(const glm::vec4 & v)
		{
			mLightParams.ambient[0] = v.r;
			mLightParams.ambient[1] = v.g;
			mLightParams.ambient[2] = v.b;
			mLightParams.ambient[3] = v.a;
		}

		void SetDiffuse(const glm::vec4 & v)
		{
			mLightParams.diffuse[0] = v.r;
			mLightParams.diffuse[1] = v.g;
			mLightParams.diffuse[2] = v.b;
			mLightParams.diffuse[3] = v.a;
		}

		void SetSpecular(const glm::vec4 & v)
		{
			mLightParams.specular[0] = v.r;
			mLightParams.specular[1] = v.g;
			mLightParams.specular[2] = v.b;
			mLightParams.specular[3] = v.a;
		}

		void SetPosition(const glm::vec4 & v)
		{
			mLightParams.position[0] = v.r;
			mLightParams.position[1] = v.g;
			mLightParams.position[2] = v.b;
			mLightParams.position[3] = v.a;
		}

		void SetHalfVector(const glm::vec4 & v)
		{
			mLightParams.halfVector[0] = v.r;
			mLightParams.halfVector[1] = v.g;
			mLightParams.halfVector[2] = v.b;
			mLightParams.halfVector[3] = v.a;
		}

		void SetSpotDirection(const glm::vec4 & v)
		{
			mLightParams.spotDirection[0] = v.r;
			mLightParams.spotDirection[1] = v.g;
			mLightParams.spotDirection[2] = v.b;
			mLightParams.spotDirection[3] = v.a;
		}

		void SetCutoof(float cutoff) { mLightParams.spotCutoff = cutoff; }
		void SetConstantAttenuation(float v) { mLightParams.constantAttenuation = v; }
		void SetLinearAttenuation(float v) { mLightParams.linearAttenuation = v; }
		void SetQudraticAttenuation(float v) { mLightParams.quadraticAttenuation = v; }
	private:
		struct LightSourceParameters {
			uint32_t type;
			float ambient[4];
			float diffuse[4];
			float specular[4];
			float position[4];
			float halfVector[4];
			float spotDirection[4];
			float spotExponent;
			float spotCutoff; // (range: [0.0,90.0], 180.0)
			float spotCosCutoff; // (range: [1.0,0.0],-1.0)
			float constantAttenuation;
			float linearAttenuation;
			float quadraticAttenuation;
		};

		LightSourceParameters mLightParams;
	};

}
