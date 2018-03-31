#include "glcairodraw.h"
#include "glcairosurface.h"
#include "glcairocontext.h"
#include "glcairoutil.h"
#include "glcontext.h"
#include <cmath>
#include <assert.h>

GLCairoDraw::GLCairoDraw(GLCairoSurface *surface,
                         GLuint texture,
                         int viewportx1,
                         int viewporty1,
                         int viewportx2,
                         int viewporty2)
{
    mStatus = CAIRO_STATUS_DEVICE_ERROR;
    mReferencedFromDraw    = false;
    mReferencedFromSurface = true;
    mReferencedFromCairo   = false;
    mPrevious              = nullptr;
    mRootDraw              = this;
    mContext               = surface->owner();

    mX1 = viewportx1;
    mY1 = viewporty1;
    mX2 = viewportx2;
    mY2 = viewporty2;
    mMatrix.x0 = 0.0;
    mMatrix.y0 = 0.0;
    mMatrix.xx = 1.0;
    mMatrix.xy = 0.0;
    mMatrix.yy = 0.0;
    mMatrix.yx = 0.0;

    if (mX2 <= mX1 || mY2 <= mY1)
        return;

    if (createResources(surface->getContent() == CAIRO_CONTENT_ALPHA ?
            0 : texture,
            surface->width(),
            surface->height()))
        mStatus = CAIRO_STATUS_SUCCESS;
}
GLCairoDraw::GLCairoDraw(GLCairoDraw *draw,
                         int viewportx1,
                         int viewporty1,
                         int viewportx2,
                         int viewporty2)
{
    mStatus = CAIRO_STATUS_DEVICE_ERROR;
    mReferencedFromDraw    = false;
    mReferencedFromSurface = true;
    mReferencedFromCairo   = false;
    mPrevious              = draw;
    mRootDraw              = draw->mRootDraw;
    mContext               = draw->mContext;
    draw->mReferencedFromDraw = true;
    mX1 = viewportx1;
    mY1 = viewporty1;
    mX2 = viewportx2;
    mY2 = viewporty2;
    mMatrix = draw->mMatrix;

    if (mX2 <= mX1 || mY2 <= mY1)
        return;

    mOwnFramebuffer = false;
    mFramebuffer    = draw->mFramebuffer;
    mDepthStencil   = 0;
    mTexture        = 0;
    mProxyTexture   = 0;
    mStatus = CAIRO_STATUS_SUCCESS;
}
GLCairoDraw::GLCairoDraw(GLCairoDraw *draw,
                         bool newbuffer)
{
    mStatus = CAIRO_STATUS_DEVICE_ERROR;
    mReferencedFromDraw    = false;
    mReferencedFromSurface = true;
    mReferencedFromCairo   = false;
    mPrevious              = draw;
    mRootDraw              = draw->mRootDraw;
    mContext               = draw->mContext;
    draw->mReferencedFromDraw = true;
    mX1 = draw->mX1;
    mY1 = draw->mY1;
    mX2 = draw->mX2;
    mY2 = draw->mY2;
    mMatrix = draw->mMatrix;

    if (mX2 <= mX1 || mY2 <= mY1)
        return;

    mOwnFramebuffer = false;
    mFramebuffer    = draw->mFramebuffer;
    mDepthStencil   = 0;
    mTexture        = 0;
    mProxyTexture   = 0;

    if (!newbuffer || createResources(0, mX2-mX1, mY2-mY1))
        mStatus = CAIRO_STATUS_SUCCESS;
}
bool GLCairoDraw::createResources(GLuint inherit_texture,
                                  unsigned w, unsigned h)
{
    auto gl = mContext->gl();
    gl->clearError();

    GLuint fbo = 0, d24s8 = 0, rgba = 0;
    do {
        gl->glGenFramebuffers(1, &fbo);
        if (!fbo) break;
        gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

        gl->glGenTextures(1, &d24s8);
        if (!d24s8) break;
        gl->glBindTexture(GL_TEXTURE_2D, d24s8);
        gl->glTexImage2D(GL_TEXTURE_2D,
                         0,
                         GL_DEPTH24_STENCIL8,
                         mX2-mX1,
                         mY2-mY1,
                         0,
                         GL_DEPTH_STENCIL,
                         GL_UNSIGNED_INT_24_8,
                         NULL);
        gl->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                                   GL_DEPTH_STENCIL_ATTACHMENT,
                                   GL_TEXTURE_2D,
                                   d24s8, 0);
        if (!inherit_texture) {
            gl->glGenTextures(1, &rgba);
            if (!rgba) break;
            gl->glBindTexture(GL_TEXTURE_2D, rgba);
            gl->glTexImage2D(GL_TEXTURE_2D,
                             0,
                             GL_RGBA8UI,
                             mX2-mX1,
                             mY2-mY1,
                             0,
                             GL_BGRA,
                             GL_UNSIGNED_INT_8_8_8_8_REV,
                             NULL);
            gl->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_2D,
                                       rgba, 0);
        } else {
            gl->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_2D,
                                       inherit_texture, 0);
        }
    } while(0);
    GLenum e = gl->checkError();
    if (e == GL_NO_ERROR) {
        mOwnFramebuffer = true;
        mFramebuffer    = fbo;
        mDepthStencil   = d24s8;
        mTexture        = inherit_texture;
        mProxyTexture   = rgba;
        return true;
    }
    if (fbo) gl->glDeleteFramebuffers(1, &fbo);
    if (d24s8) gl->glDeleteTextures(1, &d24s8);
    if (rgba) gl->glDeleteTextures(1, &rgba);
    return false;
}
void GLCairoDraw::beginPaint() {
    assert(mReferencedFromCairo == false);
    mReferencedFromCairo = true;
}
void GLCairoDraw::endPaint() {
    assert(mReferencedFromCairo == true);
    mReferencedFromCairo = false;
    //if (mReferencedFromDraw) WARNING!!!!!
    bool refs = mReferencedFromSurface |
                mReferencedFromDraw;
    if (!refs)
        delete this;
}
void GLCairoDraw::release() {
    assert(mReferencedFromSurface == true);
    mReferencedFromSurface = false;
    bool refs = mReferencedFromCairo |
                mReferencedFromDraw;
    if (!refs)
        delete this;
}
bool GLCairoDraw::finish() {
    assert(mReferencedFromSurface == true);
    //cannot finish when someone is painting on me
    if (mReferencedFromCairo | mReferencedFromDraw)
        return false;
    delete this;
    return true;
}
GLCairoDraw::~GLCairoDraw() {
    assert((mReferencedFromCairo |
            mReferencedFromDraw |
            mReferencedFromSurface) == false);

}
/*
 * drawing paths
 */
