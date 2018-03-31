#include <assert.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <iostream>
#include "glcontext.h"

enum class state { UNKNOWN, FAIL, SUCCESS };
static state qt_was_initialized = state::UNKNOWN;
static QOffscreenSurface * mSurface = nullptr;
static QOpenGLContext    * mContext = nullptr;
static GLContext         * mFuncs    = nullptr;

GLContext::GLContext(GLContext && source) {
    if (&source == mFuncs)
        mFuncs = this;
}
GLContext& GLContext::operator =(GLContext&& source) {
    if (&source == mFuncs)
        mFuncs = this;
    return *this;
}
GLContext::GLContext()
{
    assert(mFuncs = nullptr);
    mFuncs = this;
}
GLContext::~GLContext()
{
    if (mFuncs == this) {
        mFuncs = NULL;

        if (mContext) mContext->doneCurrent();
        delete mContext;
        mContext = nullptr;
        delete mSurface;
        mSurface = nullptr;

        qt_was_initialized = state::UNKNOWN;
    }
}
GLContext * GLContext::create()
{
    //we may be running in console mode, and gdk may be uninitialized
    //at the moment.
    if (qt_was_initialized == state::UNKNOWN)
    {
        qt_was_initialized = state::FAIL;

        //try to create a context
        for(auto i = 0u; gl_versions[i]; ++i)
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
            mContext->setShareContext(QOpenGLContext::globalShareContext());
#endif
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
            GLContext::beginJourney(gl_versions[i]);
            GLContext * f = new GLContext();
            mContext->doneCurrent();

            if (f->completeJourney() != gl_versions[i]) {
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

bool   GLContext::makeCurrent() {
    assert(mFuncs == this);
    assert(mContext != nullptr);
    return mContext->makeCurrent(mSurface);
}
void GLContext::release() {
    assert(mFuncs == this);
    assert(mContext != nullptr);
    mContext->doneCurrent();
}
void * GLContext::getProcAddress(const char *name) {
    assert(mFuncs == this);
    assert(mContext != nullptr);
    assert(QOpenGLContext::currentContext() == mContext);

    return (void*) mContext->getProcAddress(QByteArray::fromRawData(name, strlen(name)+1));
}
