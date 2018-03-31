#pragma once
#include "glcontext.h"
#include "glcairoitem.h"
#include <cairo/cairo.h>
#include <unordered_set>

//forward declarations
class PUBLIC_API GLCairoItem;
class PUBLIC_API GLCairoContext;
class PUBLIC_API GLCairoDraw;
class PUBLIC_API GLCairoPath;
class PUBLIC_API GLCairoSurface;
class PUBLIC_API GLCairoPattern;
class PUBLIC_API GLCairoFontFace;
class PUBLIC_API GLCairoScaledFont;

class PUBLIC_API GLCairoContext {
public:
    GLCairoContext();
    virtual ~GLCairoContext();

    //a shortcut to obtain nil surfaces
    cairo_surface_t *
    nilSurface();

    //cairo_surface_t * 	cairo_image_surface_create ()
    //https://www.cairographics.org/manual/cairo-Image-Surfaces.html#cairo-image-surface-create
    cairo_surface_t *
    cairo_image_surface_create (cairo_format_t format,
                                int width,
                                int height);
    //cairo_surface_t * 	cairo_image_surface_create_for_data ()
    //https://www.cairographics.org/manual/cairo-Image-Surfaces.html#cairo-image-surface-create-for-data
    cairo_surface_t *
    cairo_image_surface_create_for_data (unsigned char *data,
                                         cairo_format_t format,
                                         int width,
                                         int height,
                                         int stride);

    //cairo_surface_t * 	cairo_surface_create_similar ()
    //https://www.cairographics.org/manual/cairo-cairo-surface-t.html#cairo-surface-create-similar
    cairo_surface_t * cairo_surface_create_similar(cairo_surface_t *other,
                                                   cairo_content_t content,
                                                   int width,
                                                   int height);
    //cairo_surface_t * 	cairo_surface_create_similar_image ()
    //https://www.cairographics.org/manual/cairo-cairo-surface-t.html#cairo-surface-create-similar-image
    cairo_surface_t *
    cairo_surface_create_similar_image (cairo_surface_t *other,
                                        cairo_format_t format,
                                        int width,
                                        int height);

    //cairo_surface_t * 	cairo_surface_create_for_rectangle ()
    //https://www.cairographics.org/manual/cairo-cairo-surface-t.html#cairo-surface-create-for-rectangle
    cairo_surface_t *
    cairo_surface_create_for_rectangle (cairo_surface_t *target,
                                        double x,
                                        double y,
                                        double width,
                                        double height);
    //cairo_surface_t * 	cairo_surface_reference ()
    //https://www.cairographics.org/manual/cairo-cairo-surface-t.html#cairo-surface-reference
    cairo_surface_t *
    cairo_surface_reference (cairo_surface_t *surface);

    GLCairoDraw       * drawing(cairo_t * cairo);
    GLCairoSurface    * surface(cairo_surface_t * surface);
    GLCairoPath       * path(cairo_path_t * path);
    GLCairoPattern    * pattern(cairo_pattern_t * pattern);
    GLCairoFontFace   * fontface(cairo_font_face_t * font);
    GLCairoScaledFont * scaledfont(cairo_scaled_font_t * font);
    const GLContext* gl() const { return (const GLContext*)mGL; }
private:
    void                           * mGL;   //declare as void to suppress warnings
    std::unordered_set<GLCairoItem*> mItems;

    //===buffer we will be use to store vertices===========
    GLuint mVertexBuffer, m;
    GLuint mIndexBuffer;
    friend class GLCairoItem;
    void unlink(GLCairoItem* item);

    DISABLE_COPY(GLCairoContext)
    DISABLE_MOVE(GLCairoContext)
};
