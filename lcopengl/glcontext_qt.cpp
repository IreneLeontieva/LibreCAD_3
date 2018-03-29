#include "glcontext.h"
#include <assert.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <QOffscreenSurface>
#include <QOpenGLContext>
enum class state { UNKNOWN, FAIL, SUCCESS };
static state qt_was_initialized = state::UNKNOWN;
static QOffscreenSurface * mSurface = nullptr;
static QOpenGLContext    * mContext = nullptr;
static GLContext         * Funcs    = nullptr;
static const unsigned gl_versions[] = {45, 44, 43, 42, 40, 33, 30, 21, 20};
static unsigned       gl_required_version = 0;
static bool           gl_extension_enabled = true;
GLContext * GLContext::create()
{
    //we may be running in console mode, and gdk may be uninitialized
    //at the moment.
    if (qt_was_initialized == state::UNKNOWN)
    {
        qt_was_initialized = state::FAIL;

        //try to create a context
        auto version_count = sizeof(gl_versions)/sizeof(gl_versions[0]);
        for(auto i = versions_count; i; --i)
        {
            mSurface = new QOffscreenSurface();
            QSurfaceFormat format;
            format.setAlphaBufferSize(8);
            format.setProfile(QSurfaceFormat::CoreProfile);
            format.setVersion(gl_versions[i]/10, gl_versions[i]%10);
            format.setRenderableType(gl_versions[i] < 30 ? QSurfaceFormat::OpenGLES : QSurfaceFormat::OpenGL);
            mSurface->setFormat(format);
            mSurface->create();
            if (mSurface->isValid()) {
                QSurfaceFormat newformat = mSurface->format();
                if (newformat.majorVersion() < format.majorVersion() ||
                    newformat.minorVersion() < format.minorVersion())
                delete mSurface;
                mSurface = nullptr;
                continue;
            }
            mContext = new QOpenGLContext();
            mContext->setFormat(format);
            if (!mContext->create() ||
                !mContext->makeCurrent(mSurface))
            {
                delete mContext;
                mContext = nullptr;
                delete mSurface;
                mSurface = nullptr;
                continue;
            }
            //try to bind all functions
            gl_required_version = gl_versions[i];
            GLContext * f = new GLContext();
            mContext->doneCurrent();

            if (gl_required_version != gl_versions[i]) {
                delete mContext;
                mContext = nullptr;
                delete mSurface;
                mSurface = nullptr;
                continue;
            }

            assert(mFuncs == f);
            qt_was_initialized = state::SUCCESS;
            break;
        }
        //safety belts - if something gone wrong, clean up stuff
        if (qt_was_initialized != state::SUCCESS) {
            delete mContext;
            mContext = nullptr;
            delete mSurface;
            mSurface = nullptr;
        }
    }
    return mFuncs;
}
GLContext::GLContext()
{
    assert(mFuncs = nullptr);
    mFuncs = this;
}
GLContext::~GLContext()
{
    assert(mFuncs == this);
    mFuncs = NULL;

    if (mContext) mContext->doneCurrent();
    delete mContext;
    mContext = nullptr;
    delete mSurface;
    mSurface = nullptr;

    qt_was_initialized = state::UNKNOWN;
}
bool   GLContext::fakeExtention(const char *ext, int version, bool cc) {
    bool found = cc & (version <= gl_required_version);
    gl_extension_enabled = found;
    return found;
}
bool   GLContext::checkExtention(const char * ext, int version, bool cc) {
    bool found = false;
    //don't even try to resolve if not needed
    if (cc && (version <= gl_required_version)) {
        //known prefixes we will try to substitute
        static const char * prefixes[] = {
            "GL_ARB",
            "GL_ARB_ES2",
            "GL_ARB_ES3",
            "GL_OES",
            "GL_EXT",
            "GL_NV",
            "GL_ATI",
            NULL
        };

        char * extbuf = alloca(20+strlen(ext));

        for(int j = 0; prefixes[j]; ++j)
        {
            strcpy(extbuf, prefixes[j]);
            strcat(extbuf, "_");
            strcat(extbuf, ext);

            found = mContext->hasExtension(QByteArray::fromRawData(extbuf, strlen(extbuf)));
            if (found) break;
        }
    }
    gl_extension_enabled = found;
    return found;
}
void * GLContext::import(const char *name) {
    assert(mSurface != NULL);
    assert(mContext != NULL);
    assert(mFuncs   != NULL);
    assert(QOpenGLContext::currentContext() == mContext);

    //we are already broken
    if (!gl_required_version) return NULL;
    //if extension is not
    if (!gl_extension_enabled) return NULL;

    void * ret = mContext->getProcAddress(name);
    if (!ret) {
        static const char * suffixes[] = {"ARB","EXT","OES","AMD","NV",NULL};
        auto len = strlen(name);
        char * namebuffer = alloca(len+10);
        memcpy(namebuffer, name, len);
        for(int i = 0; suffixes[i]; ++i) {
            strcpy(namebuffer+len, suffixes[i]);
            ret = mContext->getProcAddress(namebuffer);
            if (ret) break;
        }
    }
    //if required function was not found, mark context as broken
    if (!ret) gl_required_version = 0;
    return ret;
}
