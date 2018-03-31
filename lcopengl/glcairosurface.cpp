#include "glcairosurface.h"
#include "glcairoutil.h"
#include "glcairodraw.h"
#include <assert.h>
#include <limits>

CAIRO_ITEM_IMPL(GLCairoSurface)

int GLCairoSurface::stride_for_width(cairo_format_t format, int w) {
    int ret;
    switch(format) {
    case CAIRO_FORMAT_A8:
        ret = w;
        break;
    case CAIRO_FORMAT_RGB16_565:
        ret = 2*w;
        break;
    case CAIRO_FORMAT_RGB24:
    case CAIRO_FORMAT_ARGB32:
    case CAIRO_FORMAT_RGB30:
        ret = 4*w;
        break;
    default:
        ret = 0;
    }
    return ret;
}
GLCairoSurface::GLCairoSurface(GLCairoContext *owner)
    : GLCairoItem(owner, type)
    , mStatus(CAIRO_STATUS_INVALID_FORMAT)
    , mWidth(0)
    , mHeight(0)
    , mFormat(CAIRO_FORMAT_INVALID)
    , mContent(CAIRO_CONTENT_COLOR)
    , mPixelSize(1)
    , mProxySurface(nullptr)
    , mFinalSurface(nullptr)
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

    mPendingCopyToTexture = false;
    mPendingDraw          = nullptr;
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

    //we limit texture size to avoid overflows later
    if (w <= 0 || h <= 0 || w > 16384 || h > 16384) {
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

    //there may be more proxy surfaces in chain
    GLCairoSurface * ps = other;
    for(;;) {
        if (ps->mStatus != CAIRO_STATUS_SUCCESS) {
            mStatus = ps->mStatus;
            return;
        }
        if (x2 > ps->mWidth || y2 > ps->mHeight) {
            mStatus = CAIRO_STATUS_INVALID_SIZE;
            return;
        }
        if (!ps->mProxySurface)
            break;
        x1 += ps->mClipX1;
        x2 += ps->mClipX1;
        y1 += ps->mClipY1;
        y2 += ps->mClipY1;
        ps  = ps->mProxySurface;
    }
    //we do not support views of non-textures
    if (!ps->mTexture || !ps->mFramebuffer) {
        mStatus = CAIRO_STATUS_INVALID_VISUAL;
        return;
    }

    //final sanity check
    if (x1 < x2 || y1 < y2 || x2 <= ps->mWidth || y2 <= ps->mHeight) {
        mStatus = CAIRO_STATUS_DEVICE_ERROR;
        return;
    }

    GLuint pbo    = 0;
    GLsync sync   = NULL;
    void  *mapped = nullptr;
    if (inmemory) {
        auto gl = getGL();
        gl->clearError();

        gl->glGenBuffers(1, &pbo);
        if (pbo) {
            unsigned datasize = (x2 - x1)*(y2 - y1)*ps->mPixelSize;
            gl->glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
            if (gl->buffer_storage)
                gl->glBufferStorage(GL_PIXEL_PACK_BUFFER,
                                    datasize,
                                    NULL,
                                    GL_MAP_READ_BIT|
                                    GL_MAP_WRITE_BIT|
                                    GL_MAP_PERSISTENT_BIT|
                                    GL_CLIENT_STORAGE_BIT);
            else
                gl->glBufferData(GL_PIXEL_PACK_BUFFER, datasize, NULL, GL_STREAM_COPY);
            gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, ps->mFramebuffer);
            gl->glReadBuffer(GL_COLOR_ATTACHMENT0);
            gl->glPixelStorei(GL_PACK_ALIGNMENT, mPixelSize);
            gl->glPixelStorei(GL_PACK_ROW_LENGTH, mWidth*mPixelSize);
            gl->glReadPixels(x1, y1, x2-x1, y2-y1,
                             ps->mOformat, ps->mComponentType,
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
    mFormat        = ps->mFormat;
    mContent       = ps->mContent;
    mPixelSize     = ps->mPixelSize;
    mIformat       = ps->mIformat;
    mOformat       = ps->mOformat;
    mComponentType = ps->mComponentType;
    mProxySurface  = other;
    mFinalSurface  = ps;
    mPixelBuffer   = pbo;
    mMappedPixelBuffer = mapped;
    mPixelCopySync = sync;
    mXOffset       = x1 + other->mXOffset;
    mYOffset       = y1 + other->mYOffset;
    mXScale        = other->mXScale;
    mYScale        = other->mYScale;
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
    finish();
}
void GLCairoSurface::finish() {
    if (mStatus == CAIRO_STATUS_SURFACE_FINISHED)
        return;
    auto gl = getGL();

    waitForSurface(true);
    if (mPixelBuffer) {
        if (mMappedPixelBuffer) {
            gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mPixelBuffer);
            gl->glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            mMappedPixelBuffer = NULL;
        }
        gl->glDeleteBuffers(1, &mPixelBuffer);
        mPixelBuffer = 0;
    }
    if (mPendingDraw) {
        mPendingDraw->release();
        mPendingDraw = nullptr;
    }
    if (mFramebuffer) {
        gl->glDeleteFramebuffers(1, &mFramebuffer);
        mFramebuffer = 0;
    }
    if (mTexture) {
        gl->glDeleteTextures(1, &mTexture);
        mTexture = 0;
    }
    if (mProxySurface) {
        mProxySurface->unref();
        mProxySurface = nullptr;
    }
    mStatus = CAIRO_STATUS_SURFACE_FINISHED;
}
void GLCairoSurface::setDeviceOffset(double x, double y) 
{
    mXOffset = x;
    mYOffset = y;
}
void GLCairoSurface::setDeviceScale(double x, double y)
{
    mXScale = x;
    mYScale = y;
}
void GLCairoSurface::getDeviceOffset(double *x, double *y) 
{
    *x = mXOffset;
    *y = mYOffset;
}
void GLCairoSurface::getDeviceScale(double *x, double *y)
{
    *x = mXScale;
    *y = mYScale;
}
cairo_status_t GLCairoSurface::createDraw(GLCairoDraw **pDraw) {
    if (mStatus != CAIRO_STATUS_SUCCESS)
        return mStatus;
    if (mPendingDraw)
        return CAIRO_STATUS_DEVICE_ERROR;

    GLCairoSurface * texsrc = this;
    while (texsrc->mProxySurface) {
        texsrc = texsrc->mProxySurface;
        if (texsrc->mPendingDraw) {
            mPendingDraw = new GLCairoDraw(texsrc->mPendingDraw,
                                           mClipX1,
                                           mClipY1,
                                           mClipX2,
                                           mClipY2);
            break;
        }
    }
    if (!mPendingDraw) {
        assert(texsrc == mFinalSurface || texsrc == this);
        if (!texsrc->mTexture || !texsrc->mFramebuffer)
            return CAIRO_STATUS_DEVICE_ERROR;

        texsrc->mPendingDraw = new GLCairoDraw(this,
                                               texsrc->mTexture,
                                               mClipX1,
                                               mClipY1,
                                               mClipX2,
                                               mClipY2);

        if (texsrc != this) {
            mPendingDraw = new GLCairoDraw(texsrc->mPendingDraw,
                                           mClipX1,
                                           mClipY1,
                                           mClipX2,
                                           mClipY2);
        }
    }
    mPendingDraw->setDeviceTransform(mXOffset,
                                     mYOffset,
                                     mXScale,
                                     mYScale);
    *pDraw = mPendingDraw;
    return mPendingDraw->status();
}
void GLCairoSurface::flush() {
    if (mPendingDraw) {
        mPendingDraw->flush();
    }
}
GLuint GLCairoSurface::getTexture() {
    if (!mTexture)
        return 0;

    //wait for pending io operations
    waitForSurface(true);

    //wait for pending draw call
    //note: it is possible to call getTexture while
    //drawing is still done. results of such operations
    //are undefined: we will try to not to crash,
    //but that's all you can expect
    if (mPendingDraw) {
        if (!mPendingDraw->finish()) return 0;
        mPendingDraw = nullptr;
    }
    return mTexture;
}
GLuint GLCairoSurface::getTextureBox(int &x1,
                                     int &y1,
                                     int &x2,
                                     int &y2)
{
    GLuint tex = mTexture;
    if (!tex && mFinalSurface)
        tex = mFinalSurface->getTexture();
    if (tex) {
        x1 = mClipX1;
        y1 = mClipY1;
        x2 = mClipX2;
        y2 = mClipY2;
    }
    return tex;
}
void GLCairoSurface::waitForSurface(bool full) {
    auto gl = getGL();
    if (mPixelCopySync) {
        GLenum e = gl->glClientWaitSync(mPixelCopySync,
                                        GL_SYNC_FLUSH_COMMANDS_BIT,
                                        std::numeric_limits<GLuint64>::max()/2);
        if (e != GL_ALREADY_SIGNALED && e != GL_CONDITION_SATISFIED)
            gl->glFinish();
        gl->glDeleteSync(mPixelCopySync);
        mPixelCopySync = NULL;
    }
    if (mPendingCopyToTexture) {
        GLuint texture = mTexture;
        if (!texture) {
            assert(mFinalSurface);
            assert(mFinalSurface->mTexture);
            texture = mFinalSurface->mTexture;
        }
        gl->glPixelStorei(GL_PACK_ALIGNMENT, mPixelSize);
        gl->glPixelStorei(GL_PACK_ROW_LENGTH, mWidth*mPixelSize);
        gl->glBindTexture(GL_TEXTURE_2D, texture);
        gl->glTexSubImage2D(GL_TEXTURE_2D, 0,
                            mClipX1, mClipY1,
                            mClipX2 - mClipX1, mClipY2 - mClipY1,
                            mOformat, mComponentType,
                            NULL);
        gl->glBindTexture(GL_TEXTURE_2D, 0);

        mPendingCopyToTexture = false;
        if (!full)
            mPixelCopySync = gl->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        else
            gl->glFinish();
    }
}
unsigned char * GLCairoSurface::get_data()
{
    if (mStatus != CAIRO_STATUS_SUCCESS)
        return nullptr;

    //if there was a pending pixel copy, wait for the surface
    waitForSurface(true);
    if (mPendingDraw) {
        mPendingDraw->finish();
        mPendingDraw = nullptr;
    }

    auto gl = getGL();
    gl->clearError();

    unsigned datasize = mWidth*mHeight*mPixelSize;
    if (!mPixelBuffer)
    {
        if (!mTexture || !mFramebuffer)
            return nullptr;

        GLuint pbo = 0;
        gl->glGenBuffers(1, &pbo);
        if (pbo) {
            gl->glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
            if (gl->buffer_storage)
                gl->glBufferStorage(GL_PIXEL_PACK_BUFFER,
                                    datasize,
                                    NULL,
                                    GL_MAP_READ_BIT|
                                    GL_MAP_WRITE_BIT|
                                    GL_MAP_PERSISTENT_BIT|
                                    GL_CLIENT_STORAGE_BIT);
            else
                gl->glBufferData(GL_PIXEL_PACK_BUFFER, datasize, NULL, GL_STREAM_COPY);

            gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer);
            gl->glReadBuffer(GL_COLOR_ATTACHMENT0);
            gl->glPixelStorei(GL_PACK_ALIGNMENT, mPixelSize);
            gl->glPixelStorei(GL_PACK_ROW_LENGTH, mWidth*mPixelSize);
            gl->glReadPixels(0, 0, mWidth, mHeight,
                             mOformat, mComponentType,
                             NULL);
            gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            gl->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

            if (gl->checkError()) {
                gl->glDeleteBuffers(1, &pbo);
                return nullptr;
            }
        }
        mPixelBuffer = pbo;
    }

    waitForSurface(false);
    if (gl->checkError()) {
        return nullptr;
    }

    if (!mMappedPixelBuffer) {
        assert(mPixelBuffer);
        gl->glBindBuffer(GL_PIXEL_PACK_BUFFER, mPixelBuffer);
        if (gl->buffer_storage) {
            mMappedPixelBuffer = gl->glMapBufferRange(GL_PIXEL_PACK_BUFFER,
                                                      0, datasize,
                                                      GL_MAP_READ_BIT|
                                                      GL_MAP_WRITE_BIT|
                                                      GL_MAP_PERSISTENT_BIT|
                                                      //GL_MAP_UNSYNCHRONIZED_BIT|
                                                      GL_MAP_FLUSH_EXPLICIT_BIT);
        } else {
            mMappedPixelBuffer = gl->glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_WRITE);
        }
        gl->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }
    if (gl->checkError()) {
        return nullptr;
    }
    return (unsigned char*)mMappedPixelBuffer;
}
void GLCairoSurface::mark_dirty()
{
    auto gl = getGL();
    gl->clearError();

    unsigned datasize = mWidth*mHeight*mPixelSize;

    //wait until previous operation will complete if any
    waitForSurface(false);

    if (mMappedPixelBuffer) {
        gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mPixelBuffer);
        if (gl->buffer_storage) {
            gl->glFlushMappedBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, datasize);
            mPixelCopySync = gl->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            mPendingCopyToTexture = true;
        } else {
            gl->glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            mMappedPixelBuffer = nullptr;
            mPendingCopyToTexture = true;
            waitForSurface(true);
        }
        gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }
}