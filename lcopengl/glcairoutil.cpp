#include "glcairoutil.h"

cairo_status_t glErrorToStatus(GLenum error) {
    cairo_status_t result;
    switch(error) {
    case GL_OUT_OF_MEMORY:
        result = CAIRO_STATUS_NO_MEMORY;
        break;
    default:
        result = CAIRO_STATUS_DEVICE_ERROR;
        break;
    }
    return result;
}
