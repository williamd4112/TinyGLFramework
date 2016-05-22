#include "MouseControl.h"

#include "tiny_glframework.h"

tiny_ctrl::MouseControl::MouseControl()
	: mLastX(0), mLastY(0)
{
}


tiny_ctrl::MouseControl::~MouseControl()
{
}

void tiny_ctrl::MouseControl::Update(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
		mButtonPressed = button;
	else if (state == GLUT_UP)
		mButtonPressed = 0;
	Update(x, y);
}

void tiny_ctrl::MouseControl::Update(int x, int y)
{
	mDiff.x = x - mLastX;
	mDiff.y = y - mLastY;
	mLastX = x;
	mLastY = y;
}

glm::vec2 tiny_ctrl::MouseControl::Diff()
{
	return mDiff;
}

int tiny_ctrl::MouseControl::Button()
{
	return mButtonPressed;
}
