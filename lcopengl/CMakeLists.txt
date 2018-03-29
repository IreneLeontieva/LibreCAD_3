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

# SOURCE FILES
set(qt_srcs glcontext_qt.cpp)
set(gdk_srcs glcontext_gdk.cpp)
set(opengl_srcs
#gl_cairo.cpp
#gl_cairo_surface.cpp
glcontext.cpp
)

# HEADER FILES
set(opengl_hdrs
common.h
#gl_cairo.h
#gl_cairo_p.h
#gl_cairo_surface.h
glcontext.h
)

find_package(PkgConfig)

# Cairo
pkg_check_modules(CAIRO REQUIRED cairo>=1.13)
include_directories(${CAIRO_INCLUDE_DIRS})
link_directories(${CAIRO_LIBRARY_DIRS})

# Pango
pkg_check_modules(PANGO REQUIRED pango>=1.36)
include_directories(${PANGO_INCLUDE_DIRS})
link_directories(${PANGO_LIBRARY_DIRS})

# GDK-Pixbuf
find_package(GDK-Pixbuf REQUIRED)
include_directories(${GDK-PIXBUF_INCLUDE_DIRS})

# GDK
find_package(GDK REQUIRED)
include_directories(${GDK_INCLUDE_DIRS})

# GDK
find_package(GDK REQUIRED)
include_directories(${GDK_INCLUDE_DIRS})

# OpenGL
pkg_check_modules(GL REQUIRED gl)
include_directories(${GL_INCLUDE_DIRS})
link_directories(${GL_LIBRARY_DIRS})

# GLU
pkg_check_modules(GLU REQUIRED glu)
include_directories(${GLU_INCLUDE_DIRS})
link_directories(${GLU_LIBRARY_DIRS})

find_package(Qt5Core REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Gui REQUIRED)

add_library(lcopengl_qt SHARED ${opengl_srcs} ${qt_srcs}  ${opengl_hdrs})
add_library(lcopengl_gdk SHARED ${opengl_srcs} ${gdk_srcs} ${opengl_hdrs})

target_link_libraries(lcopengl_gdk
    ${CAIRO_LIBRARIES}
    ${PANGO_LIBRARIES}
    ${GDK-PIXBUF_LIBRARIES}
    ${GDK_LIBRARIES}
    ${GL_LIBRARIES}
    ${GLU_LIBRARIES})

target_link_libraries(lcopengl_qt
    ${CAIRO_LIBRARIES}
    ${GL_LIBRARIES}
    ${GLU_LIBRARIES}
    Qt5::Core
    Qt5::Gui
    Qt5::OpenGL)
