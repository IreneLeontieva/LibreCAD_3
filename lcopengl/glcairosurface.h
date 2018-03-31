#include "glcairoitem.h"
#include <cairo/cairo.h>

class PRIVATE_API GLCairoSurface : public GLCairoItem {
public:
    CAIRO_ITEM
    static int stride_for_width(cairo_format_t format, int w);

    GLCairoSurface(GLCairoContext * owner); //constructs nil surface
    GLCairoSurface(GLCairoContext * owner,
                   cairo_format_t format,
                   int w,
                   int h);
    GLCairoSurface(GLCairoContext * owner,
                   cairo_format_t format,
                   int w,
                   int h,
                   int stride,
                   void * data);
    GLCairoSurface(GLCairoSurface * other,
                   int x,
                   int y,
                   int width,
                   int height,
                   bool inmemory);
    ~GLCairoSurface();

    int width() const { return mWidth; }
    int height() const { return mHeight; }
    int getStride() const { return mWidth*mPixelSize; }
    cairo_format_t getFormat() const { return mFormat; }

    void            cairo_surface_flush();
    unsigned char * get_data();
    void            mark_dirty();
    void            finish();
    void            waitForSurface();
private:
    cairo_status_t      mStatus;        //cached status
    unsigned            mWidth;         //texture width;
    unsigned            mHeight;        //texture height

    cairo_format_t      mFormat;
    cairo_content_t     mContent;
    unsigned            mPixelSize;     //default pixel size
    GLenum              mIformat;       //internal texture format
    GLenum              mOformat;       //external texture format
    GLenum              mComponentType; //componen type

    GLCairoSurface    * mProxySurface;
    GLuint              mTexture;       //texture itself
    GLuint              mFramebuffer;   //associated framebuffer
    GLuint              mPixelBuffer;   //pixel buffer
    GLsync              mPixelCopySync; //sync object for pixel copying
    void               *mMappedPixelBuffer;

    //a surface may be created as a proxy
    double              mXOffset;
    double              mYOffset;
    double              mXScale;
    double              mYScale;

    unsigned            mClipX1;
    unsigned            mClipY1;
    unsigned            mClipX2;
    unsigned            mClipY2;
    bool                mClipEnabled;

    bool                mPendingCopyToTexture = false;
    void performPendingCopy();
};
