CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LCOPENGL)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")
set(CMAKE_CXX_STANDARD 14)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("***** LibreCAD opengl acceleration layer *****")
# The version number.
set (lcopengl_VERSION_MAJOR 1)
set (lcopengl_VERSION_MINOR 0)

set(opengl_srcs
#glcairo.cpp
#glcairo_p.cpp
#glcairodraw.cpp
#glcairopath.cpp
#glcairosurface.cpp
glcontext.cpp
)

# HEADER FILES
set(opengl_hdrs
common.h
#glcairo.h
#glcairo_p.h
#glcairodraw.h
#glcairopath.h
#glcairosurface.h
glcontext.h
)

find_package(PkgConfig)

# Cairo
pkg_check_modules(CAIRO REQUIRED cairo>=1.13)
include_directories(${CAIRO_INCLUDE_DIRS})
link_directories(${CAIRO_LIBRARY_DIRS})

if (WIN32)
    set(noqt_srcs glcontext_w32.cpp)
    set(NOQT TRUE)
else()
    # OpenGL
    pkg_check_modules(GL gl)
    include_directories(${GL_INCLUDE_DIRS})
    link_directories(${GL_LIBRARY_DIRS})
    # GLU
    pkg_check_modules(GLU glu)
    include_directories(${GLU_INCLUDE_DIRS})
    link_directories(${GLU_LIBRARY_DIRS})
    if (GL_FOUND AND GLU_FOUND)
        pkg_check_modules(GDK gdk-3.0)
        pkg_check_modules(GLIB glib-2.0)
        pkg_check_modules(X11 x11)

        message("FOUND GL=${GL_FOUND} GLU=${GLU_FOUND} GDK=${GDK_FOUND} GLIB=${GLIB_FOUND} X11=${X11_FOUND}")
        #GDK may actually be built without GL support
        if (GDK_FOUND AND GLIB_FOUND)
            # Main include dir
            find_file(GDK_USABLE
                      gdk/gdkglcontext.h
                      PATHS ${GDK_INCLUDE_DIRS})
        endif()
        message("FOUND GL=${GL_FOUND} GLU=${GLU_FOUND} GDK=${GDK_USABLE} X11=${X11_FOUND}")
        if (GDK_USABLE)
            include_directories(${GLIB_INCLUDE_DIRS})
            include_directories(${GDK_INCLUDE_DIRS})
            set(noqt_srcs glcontext_gdk.cpp)
            set(EXTRA_LIBRARIES ${GDK_LIBRARIES})
            set(NOQT TRUE)
        elseif(X11_FOUND)
            include_directories(${X11_INCLUDE_DIRS})
            set(noqt_srcs glcontext_x11.cpp)
            set(EXTRA_LIBRARIES ${X11_LIBRARIES})
            set(NOQT TRUE)
        endif()
    endif()
endif()

if (NOQT)
    add_library(lcopengl_noqt SHARED ${opengl_srcs} ${noqt_srcs} ${opengl_hdrs})
    target_link_libraries(lcopengl_noqt
        ${CAIRO_LIBRARIES}
        ${GL_LIBRARIES}
        ${GLU_LIBRARIES}
        ${EXTRA_LIBRARIES})
endif()

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)

add_library(lcopengl_qt SHARED ${opengl_srcs} ${qt_srcs} glcontext_qt.cpp ${opengl_hdrs})
target_link_libraries(lcopengl_qt
    ${CAIRO_LIBRARIES}
    Qt5::Core
    Qt5::Gui
    Qt5::OpenGL)

#trick to show all files in project tree
file(GLOB ALL_SRC_HEADER_FILES "*.h")
file(GLOB ALL_TEST_HEADER_FILES "*.cpp")
add_library(dummy_target_with_all_headers EXCLUDE_FROM_ALL ${ALL_SRC_HEADER_FILES} ${ALL_TEST_HEADER_FILES})
target_link_libraries(dummy_target_with_all_headers)
set_target_properties(dummy_target_with_all_headers PROPERTIES LINKER_LANGUAGE CXX)
