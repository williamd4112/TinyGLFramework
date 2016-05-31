#include "GLTangentMesh.h"



tiny_gl::GLTangentMesh::GLTangentMesh()
{
}


tiny_gl::GLTangentMesh::~GLTangentMesh()
{
}

void tiny_gl::GLTangentMesh::Load(std::string filepath)
{
	GLMesh::Load(filepath);
	GLuint tangent_buffer;
	
	auto groups = Groups();

	for (int i = 0; i < groups.size(); i++)
	{
		auto group = groups[i];
		auto shape = mShapes[i];
		glBindVertexArray(group.vaoid);

		// Compute tangent
		float *tangents = new float[shape.mesh.normals.size()];
		for (unsigned int i = 0; i < shape.mesh.indices.size(); i += 3)
		{
			int idx0 = shape.mesh.indices[i];
			int idx1 = shape.mesh.indices[i + 1];
			int idx2 = shape.mesh.indices[i + 2];

			float dp1[3];
			float dp2[3];
			float duv1[2];
			float duv2[2];

			dp1[0] = shape.mesh.positions[idx1 * 3 + 0] - shape.mesh.positions[idx0 * 3 + 0];
			dp1[1] = shape.mesh.positions[idx1 * 3 + 1] - shape.mesh.positions[idx0 * 3 + 1];
			dp1[2] = shape.mesh.positions[idx1 * 3 + 2] - shape.mesh.positions[idx0 * 3 + 2];

			dp2[0] = shape.mesh.positions[idx2 * 3 + 0] - shape.mesh.positions[idx0 * 3 + 0];
			dp2[1] = shape.mesh.positions[idx2 * 3 + 1] - shape.mesh.positions[idx0 * 3 + 1];
			dp2[2] = shape.mesh.positions[idx2 * 3 + 2] - shape.mesh.positions[idx0 * 3 + 2];

			duv1[0] = shape.mesh.texcoords[idx1 * 2 + 0] - shape.mesh.texcoords[idx0 * 2 + 0];
			duv1[1] = shape.mesh.texcoords[idx1 * 2 + 1] - shape.mesh.texcoords[idx0 * 2 + 1];

			duv2[0] = shape.mesh.texcoords[idx2 * 2 + 0] - shape.mesh.texcoords[idx0 * 2 + 0];
			duv2[1] = shape.mesh.texcoords[idx2 * 2 + 1] - shape.mesh.texcoords[idx0 * 2 + 1];

			float r = 1.0f / (duv1[0] * duv2[1] - duv1[1] * duv2[0]);

			float t[3];
			t[0] = (dp1[0] * duv2[1] - dp2[0] * duv1[1]) * r;
			t[1] = (dp1[1] * duv2[1] - dp2[1] * duv1[1]) * r;
			t[2] = (dp1[2] * duv2[1] - dp2[2] * duv1[1]) * r;

			tangents[idx0 * 3 + 0] += t[0];
			tangents[idx0 * 3 + 1] += t[1];
			tangents[idx0 * 3 + 2] += t[2];

			tangents[idx1 * 3 + 0] += t[0];
			tangents[idx1 * 3 + 1] += t[1];
			tangents[idx1 * 3 + 2] += t[2];

			tangents[idx2 * 3 + 0] += t[0];
			tangents[idx2 * 3 + 1] += t[1];
			tangents[idx2 * 3 + 2] += t[2];
		}
		glGenBuffers(1, &tangent_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, tangent_buffer);
		glBufferData(GL_ARRAY_BUFFER, shape.mesh.normals.size() * sizeof(float), tangents, GL_STATIC_DRAW);
		glEnableVertexAttribArray(TANGENT_LOC);
		glVertexAttribPointer(TANGENT_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);
		delete tangents;

		glBindVertexArray(0);
	}
}

void tiny_gl::GLTangentMesh::load_texture()
{
	GLMesh::load_texture();
	for (int i = 0; i < mMaterials.size(); i++)
	{
		ILuint ilTexName;
		ilGenImages(1, &ilTexName);
		ilBindImage(ilTexName);

		if (ilLoadImage(mMaterials[i].bump_texname.c_str()) && ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE))
		{
			std::string bname = mMaterials[i].bump_texname.c_str();
			unsigned char *data = new unsigned char[ilGetInteger(IL_IMAGE_WIDTH) * ilGetInteger(IL_IMAGE_HEIGHT) * 4];
			ilCopyPixels(0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 1, IL_RGB, IL_UNSIGNED_BYTE, data);

			glActiveTexture(GL_TEXTURE1);
			glGenTextures(1, &mTextureGroups[i].bumpid);
			glBindTexture(GL_TEXTURE_2D, mTextureGroups[i].bumpid);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGB, GL_UNSIGNED_BYTE, data);

			delete[] data;
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		ilDeleteImages(1, &ilTexName);
	}
}
