#pragma once

#include "tiny_glframework.h"

namespace tiny_gl
{
	class GLMesh
	{
		friend class GLMeshObject;

		struct group_t;
		struct texture_group_t;
	public:
#define NUM_VBO 4
#define VBO_POS 0
#define VBO_NORMAL 1
#define VBO_TEXCOORD 2
#define VBO_INDEX 3

		struct texture_group_t
		{
			GLuint diffuseid;
			GLuint bumpid;
			GLuint specularid;

			texture_group_t() : diffuseid(0), bumpid(0), specularid(0) {}
			~texture_group_t() {}
		};

		struct index_group_t
		{
			GLuint ibo;
			GLint matid;
			unsigned int numIndices;
		};

		struct group_t
		{
			std::string name;
			uint32_t numIndices;
			GLuint vaoid;
			std::vector<GLint> matids;
			GLuint vboid[NUM_VBO];
			std::vector<index_group_t> indexGroups;
		};

		GLMesh();
		~GLMesh();

		const std::vector<group_t> & Groups() const { return mGroups; }
		const std::vector<texture_group_t> & Textures() const { return mTextureGroups; }
		const std::vector<tinyobj::material_t> & Materials() const { return mMaterials; }
		const std::vector<float> & GlowCoffs() const { return mGlowCoffs; }

		void SetupGlow(int index, float power);
	protected:

		std::vector<tinyobj::shape_t> mShapes;
		std::vector<group_t> mGroups;
		std::vector<tinyobj::material_t> mMaterials;
		std::vector<texture_group_t> mTextureGroups;
		std::vector<float> mGlowCoffs;

		virtual void Load(std::string filepath);
		virtual void load_file(std::string filepath);
		virtual void load_texture();
		void load_diffuse(tinyobj::material_t & mat, texture_group_t & tex_group);
		void load_specular(tinyobj::material_t & mat, texture_group_t & tex_group);
	};
}

