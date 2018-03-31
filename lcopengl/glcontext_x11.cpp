#include <assert.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <iostream>
#include "glcontext.h"

enum class state { UNKNOWN, FAIL, SUCCESS };
static state x11_was_initialized = state::UNKNOWN;
static Display           * x11Display   = NULL;
static Window              x11Window    = None;
static GLXContext          x11Context   = NULL;
static GLContext         * mFuncs    = nullptr;
GLContext * GLContext::create()
{
    //prevent double-creation
    assert(mFuncs == nullptr);
    //we may be running in console mode, and gdk may be uninitialized
    //at the moment.
    if (x11_was_initialized == state::UNKNOWN)
    {
        x11_was_initialized = state::FAIL;

        static const GLubyte *procname = (const GLubyte*) "glXCreateContextAttribsARB";
        PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribs =
              (PFNGLXCREATECONTEXTATTRIBSARBPROC) glXGetProcAddress(procname);
        if (!glXCreateContextAttribs) {
            WARNING("wont work without glXCreateContextAttribs");
            return nullptr;
        }

        if (!x11Display)
            x11Display = XOpenDisplay(NULL);
        if (x11Display) {
            //choose visual
            //we don't need double buffering on an offscreen window
            //https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glXIntro.xml
            static const int singleBufferAttributess[] = {
                GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
                GLX_RENDER_TYPE,   GLX_RGBA_BIT,
                GLX_RED_SIZE,      1,   /* Request a single buffered color buffer */
                GLX_GREEN_SIZE,    1,   /* with the maximum number of color bits  */
                GLX_BLUE_SIZE,     1,   /* for each component                     */
                None
            };
            int numReturned;
            GLXFBConfig *fbConfigs = glXChooseFBConfig(
                        x11Display,
                        DefaultScreen(x11Display),
                        singleBufferAttributess,
                        &numReturned );
            if (fbConfigs) {
                XVisualInfo * visualinfo = glXGetVisualFromFBConfig(x11Display, fbConfigs[0] );
                if (visualinfo) {
                    x11Window = XCreateWindow(x11Display,
                                              RootWindow(x11Display, visualinfo->screen),
                                              0, 0, 256, 256,
                                              0, visualinfo->depth, InputOutput,
                                              visualinfo->visual,
                                              0, NULL );
                    if (x11Window) {
                        XMapWindow(x11Display, x11Window);
                        //try to create a context
                        for(auto i = 0u; gl_versions[i]; ++i)
                        {
                            //now we are using
                            //OpenGL SuperBible: Comprehensive Tutorial and Reference, page 697

                            //motherfuckers, you really think you are smart?
                            //what a hell have you designed instead of API...

                            GLint attribs[] = {
                                GLX_CONTEXT_MAJOR_VERSION_ARB, gl_versions[i]/10,
                                GLX_CONTEXT_MINOR_VERSION_ARB, gl_versions[i]%10,
                                GLX_CONTEXT_PROFILE_MASK_ARB, gl_versions[i] < 30 ?
                                    GLX_CONTEXT_ES2_PROFILE_BIT_EXT :
                                    GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                                0
                            };
                            x11Context = glXCreateContextAttribs(x11Display, fbConfigs[0], NULL, True, attribs);
                            if (!x11Context)
                                continue;

                            if (True ==glXMakeCurrent(x11Display, x11Window, x11Context))
                            {
                                GLContext::beginJourney(gl_versions[i]);
                                GLContext * f = new GLContext();
                                glXMakeCurrent(x11Display, None, NULL);
                                if (f->completeJourney() >= gl_versions[i])
                                {
                                    x11_was_initialized = state::SUCCESS;
                                    break;
                                }
                                delete f;
                            }
                            glXDestroyContext(x11Display, x11Context);
                            x11Context = NULL;
                        }
                        XUnmapWindow(x11Display, x11Window);
                        if (x11_was_initialized != state::SUCCESS) {
                            XDestroyWindow(x11Display, x11Window);
                            x11Window = None;
                        }
                    }
                    XFree(visualinfo);
                }
                XFree(fbConfigs);
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

    assert(x11Display != NULL);
    glXMakeCurrent(x11Display, None, NULL);
    if (x11Context != NULL)
        glXDestroyContext(x11Display, x11Context);
    x11Context = NULL;
    if (x11Window != None)
        XDestroyWindow(x11Display, x11Window);
    x11Window = None;
    x11_was_initialized = state::UNKNOWN;
}
bool   GLContext::makeCurrent() {
    assert(mFuncs == this);
    assert(x11Display != NULL);
    assert(x11Window != None);
    assert(x11Context != NULL);
    return (True == glXMakeCurrent(x11Display, x11Window, x11Context));
}
void GLContext::release() {
    assert(mFuncs == this);
    assert(x11Display != NULL);
    assert(x11Window != None);
    assert(x11Context != NULL);
    glXMakeCurrent(x11Display, None, NULL);
}
void * GLContext::getProcAddress(const char *name) {
    assert(mFuncs == this);
    assert(x11Display != NULL);
    assert(x11Window != None);
    assert(x11Context != NULL);
    assert(glXGetCurrentContext() == x11Context);

    return (void*) glXGetProcAddress((const GLubyte*)name);
}

