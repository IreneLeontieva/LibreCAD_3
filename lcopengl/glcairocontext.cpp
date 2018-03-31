#include "glcairocontext.h"
#include <assert.h>

GLCairoContext::GLCairoContext()
    : mGL(nullptr)
{
    auto gl = GLContext::create();
    mGL = gl;
}
GLCairoContext::~GLCairoContext()
{
    for(auto i = mItems.cbegin(); i != mItems.cend(); ++i)
        GLCairoItem::forceDelete(*i);//will not call unlink
    delete (GLContext*)mGL;
}
void GLCairoContext::unlink(GLCairoItem *item) {
    auto n = mItems.erase(item);
    assert(n == 1);
}
