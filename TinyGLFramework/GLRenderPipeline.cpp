#include "GLRenderPipeline.h"



tiny_gl::GLRenderPipeline::GLRenderPipeline()
{
}


tiny_gl::GLRenderPipeline::~GLRenderPipeline()
{
}

void tiny_gl::GLRenderPipeline::SetScene(GLScene * scene)
{
	mScenePtr = scene;
}

void tiny_gl::GLRenderPipeline::Render(int w, int h)
{
	std::vector<GLShaderProgram>::iterator sit = mShaderPrograms.begin();
	std::vector<GLFrameBufferObject>::iterator fit = mFrameBuffers.begin();

	while (sit != mShaderPrograms.end())
	{
		// Bind current fbo
		bool isLastFbo = fit == mFrameBuffers.end();
		std::vector<GLFrameBufferObject>::iterator fit_lookahead = (isLastFbo) ? fit : fit + 1;
		GLFrameBufferObject screenFbo = GLFrameBufferObject(w, h);
		GLFrameBufferObject & current_fbo = (isLastFbo) ? screenFbo : *fit;
		GLFrameBufferObject & next_fbo = (fit_lookahead == mFrameBuffers.end()) ? screenFbo : *fit_lookahead;

		switch (sit->GetType())
		{
		case SCENE:
			current_fbo.BindFBO();
			sit->Render(*mScenePtr);
			break;
		case FBO:
			next_fbo.BindFBO();
			sit->Render(current_fbo);
			fit++;
			break;
		default:
			break;
		}
		sit++;
	}
}
