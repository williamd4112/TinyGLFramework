#pragma once

#include "tiny_glframework.h"
#include "GLScene.h"
#include "GLShaderProgram.h"
#include "GLFrameBufferObject.h"

namespace tiny_gl
{
	/*
		GLRenderPipeline
		Manager of GLScene, GLShaderProgram. DON'T release GLScene, GLShaderProgram outside.
	*/
	class GLRenderPipeline
	{
	public:
		GLRenderPipeline();
		~GLRenderPipeline();

		void AddShaderProgram(GLShaderProgram program) { mShaderPrograms.push_back(program); }
		void AddFrameBufferObject(GLFrameBufferObject fbo) { mFrameBuffers.push_back(fbo); }
		void SetScene(GLScene * scene);
		void Render(int w, int h);
	private:
		GLScene *mScenePtr;
		std::vector<GLShaderProgram> mShaderPrograms;
		std::vector<GLFrameBufferObject> mFrameBuffers;
	};

}
