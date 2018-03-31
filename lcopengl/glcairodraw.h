#pragma once
#include "common.h"
#include <GL/gl.h>
#include <cairo/cairo.h>

class PUBLIC_API GLCairoContext;
class PUBLIC_API GLCairoSurface;
class PUBLIC_API GLCairoDraw {
public:
    cairo_status_t status() const { return mStatus; }

    /* DRAWING PATHS */
    void newPath();
    void newSubPath();
    void closePath();
    void moveTo(double x, double y, bool relative);
    void lineTo(double x, double y, bool relative);
    /* DRAWING ATTRIBUTES */
    void setLineWidth(double width);
    void setLineCap(cairo_line_cap_t cap);
    void setLineJoin(cairo_line_join_t join);
    void setMiterLimit(double limit);
    void setSourceRGBA(double r, double b, double g, double a);
    void setSourceSurface(GLCairoSurface * surface);
    /* DRAWING PRIMITIVES */
    void strokePath();
    void fillPath();
    /* TRANSFORMS */
    void identityMatrix();
    void getMatrix(cairo_matrix_t * matrix) const;
    void setMatrix(const cairo_matrix_t * matrix);
    void translate(double dx, double dy);
    void scale(double sx, double sy);
    void rotate(double radians);
    //FIXME: implement transform();
    //FIXME: implement cairo_user_to_device ()
    //FIXME: implement cairo_user_to_device_distance ()
    //FIXME: implement cairo_device_to_user ()
    //FIXME: implement cairo_device_to_user_distance ()
    void flush();

private:
    ~GLCairoDraw();

    friend class GLCairo;
    GLCairoDraw(GLCairoDraw   * draw, bool newbuffer);
    void beginPaint();
    void endPaint();//flushes and drops a reference from cairo_t

    friend class GLCairoSurface;
    GLCairoDraw(GLCairoSurface * surface,
                GLuint texture,
                int    viewportx1,
                int    viewporty1,
                int    viewportx2,
                int    viewporty2);
    GLCairoDraw(GLCairoDraw    * draw,
                int    viewportx1,
                int    viewporty1,
                int    viewportx2,
                int    viewporty2);
    void setDeviceTransform(double dx, double dy, double sx, double sy);
    void release(); //release ASAP
    bool finish();  //syncronize and release

    DISABLE_COPY(GLCairoDraw)
    DISABLE_MOVE(GLCairoDraw)

    cairo_status_t mStatus;
    bool           mReferencedFromDraw; //draw was pushed on the stack and is referenced by another draw
    bool           mReferencedFromSurface;
    bool           mReferencedFromCairo;
    GLCairoDraw   *mPrevious;//draw which is located on the stack
    GLCairoDraw   *mRootDraw;//root draw
    GLCairoContext*mContext;
    //viewport
    int            mX1,mY1;
    int            mX2,mY2;
    //current transform
    double         mDevX;
    double         mDevY;
    double         mDevSX;
    double         mDevSY;
    cairo_matrix_t mMatrix;
    void transformVertex(double& x, double& y);
    //per-draw resources
    bool createResources(GLuint inherit_texture,
                         unsigned w, unsigned h);
    void deleteResources();
    bool           mOwnFramebuffer;
    GLuint         mFramebuffer;
    GLuint         mDepthStencil;
    GLuint         mTexture;
    GLuint         mProxyTexture;

};
