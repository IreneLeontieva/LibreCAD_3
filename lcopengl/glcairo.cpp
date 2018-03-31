#include "glcairo.h"

GLCairo::GLCairo(GLCairoSurface *target)
    : GLCairoItem(target->owner())
{
    target->ref();
    mTarget = target;
    mStatus = target->createDraw(&mDraw);
    if (mDraw) {
        mStatus = mDraw->status();
        if (mStatus == CAIRO_STATUS_SUCCESS)
            mDraw->beginPaint();
    }
}
GLCairo::~GLCairo()
{
    if (mDraw)
        mDraw->release();
    mTarget->unref();
}
void GLCairo::setSourceRGB(double r, double b, double g)
{
    if (mStatus != CAIRO_STATIS_SUC)
}
