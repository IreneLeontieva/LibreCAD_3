#include "glcontext.h"
#include <iostream>
#include <assert.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#include <gdk/gdk.h>
#include <gdk/gdkglcontext.h>

//resolving symbols is HELL
#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
static void * portableProcAddress(const char * name) {
    return wglGetProcAddress((LPCSTR)(name));
}
#else
#include <dlfcn.h>
#include <GL/glx.h>
#include <gdk/gdkwayland.h>
static void * portableProcAddress(const char * name) {
#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_DISPLAY (display))
    {
        typedef void* (*eglGetProcAddress_t)(char const * procname);
        static eglGetProcAddress_t getProcAddress = (eglGetProcAddress_t)dlsym(RTLD_DEFAULT, "eglGetProcAddress");
        return (getProcAddress ? getProcAddress(name) : NULL);
    }
#endif

#ifdef GDK_WINDOWING_X11
    if (GDK_IS_X11_DISPLAY (display))
    {
        typedef void* (*glxGetProcAddress_t)(char const * procname);
        static glxGetProcAddress_t getProcAddress = (glxGetProcAddress_t)dlsym(RTLD_DEFAULT, "glXGetProcAddressARB");
        return (getProcAddress ? getProcAddress(name) : NULL);
    }
#endif

#ifdef GDK_WINDOWING_QUARTZ
    return dlsym(RTLD_DEFAULT, name);
#else
    return NULL;
#endif

#ifdef GDK_WINDOWING_WIN32
#error "GDK_WINDOWING_WIN32 defined in non windows build"
#endif
}
#endif


