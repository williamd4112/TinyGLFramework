#include "GLMesh.h"

tiny_gl::GLMesh::GLMesh()
{
}


tiny_gl::GLMesh::~GLMesh()
{
}

void tiny_gl::GLMesh::Load(std::string filepath)
{
	std::string err;

	load_file(filepath);

	// Load texture groups
	load_texture();

	// Allocate groups
	mGroups.resize(mShapes.size());

	// For Each Shape (or Mesh, Object)
	for (int i = 0; i < mShapes.size(); i++)
	{
		group_t & group = mGroups[i];

		tinyobj::shape_t &shape = mShapes[i];
		group.name = shape.name;
		bool hasNormal = shape.mesh.normals.size() > 0;
		bool hasTex = shape.mesh.texcoords.size() > 0;

		if (!hasNormal)
			shape.mesh.normals.assign(shape.mesh.positions.size(), 0);
		if (!hasTex)
			shape.mesh.normals.assign(shape.mesh.positions.size() - shape.mesh.positions.size() / 3, 0);

		group.matids = shape.mesh.material_ids;
		group.numIndices = shape.mesh.indices.size();

		glGenVertexArrays(1, &group.vaoid);
		glBindVertexArray(group.vaoid);

		glGenBuffers(1, &group.vboid[VBO_POS]);
		glBindBuffer(GL_ARRAY_BUFFER, group.vboid[VBO_POS]);
		glBufferData(GL_ARRAY_BUFFER, shape.mesh.positions.size() * sizeof(float), &shape.mesh.positions[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(POSITION_LOC);
		glVertexAttribPointer(POSITION_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &group.vboid[VBO_NORMAL]);
		glBindBuffer(GL_ARRAY_BUFFER, group.vboid[VBO_NORMAL]);
		glBufferData(GL_ARRAY_BUFFER, shape.mesh.normals.size() * sizeof(float), &shape.mesh.normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(NORMAL_LOC);
		glVertexAttribPointer(NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &group.vboid[VBO_TEXCOORD]);
		glBindBuffer(GL_ARRAY_BUFFER, group.vboid[VBO_TEXCOORD]);
		glBufferData(GL_ARRAY_BUFFER, shape.mesh.texcoords.size() * sizeof(float), &shape.mesh.texcoords[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(TEXCOORD_LOC);
		glVertexAttribPointer(TEXCOORD_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);

		std::unordered_map<GLint, std::vector<unsigned int>> indexGroupMap;
		for (int j = 0; j < shape.mesh.indices.size() / 3; j++)
		{
			GLint matid = shape.mesh.material_ids[j];
			std::vector<unsigned int> &indexGroup = indexGroupMap[matid];
			indexGroup.push_back(shape.mesh.indices[j * 3]);
			indexGroup.push_back(shape.mesh.indices[j * 3 + 1]);
			indexGroup.push_back(shape.mesh.indices[j * 3 + 2]);
		}


		int igIndex = 0;
		group.indexGroups.resize(indexGroupMap.size());
		for (std::unordered_map<GLint, std::vector<unsigned int>>::iterator it = indexGroupMap.begin();
			it != indexGroupMap.end();
			it++)
		{
			group.indexGroups[igIndex].matid = it->first;
			group.indexGroups[igIndex].numIndices = it->second.size();
			glGenBuffers(1, &group.indexGroups[igIndex].ibo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, group.indexGroups[igIndex].ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, it->second.size() * sizeof(unsigned int), &it->second[0], GL_STATIC_DRAW);
			igIndex++;
		}
		glBindVertexArray(0);

	}
}

void tiny_gl::GLMesh::load_file(std::string filepath)
{
	std::string err;
	if (!tinyobj::LoadObj(mShapes, mMaterials, err, filepath.c_str()))
		throw tiny_gl_exception_t("Load error");
}

void tiny_gl::GLMesh::load_texture()
{
	// Allocate texture group
	mTextureGroups.resize(mMaterials.size());

	// For Each Material
	for (int i = 0; i < mMaterials.size(); i++)
	{
		ILuint ilTexName;
		ilGenImages(1, &ilTexName);
		ilBindImage(ilTexName);

		if (ilLoadImage(mMaterials[i].diffuse_texname.c_str()) && ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE))
		{
			unsigned char *data = new unsigned char[ilGetInteger(IL_IMAGE_WIDTH) * ilGetInteger(IL_IMAGE_HEIGHT) * 4];
			ilCopyPixels(0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 1, IL_RGBA, IL_UNSIGNED_BYTE, data);

			glGenTextures(1, &mTextureGroups[i].diffuseid);
			glBindTexture(GL_TEXTURE_2D, mTextureGroups[i].diffuseid);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			delete[] data;

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		ilDeleteImages(1, &ilTexName);
	}
}
