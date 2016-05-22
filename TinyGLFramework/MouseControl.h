#pragma once

#include <glm\glm.hpp>

namespace tiny_ctrl
{
	class MouseControl
	{
	public:
		MouseControl();
		~MouseControl();

		void Update(int button, int state, int x, int y);
		void Update(int x, int y);
		glm::vec2 Diff();
		int Button();
	private:
		int mLastX;
		int mLastY;
		glm::vec2 mDiff;

		int mButtonPressed;
	};

}
