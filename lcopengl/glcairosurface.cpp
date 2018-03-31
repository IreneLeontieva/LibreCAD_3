#include "glcairosurface.h"
#include "glcairoutil.h"
#include <assert.h>

CAIRO_ITEM_IMPL(GLCairoSurface)

GLCairoSurface::GLCairoSurface(GLCairoContext *owner)
    : GLCairoItem(owner, type)
    , mStatus(CAIRO_STATUS_INVALID_FORMAT)
    , mWidth(0)
    , mHeight(0)
    , mFormat(CAIRO_FORMAT_INVALID)
    , mContent(CAIRO_CONTENT_COLOR)
    , mPixelSize(1)
    , mProxySurface(nullptr)
    , mTexture(0)
    , mPixelBuffer(0)
    , mPixelCopySync(NULL)
    , mMappedPixelBuffer(nullptr)
{
    mXOffset = 0.0;
    mYOffset = 0.0;
    mXScale  = 1.0;
    mYScale  = 1.0;
    mClipX1  = 0;
    mClipY1  = 0;
    mClipX2  = 0;
    mClipY2  = 0;
    mClipEnabled = false;
}
GLCairoSurface::GLCairoSurface(GLCairoContext *owner,
                               cairo_format_t format,
                               int w,
                               int h,
                               int stride,
                               void * data)
    : GLCairoSurface(owner)
{
    auto gl = getGL();

    if (w <= 0 || h <= 0) {
        mStatus = CAIRO_STATUS_INVALID_SIZE;
        return;
    }
    if (w > gl->MAX_TEXTURE_SIZE || h > gl->MAX_TEXTURE_SIZE) {
        mStatus = CAIRO_STATUS_NO_MEMORY;
        return;
    }    
    mFormat = format;
    switch(format) {
    case CAIRO_FORMAT_A8:
        mContent       = CAIRO_CONTENT_ALPHA;
        mIformat       = GL_R8UI;
        mOformat       = GL_RED;
        mComponentType = GL_UNSIGNED_BYTE;
        mPixelSize     = 1;
        break;
    case CAIRO_FORMAT_RGB16_565:
        mContent       = CAIRO_CONTENT_COLOR;
        mIformat       = GL_RGB565;
        mOformat       = GL_BGR;
        mComponentType = GL_UNSIGNED_SHORT_5_6_5;
        mPixelSize     = 2;
        break;
        break;
    case CAIRO_FORMAT_RGB24:
        mContent       = CAIRO_CONTENT_COLOR;
        mIformat       = GL_RGBA8UI;
        mOformat       = GL_BGRA;
        mComponentType = GL_UNSIGNED_INT_8_8_8_8_REV;
        mPixelSize     = 4;
        break;
    case CAIRO_FORMAT_ARGB32:
        mContent       = CAIRO_CONTENT_COLOR_ALPHA;
        mIformat       = GL_RGBA8UI;
        mOformat       = GL_BGRA;
        mComponentType = GL_UNSIGNED_INT_8_8_8_8_REV;
        mPixelSize     = 4;
        break;
    case CAIRO_FORMAT_RGB30:
        mContent       = CAIRO_CONTENT_COLOR;
        mIformat       = GL_RGB10_A2UI;
        mOformat       = GL_BGRA;
        mComponentType = GL_UNSIGNED_INT_2_10_10_10_REV;
        mPixelSize     = 4;
        break;
    default:
        mStatus = CAIRO_STATUS_INVALID_FORMAT;
        return;
    }
    if (data && (stride < 0 || stride != int(mPixelSize)*w)) {
        mStatus = CAIRO_STATUS_INVALID_STRIDE;
        return;
    }

    gl->clearError();
    GLuint texture = 0;
    GLuint fbo = 0;
    gl->glGenTextures(1, &texture);
    gl->glGenFramebuffers(1, &fbo);
    if (texture & fbo) {
        gl->glBindTexture(GL_TEXTURE_2D, texture);
        gl->glPixelStorei(GL_UNPACK_ALIGNMENT, mPixelSize);
        gl->glPixelStorei(GL_UNPACK_ROW_LENGTH, w*mPixelSize);
        gl->glTexImage2D(GL_TEXTURE_2D,
                         0,
                         mIformat,
                         w,
                         h,
                         0,
                         mOformat,
                         mComponentType,
                         data);
        gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        gl->glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D,
                                   texture, 0);
        if (!data) {
            gl->glClearColor(0,0,0,0);
            gl->glClear(GL_COLOR_BUFFER_BIT);
        }
        gl->glBindTexture(GL_TEXTURE_2D, 0);
        gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    GLenum e = gl->checkError();
    if (e != GL_NO_ERROR) {
        if (texture) gl->glDeleteTextures(1, &texture);
        mStatus = glErrorToStatus(e);
        return;
    }
    mStatus = CAIRO_STATUS_SUCCESS;
    mWidth         = w;
    mHeight        = h;
    //mFormat        = ...;
    //mContent       = ...;
    //mPixelSize     = ...;
    //mIformat       = ...;
    //mOformat       = ...;
    //mComponentType = ...;
    mTexture = texture;
    mFramebuffer = fbo;
    mClipX1  = 0;
    mClipY1  = 0;
    mClipX2  = w;
    mClipY2  = h;
}
GLCairoSurface::GLCairoSurface(GLCairoContext *owner,
                               cairo_format_t format,
                               int w,
                               int h)
    : GLCairoSurface(owner, format, w, h, 0, 0)
{
}
GLCairoSurface::GLCairoSurface(GLCairoSurface *other,
                               int x,
                               int y,
                               int width,
                               int height,
                               bool inmemory)
    : GLCairoSurface(other->owner())
{
    /*
     * FIXME:
     * https://cairographics.org/manual/cairo-cairo-surface-t.html#cairo-surface-create-for-rectangle
     *
     *    The semantics of subsurfaces have not been finalized yet
     *    unless the rectangle is in full device units, is contained
     *    within the extents of the target surface, and the target or
     *    subsurface's device transforms are not changed.
     *
     * we will ignore device transforms, and will create nil surface
     * when requested to create something outside of source surface
     */
    if ((x < 0) || (y < 0) || (width<=0) || (height<=0)) {
        mStatus = CAIRO_STATUS_INVALID_SIZE;
        return;
    }

    unsigned x1 = x;
    unsigned x2 = x + width;
    unsigned y1 = y;
    unsigned y2 = y + width;
    //check for overflows
    if ((x2 <= x1) || (y2 <= y1)) {
        mStatus = CAIRO_STATUS_INVALID_SIZE;
        return;
    }

    for(;;) {
        if (other->mStatus != CAIRO_STATUS_SUCCESS) {
            mStatus = other->mStatus;
            return;
        }
        if (x2 > other->mWidth || y2 > other->mHeight) {
            mStatus = CAIRO_STATUS_INVALID_SIZE;
            return;
        }
        //there may be more proxy surfaces in chain
        if (!other->mProxySurface)
            break;
        x1 += other->mClipX1;
        x2 += other->mClipX1;
        y1 += other->mClipY1;
        y2 += other->mClipY1;
        other = other->mProxySurface;
    }
    //we do not support views of non-textures
    if (!other->mTexture || !other->mFramebuffer) {
        mStatus = CAIRO_STATUS_INVALID_VISUAL;
        return;
    }

    //final sanity check
    assert (x1 < x2);
    assert (y1 < y2);
    assert (x2 <= other->mWidth);
    assert (y2 <= other->mHeight);

    GLuint pbo    = 0;
    GLsync sync   = NULL;
    void  *mapped = nullptr;
    if (inmemory) {
        auto gl = getGL();
        gl->clearError();

        gl->glGenBuffers(1, &pbo);
        if (pbo) {
            unsigned datasize = (x2 - x1)*(y2 - y1)*other->mPixelSize;
            gl->glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
            gl->glBufferData(GL_PIXEL_PACK_BUFFER, datasize, NULL, GL_STREAM_COPY);

            gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, other->mFramebuffer);
            gl->glReadBuffer(GL_COLOR_ATTACHMENT0);
            gl->glReadPixels(x1, y1, x2-x1, y2-y1,
                             other->mOformat, other->mComponentType,
                             NULL);
            gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            sync = gl->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        }

        GLenum e = gl->checkError();
        if (e != GL_NO_ERROR) {
            if (sync) gl->glDeleteSync(sync);
            if (pbo)  gl->glDeleteBuffers(1, &pbo);
            mStatus = glErrorToStatus(e);
            return;
        }
    }

    mStatus        = CAIRO_STATUS_SUCCESS;
    mWidth         = x2 - x1;
    mHeight        = y2 - y1;
    mFormat        = other->mFormat;
    mContent       = other->mContent;
    mPixelSize     = other->mPixelSize;
    mIformat       = other->mIformat;
    mOformat       = other->mOformat;
    mComponentType = other->mComponentType;
    mProxySurface  = other;
    mPixelBuffer   = pbo;
    mMappedPixelBuffer = mapped;
    mPixelCopySync = sync;
    mXOffset       = x1;
    mYOffset       = y1;
    mClipX1        = x1;
    mClipY1        = y1;
    mClipX2        = x2;
    mClipY2        = y2;
    mClipEnabled   = true;
    //ensure target surface won't be destroyed
    other->ref();
}
GLCairoSurface::~GLCairoSurface()
{
    auto gl = getGL();

    if (mFramebuffer)
        gl->glDeleteFramebuffers(1, &mFramebuffer);
    if (mTexture)
        gl->glDeleteTextures(1, &mFramebuffer);
    if (mPixelCopySync) gl->glDeleteSync(mPixelCopySync);
    if (mPixelBuffer) {
        if (mMappedPixelBuffer) {
            gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mPixelBuffer);
            gl->glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
        gl->glDeleteBuffers(1, &mPixelBuffer);
    }
    if (mProxySurface) mProxySurface->unref();
}

