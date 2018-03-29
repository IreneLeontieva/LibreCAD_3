#pragma once

#ifdef __WIN32
#define PRIVATE_API
#else
#define PRIVATE_API     __attribute__((visibility("hidden")))
#endif

#ifdef __WIN32
#  ifdef LIB_LC_OPENGL
#    define PUBLIC_API     __declspec(dllexport)
#  else
#    define PUBLIC_API     __declspec(dllimport)
#  endif
#else
#define PUBLIC_API
#endif

#define DISABLE_COPY(xxx) \
xxx(const xxx&) = delete;\
xxx& operator =(const xxx&) = delete;\
xxx(xxx&&) = delete;\
xxx& operator =(xxx&&) = delete;

#define WARNING(xxxx) \
    std::cerr<<(xxxx)<<std::endl;

#define BUG_ON(cond, xxxx) \
    if(cond){std::cerr<<(xxxx)<<std::endl;abort();}
