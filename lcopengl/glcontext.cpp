#include "glcontext.h"
#include <iostream>
#include <GL/glu.h>
#include <string.h>
#include <assert.h>

static int            gl_required_version = 0;
static bool           gl_extension_enabled = true;

int GLContext::limit(GLenum value) {
    int val = 0;
    clearError();
    glGetIntegerv(value, &val);
    if (checkError()) val = 0;
    return val;
}
void GLContext::clearError() const {
    GLenum e = glGetError();
    while (e != GL_NO_ERROR)
        e = glGetError();
}
GLenum GLContext::checkError() const {
    GLenum e = glGetError();
    if (e != GL_NO_ERROR) {
        GLenum e1 = e;
        do {
            std::cerr<<"OpenGL error: "<<gluErrorString(e1)<<std::endl;
            e1 = glGetError();
        } while (e1 != GL_NO_ERROR);
    }
    return e;
}
void   GLContext::beginJourney(int version) {
    gl_required_version  = version;//initial point of our journey
    gl_extension_enabled = true;
}
int   GLContext::completeJourney()
{
    return gl_required_version;
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
                    if (uint(next - extensions) <= len + 2u) //at least one more symbol must exist after prefix
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
    //we are already broken
    if (!gl_required_version) return NULL;
    //if extension is not
    if (!gl_extension_enabled) return NULL;

    void * ret = (void*) getProcAddress(name);
    if (!ret) {
        static const char * suffixes[] = {"ARB","EXT","OES","AMD","NV",NULL};
        auto len = strlen(name);
        char * namebuffer = (char*)alloca(len+10);
        memcpy(namebuffer, name, len);
        for(int i = 0; suffixes[i]; ++i) {
            strcpy(namebuffer+len, suffixes[i]);
            ret = getProcAddress(namebuffer);
            if (ret) break;
        }
    }
    //if required function was not found, mark context as broken
    if (!ret) gl_required_version = 0;
    return ret;
}

static const int gl_versions_static[] = {45, 44, 43, 42, 40, 33, 30, 21, 20, 0};
const int * GLContext::gl_versions = gl_versions_static;