void GLCairoDraw::newPath() {

}
void GLCairoDraw::newSubPath() {
    //
}
/*
 * transforms
 */
void GLCairoDraw::transformVertex(double &x, double &y) {
    x = mMatrix.xx*x + mMatrix.xy*y + mMatrix.x0;
    y = mMatrix.yx*x + mMatrix.yy*y + mMatrix.y0;
}
void GLCairoDraw::identityMatrix() {
    mMatrix.x0 = 0.0;
    mMatrix.y0 = 0.0;
    mMatrix.xx = 1.0;
    mMatrix.xy = 0.0;
    mMatrix.yy = 0.0;
    mMatrix.yx = 0.0;
}
void GLCairoDraw::getMatrix(cairo_matrix_t *matrix) const {
    *matrix = mMatrix;
}
void GLCairoDraw::setMatrix(const cairo_matrix_t *matrix) {
    mMatrix = *matrix;
}
void GLCairoDraw::translate(double dx, double dy) {
    mMatrix.x0 += dx*mMatrix.xx + dy*mMatrix.xy;
    mMatrix.y0 += dx*mMatrix.yx + dy*mMatrix.yy;
}
void GLCairoDraw::scale(double sx, double sy) {
    mMatrix.xx *= sx;
    mMatrix.xy *= sx;
    mMatrix.yx *= sy;
    mMatrix.yy *= sy;
}
void GLCairoDraw::rotate(double radians) {
    double C = ::cos(radians);
    double S = ::sin(radians);

    double xx0 = mMatrix.xx;
    double xy0 = mMatrix.xy;
    double yx0 = mMatrix.yx;
    double yy0 = mMatrix.yy;

    mMatrix.xx = C * xx0 - S * yx0;
    mMatrix.yx = S * xx0 + C * yx0;
    mMatrix.xy = C * xy0 - S * yy0;
    mMatrix.yy = S * xy0 + C * yy0;
}
void GLCairoDraw::setDeviceTransform(double dx, double dy, double sx, double sy)
{
    mDevX  = dx;
    mDevY  = dy;
    mDevSX = sx;
    mDevSY = sy;
}
