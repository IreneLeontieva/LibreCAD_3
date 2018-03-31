#include "glcontext.h"
#include <iostream>
#include <assert.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#include <windows.h>

enum class state { UNKNOWN, FAIL, SUCCESS };
static state w32_was_initialized = state::UNKNOWN;
static HINSTANCE      mInstance = NULL;
static HWND           mWindow  = NULL;
static HDC            mHDC     = NULL;
static HGLRC          mGLRC    = NULL;
static GLContext    * mFuncs   = NULL;
GLContext * GLContext::create()
{
    //we may be running in console mode, and gdk may be uninitialized
    //at the moment.
    if (w32_was_initialized == state::UNKNOWN)
    {
        w32_was_initialized = state::FAIL;
        /*
         * I used this tutorial
         * http://nehe.gamedev.net/tutorial/creating_an_opengl_window_(win32)/13001/
         */
        assert(mWindow == 0);

        if (!mInstance) {
            WNDCLASS wc;
            wc.style        = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Redraw On Move, And Own DC For Window
            wc.lpfnWndProc  = (WNDPROC) DefWindowProc;     // WndProc Handles Messages
            wc.cbClsExtra   = 0;                           // No Extra Window Data
            wc.cbWndExtra   = 0;                           // No Extra Window Data
            wc.hInstance    = GetModuleInstance(NULL);     // Set The Instance
            wc.hIcon        = LoadIcon(NULL, IDI_WINLOGO); // Load The Default Icon
            wc.hCursor      = LoadCursor(NULL, IDC_ARROW); // Load The Arrow Pointer
            wc.hbrBackground= NULL;                        // No Background Required For GL
            wc.lpszMenuName = NULL;                        // We Don't Want A Menu
            wc.lpszClassName= "__OpenGL__";                // Set The Class Name
            if (!RegisterClass(&wc)) {
                std::cerr<<"Failed to register window class"<<std::endl;
                return nullptr;
            }
            mInstance = wc.hInstance;
        }
        mWindow = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, // Extended Style For The Window
                                 "__OpenGL__",             // Class Name
                                 "__OpenGL__",             // Window Title
                                 WS_CLIPSIBLINGS |         // Required Window Style
                                 WS_CLIPCHILDREN |         // Required Window Style
                                 WS_OVERLAPPEDWINDOW,      // Selected Window Style
                                 0, 0,                     // Window Position
                                 64, 64,                   // Window Size,
                                 NULL,                     // No Parent Window
                                 NULL,                     // No Menu
                                 mInstance,                // Instance
                                 NULL);                    // No parameters
        if (mWindow) {
            HDC dc = GetDC(mWindow);
            if (dc) {
                for(auto i = 0u; gl_versions[i]; ++i)
                {
                    GLint attribs[] = {
                        WGL_CONTEXT_MAJOR_VERSION_ARB, gl_versions[i]/10,
                        WGL_CONTEXT_MINOR_VERSION_ARB, gl_versions[i]%10,
                        WGL_CONTEXT_PROFILE_MASK_ARB, gl_versions[i] < 30 ?
                            WGL_CONTEXT_ES2_PROFILE_BIT_EXT :
                            WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                        0
                    };
                    HGLRC glrc = wglCreateContextAttribsARB(dc, NULL, attribs);
                    if (!glrc)
                        continue;

                    if (wglMakeCurrent(dc, glrc))
                    {
                        GLContext::beginJourney(gl_versions[i]);
                        GLContext * f = new GLContext();
                        wglMakeCurrent(dc, NULL);
                        if (f->completeJourney() >= gl_versions[i])
                        {
                            mHDC  = dc;
                            mGLRC = glrc;
                            w32_was_initialized = state::SUCCESS;
                            break;
                        }
                        delete f;
                    }
                    wglDestroyContext(glrc);
                }
            }
            if (w32_was_initialized != state::SUCCESS) {
                DestroyWindow(mWindow);
                mHDC    = NULL;
                mWindow = NULL;
            }
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
    if (mGLRC != NULL) {
        wglMakeCurrent(mHDC, NULL);
        wglDestroyContext(mGLRC);
        mGLRC = NULL;
    }
    if (mWindow)
        DestroyWindow(mWindow);
    mHDC    = NULL;
    mWindow = NULL;
    w32_was_initialized = state::UNKNOWN;
}
bool   GLContext::makeCurrent() {
    assert(mFuncs == this);
    assert(mWindow != NULL);
    assert(mHDC != NULL);
    assert(mGLRC != NULL);
    return (bool)wglMakeCurrent(mHDC, mGLRC);
}
void GLContext::release() {
    assert(mFuncs == this);
    assert(mWindow != NULL);
    assert(mHDC != NULL);
    assert(mGLRC != NULL);
    glXMakeCurrent(mHDC, NULL);
}
void * GLContext::getProcAddress(const char *name) {
    assert(mWindow  != NULL);
    assert(mContext != NULL);
    assert(mFuncs   != NULL);
    assert(wglGetCurrentContext() == mContext);

    return (void*) wglGetProcAddress(name);
}

