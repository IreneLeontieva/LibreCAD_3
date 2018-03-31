#include "glcairoitem.h"
#include "glcairocontext.h"
#include <assert.h>

GLCairoItem::GLCairoItem(GLCairoContext *owner, const char *type)
    : mOwner(owner)
    , mType(type)
    , mGL(owner->gl())
    , mRefcount(1)
{

}
GLCairoItem::~GLCairoItem()
{
    assert(mRefcount == 0);
}
void GLCairoItem::forceDelete(GLCairoItem *item) {
    item->mRefcount = 0;
    delete item;
}
void GLCairoItem::unref() {
    assert(mRefcount > 0);
    if (--mRefcount == 0) {
        mOwner->unlink(this);
        delete this;
    }
}
