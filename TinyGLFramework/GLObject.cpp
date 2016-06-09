#include "GLObject.h"



tiny_gl::GLObject::GLObject(): mObjectType(0), mVariant(0)
{
}

tiny_gl::GLObject::GLObject(int type): mObjectType(type), mVariant(0)
{
}

tiny_gl::GLObject::~GLObject()
{
}
