#pragma once

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstring>
#include <algorithm>
#include <cassert>
#include <cmath>

#include <glew.h>
#include <freeglut.h>
#include <IL/il.h>

#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <tiny_obj_loader.h>

namespace tiny_gl
{
#define Warning(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)

	struct tiny_gl_exception_t
	{
		std::string msg;
		uint32_t code;

		tiny_gl_exception_t(std::string _msg, uint32_t _code) : msg(_msg), code(_code) {}
		tiny_gl_exception_t(std::string _msg) : msg(_msg), code(0) {}
	};
}