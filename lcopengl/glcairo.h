#pragma once
#include "common.h"
#include "glcairosurface.h"
#include "glcairodraw.h"
#include <cairo/cairo.h>

class PUBLIC_API GLCairo : public GLCairoItem {
public:
    CAIRO_ITEM

    GLCairo(GLCairoSurface * target);
    cairo_status_t status() const { return mStatus; }

    void setSourceRGB(double r, double b, double g);
    void setSourceRGBA(double r, double b, double g, double a);

private:
    ~GLCairo();

    cairo_status_t   mStatus;
    GLCairoSurface * mTarget;
    GLCairoDraw    * mDraw;
};
