#include "glcontext.h"

int GLContext::limit(GLenum value) {
    int val = 0;
    clearError();
    glGetIntegerv(value, &val);
    if (checkError()) val = 0;
    return val;
}
