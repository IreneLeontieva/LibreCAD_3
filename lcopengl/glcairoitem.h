#pragma once

#include "common.h"
#include "glcontext.h"

/*
 * base for all our classes
 */
#define CAIRO_ITEM static const char * type;
#define CAIRO_ITEM_IMPL(cls) const char * cls::type = #cls;

class PUBLIC_API GLCairoContext;
class PUBLIC_API GLCairoItem {
public:
    void ref() { ++mRefcount; }
    void unref();

    template <class T>
    T * get() const {
        return mType == T::type ? static_cast<T*>(this) : nullptr;
    }
    GLCairoContext  * owner() const { return mOwner; }
protected:
    GLCairoItem(GLCairoContext * owner, const char * type);
    virtual ~GLCairoItem();
    const GLContext * getGL() const { return (const GLContext*)mGL; }
private:
    GLCairoContext * mOwner;
    const char     * mType;
    const void     * mGL;       //declare as void to suppress warnings
    unsigned         mRefcount;

    //this function will be called from GLCairoContext destructor
    //it will forcefully clean up all resources
    friend class GLCairoContext;
    static void forceDelete(GLCairoItem * item);

    DISABLE_COPY(GLCairoItem)
    DISABLE_MOVE(GLCairoItem)
};