enum class state { UNKNOWN, FAIL, SUCCESS };
static state gdk_was_initialized = state::UNKNOWN;
static GdkWindow    * mWindow  = NULL;
static GdkGLContext * mContext = NULL;
static GLContext    * mFuncs   = NULL;
static const unsigned gl_versions[] = {45, 44, 43, 42, 40, 33, 30, 21, 20};
static unsigned       gl_required_version = 0;
static bool           gl_extension_enabled = true;
GLContext * GLContext::create()
{
    //we may be running in console mode, and gdk may be uninitialized
    //at the moment.
    if (gdk_was_initialized == state::UNKNOWN)
    {
        gdk_was_initialized = state::FAIL;
        int    fakeargc   = 1;
        char * fakeargv[] = {"appname", NULL};
        do {
            assert(mWindow  == NULL);
            assert(mContext == NULL);
            assert(mFuncs   == NULL);


            //gdk_init_check calls gdk_parse_args
            //https://github.com/nobled/gtk/blob/master/gdk/gdk.c#L366

            //gdk_parse_args return without touching arguments if already initialized
            //https://github.com/nobled/gtk/blob/master/gdk/gdk.c#L275

            //so it's safe to call it once again
            if (!gdk_init_check (&fakeargc, &fakeargv))
                break;

            GdkDisplay * display = gdk_display_get_default();
            if (!display) break;
            GdkScreen * screen = gdk_display_get_default_screen(display);
            if (!screen) break;
            GdkVisual * visual = gdk_screen_get_rgba_visual(screen);
            if (!visual) break;

            //try to create an invisible window
            //I wonder why clowns who designed OpenGL do not provide
            //API to create OpenGL without windows. Linus Torvalds
            //should show them a finger to make them fix this.
            GdkWindowAttr attributes;
            memset(attributes, 0, sizeof(attributes));
            attributes.title      = "";
            attributes.width      = 64;
            attributes.height     = 64;
            attributes.visual     = visual;
            attributes.window_type= GDK_WINDOW_TOPLEVEL;
            attributes.override_redirect = TRUE;
            mWindow = gdk_window_new(NULL,
                                     &attributes,
                                     GDK_WA_NOREDIR|
                                     GDK_WA_VISUAL|
                                     GDK_WA_TYPE_HINT);
            if (!mWindow)
                break;

            gdk_window_hide(mWindow);

            //try to create a context
            auto version_count = sizeof(gl_versions)/sizeof(gl_versions[0]);

            GLContext * result = nullptr;
            GError      * error  = NULL;
            for(auto i = versions_count; i; --i)
            {
                gdk_gl_context_set_required_version(mContext, gl_versions[i]/10, gl_versions[i]%10);
                gdk_gl_context_set_forward_compatible(mContext, FALSE);
                mContext = gdk_window_create_gl_context(mWindow, &error);
                if (mContext && gdk_gl_context_realize(mContext, &error)) {
                    assert(error == NULL);

                    //try to bind all functions
                    gl_required_version  = gl_versions[i];
                    gl_extension_enabled = true;
                    gdk_gl_context_make_current(mContext);
                    GLContext * f = new GLContext();
                    gdk_gl_context_clear_current();

                    if (gl_required_version == gl_versions[i]) {
                        assert(mFuncs == f);
                        gdk_was_initialized = state::SUCCESS;
                        break;
                    }
                    //we failed to bind something, try another version
                    g_object_unref(mContext);
                    mContext = NULL;
                    continue;
                }
                //FIXME: log error?
                g_clear_error(&error);
                error = NULL;
            }
        } while(0);
        //safety belts - if something gone wrong, clean up stuff
        if (gdk_was_initialized != state::SUCCESS) {
            if (mFuncs) {
                delete mFuncs;
                mFuncs   = NULL;
            }
            if (mContext) {
                g_object_unref(mContext);
                mContext = NULL;
            }
            if (mWindow) {
                gdk_window_destroy(mWindow);
                mWindow  = NULL;
            }
            std::cerr<<"Failed to create OpenGL context"<<std::endl;
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
    if (mContext) {
        g_object_unref(mContext);
        mContext = NULL;
    }
    if (mWindow) {
        gdk_window_destroy(mWindow);
        mWindow  = NULL;
    }
    gdk_was_initialized = state::UNKNOWN;
}
bool   GLContext::fakeExtension(const char *ext, int version, bool cc) {
    bool found = cc & (version <= gl_required_version);
    gl_extension_enabled = found;
    return found;
}
bool   GLContext::checkExtension(const char * ext, int version, bool cc) {
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
        //glGetString(GL_EXTENSIONS) is deprecated
        //https://github.com/kbranigan/Simple-OpenGL-Image-Library/issues/8
        //we will try it and if it wont work, we will try glgetstring

        if (glGetStringi) {
            for(int i = 0; i < 10000; ++i) {
                const char * extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
                if (!extension) break;

                for(int j = 0; prefixes[j]; ++j)
                {
                    auto len = strlen(prefixes[j]);
                    if (strncmp(prefixes[j], extension, len))
                        continue;
                    if (extension[len] != '_')
                        continue;
                    if (strcmp(extension+len+1, ext))
                        continue;

                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            const char * extensions = (const char *)glGetString(GL_EXTENSIONS);
            while (extensions && extensions[0]) {
                const char * next = strchr(extensions, ' ');
                if (!next) next = extensions + strlen(extensions);

                for(int j = 0; prefixes[j]; ++j)
                {
                    auto len = strlen(prefixes[j]);
                    if (strncmp(prefixes[j], extensions, len))
                        continue;
                    if (extensions[len] != '_')
                        continue;
                    if (next - extensions <= len + 2) //at least one more symbol must exist after prefix
                        continue;
                    if (strncmp(extensions+len+1, ext, next-extensions-len-1))
                        continue;

                    found = true;
                    break;
                }
            }
        }
    }
    gl_extension_enabled = found;
    return found;
}
void * GLContext::import(const char *name) {
    assert(mWindow  != NULL);
    assert(mContext != NULL);
    assert(mFuncs   != NULL);
    assert(gdk_gl_context_get_current() == mContext);

    //we are already broken
    if (!gl_required_version) return NULL;
    //if extension is not
    if (!gl_extension_enabled) return NULL;

    void * ret = portableProcAddress(name);
    if (!ret) {
        static const char * suffixes[] = {"ARB","EXT","OES","AMD","NV",NULL};
        auto len = strlen(name);
        char * namebuffer = alloca(len+10);
        memcpy(namebuffer, name, len);
        for(int i = 0; suffixes[i]; ++i) {
            strcpy(namebuffer+len, suffixes[i]);
            ret = portableProcAddress(namebuffer);
            if (ret) break;
        }
    }
    //if required function was not found, mark context as broken
    if (!ret) gl_required_version = 0;
    return ret;
}
