#pragma once

#include "common.h"
#include <GL/gl.h>
#include <GL/glext.h>

/*
 * useful links
 *
 * Feature support matrix
 * https://people.freedesktop.org/~imirkin/glxinfo/#v=Mesa%2017.1.0
 *
 * Modern OpenGL
 * http://github.prideout.net/modern-opengl-prezo/
 */
#define DLL_IMPORT_FUNC(retval, name, ...) \
    typedef GLAPIENTRY retval (*name ## _t) (__VA_ARGS__); \
    const name ## _t name = (name ## _t) import(#name)

#define DLL_CHECK_EXT(extname, ver) const bool extname = checkExtension(#extname, ver, true)

#define DLL_COND_EXT(extname, version, cc) const bool extname = checkExtension(#extname, version, cc)

#define DLL_FAKE_EXT(extname, version) const bool extname = fakeExtension(#extname, version, true)

#define DLL_CFAKE_EXT(extname, version, cc) const bool extname = fakeExtension(#extname, version, cc)

#define DLL_LIMIT(name) const int name = limit(GL_##name)

class PRIVATE_API GLContext {
    public:
        static GLContext * create();
        DISABLE_COPY(GLContext)
        MOVABLE(GLContext)
        ~GLContext();

        bool makeCurrent();
        void release();

        //UTILITY FUNCTIONS
        void clearError() const;
        GLenum checkError() const;
        //CRITICAL FUNCTIONS: we can't do anything without them
        DLL_IMPORT_FUNC(GLenum, glGetError);
        DLL_IMPORT_FUNC(void, glFinish);
        DLL_IMPORT_FUNC(void, glFlush);
        DLL_IMPORT_FUNC(void, glEnable, GLenum);
        DLL_IMPORT_FUNC(void, glDisable, GLenum);
        DLL_IMPORT_FUNC(const GLubyte*, glGetStringi,
                        GLenum name,
                        GLuint index);
        DLL_IMPORT_FUNC(void, glGetIntegerv,
                        GLenum pname,
                        GLint * value);
        DLL_IMPORT_FUNC(void, glGetFloatv,
                        GLenum pname,
                        GLfloat * value);
        //some critical limits
        DLL_LIMIT(MAX_TEXTURE_SIZE);
        DLL_LIMIT(MAX_TEXTURE_STACK_DEPTH);
        //more CRITICAL FUNCTIONS
        DLL_IMPORT_FUNC(void, glPixelStorei,
                        GLenum pname,
                        GLint param);
        DLL_IMPORT_FUNC(void, glDepthFunc,
                        GLenum func);
        DLL_IMPORT_FUNC(void, glViewport,
                        GLint x,
                        GLint y,
                        GLsizei width,
                        GLsizei height);
        DLL_IMPORT_FUNC(void, glClear,
                        GLbitfield mask);
        DLL_IMPORT_FUNC(void, glDrawBuffer,
                        GLenum buf);
        DLL_IMPORT_FUNC(void, glClearColor,
                        GLclampf red,
                        GLclampf green,
                        GLclampf blue,
                        GLclampf alpha);
        DLL_IMPORT_FUNC(void, glClearDepth,
                        GLclampd depth);
        DLL_IMPORT_FUNC(void, glClearStencil,
                        GLint stencil);
        DLL_IMPORT_FUNC(void, glScissor,
                        GLint x,
                        GLint y,
                        GLsizei width,
                        GLsizei height);
        //===sync objects===================
        DLL_CHECK_EXT(sync, 20);
        DLL_IMPORT_FUNC(GLsync, glFenceSync,
                        GLenum condition,
                        GLbitfield flags);
        DLL_IMPORT_FUNC(void, glDeleteSync,
                        GLsync sync);
        DLL_IMPORT_FUNC(GLenum, glClientWaitSync,
                        GLsync sync,
                        GLbitfield flags,
                        GLuint64 timeout);
        //===vertex array objects============
        DLL_CHECK_EXT(vertex_array_object, 20);
        DLL_IMPORT_FUNC(void, glVertexAttribPointer,
                        GLuint index,
                        GLint size,
                        GLenum type,
                        GLboolean normalized,
                        GLsizei stride,
                        const GLvoid * pointer);
        DLL_IMPORT_FUNC(void, glVertexAttrib1f,
                        GLuint index,
                        GLfloat v0);
        DLL_IMPORT_FUNC(void, glVertexAttrib2f,
                        GLuint index,
                        GLfloat v0,
                        GLfloat v1);
        DLL_IMPORT_FUNC(void, glVertexAttrib3f,
                        GLuint index,
                        GLfloat v0,
                        GLfloat v1,
                        GLfloat v2);
        DLL_IMPORT_FUNC(void, glVertexAttrib4f,
                        GLuint index,
                        GLfloat v0,
                        GLfloat v1,
                        GLfloat v2,
                        GLfloat v3);
        DLL_IMPORT_FUNC(void, glVertexAttrib1i,
                        GLuint index,
                        GLint v0);
        DLL_IMPORT_FUNC(void, glVertexAttrib2i,
                        GLuint index,
                        GLint v0,
                        GLint v1);
        DLL_IMPORT_FUNC(void, glVertexAttrib3i,
                        GLuint index,
                        GLint v0,
                        GLint v1,
                        GLint v2);
        DLL_IMPORT_FUNC(void, glVertexAttrib4i,
                        GLuint index,
                        GLint v0,
                        GLint v1,
                        GLint v2,
                        GLint v3);
        DLL_IMPORT_FUNC(void, glGenVertexArrays,
                        GLsizei n,
                        GLuint *arrays);
        DLL_IMPORT_FUNC(void, glDeleteVertexArrays,
                        GLsizei n,
                        const GLuint *arrays);
        DLL_IMPORT_FUNC(void, glBindVertexArray,
                        GLuint array);
        DLL_IMPORT_FUNC(void, glEnableVertexAttribArray,
                        GLuint index);
        DLL_IMPORT_FUNC(void, glDisableVertexAttribArray,
                        GLuint index);
        DLL_IMPORT_FUNC(void, glVertexAttribIPointer,
                        GLuint index,
                        GLint size,
                        GLenum type,
                        GLsizei stride,
                        const GLvoid * pointer);
        DLL_COND_EXT(vertex_type_2_10_10_10_rev, 20,
                     vertex_array_object);
        //===provoking vertex===============
        DLL_CHECK_EXT(provoking_vertex, 20);
        DLL_IMPORT_FUNC(void, glProvokingVertex,
                        GLenum mode);
        //===buffer objects=================
        DLL_CHECK_EXT(pixel_buffer_object, 20);
        DLL_CHECK_EXT(vertex_buffer_object, 20);
        DLL_CFAKE_EXT(buffer_object, 20,
                pixel_buffer_object &
                vertex_buffer_object);
        DLL_IMPORT_FUNC(void, glGenBuffers,
                        GLsizei count,
                        GLuint* buffers);
        DLL_IMPORT_FUNC(void, glDeleteBuffers,
                        GLsizei count,
                        GLuint* buffers);
        DLL_IMPORT_FUNC(void, glBindBuffer,
                        GLenum target,
                        GLuint buffer);
        DLL_IMPORT_FUNC(void, glBufferData,
                        GLenum target,
                        GLsizeiptr size,
                        const GLvoid * data,
                        GLenum usage);
        DLL_IMPORT_FUNC(void *, glMapBuffer,
                        GLenum target,
                        GLenum access);
        DLL_IMPORT_FUNC(GLboolean, glUnmapBuffer,
                        GLenum target);
        DLL_CHECK_EXT(buffer_storage, 44);
        DLL_IMPORT_FUNC(void, glBufferStorage,
                        GLenum,
                        GLsizeiptr,
                        const GLvoid *,
                        GLbitfield);
        DLL_CHECK_EXT(clear_buffer_object, 43);
        DLL_IMPORT_FUNC(void, glClearBufferData,
                        GLenum target,
                        GLenum internalformat,
                        GLenum format,
                        GLenum type,
                        const GLvoid * data);
        DLL_IMPORT_FUNC(void, glClearBufferSubData,
                        GLenum target,
                        GLenum internalformat,
                        GLintptr offset,
                        GLsizeiptr size,
                        GLenum format,
                        GLenum type,
                        const GLvoid * data);
        //===uniform buffer object===========
        DLL_COND_EXT(uniform_buffer_object, 20,
                     buffer_object);
        DLL_IMPORT_FUNC(GLuint, glGetUniformBlockIndex,
                        GLuint program,
                        const GLchar* uniformBlockName);
        DLL_IMPORT_FUNC(void, glGetActiveUniformBlockiv,
                        GLuint program,
                        GLuint uniformBlockIndex,
                        GLenum pname,
                        GLint* params);
        DLL_IMPORT_FUNC(void, glBindBufferRange,
                        GLenum target,
                        GLuint index,
                        GLuint buffer,
                        GLintptr offset,
                        GLsizeiptr size);
        DLL_IMPORT_FUNC(void, glBindBufferBase,
                        GLenum target,
                        GLuint index,
                        GLuint buffer);
        DLL_IMPORT_FUNC(void, glUniformBlockBinding,
                        GLuint program,
                        GLuint uniformBlockIndex,
                        GLuint uniformBlockBinding);
        //===bledings=======================
        DLL_FAKE_EXT(blending, 20);
        DLL_IMPORT_FUNC(void, glBlendEquation,
                        GLenum mode);
        DLL_CHECK_EXT(blend_equation_separate, 20);
        DLL_IMPORT_FUNC(void, glBlendEquationSeparate,
                        GLenum modeRGB,
                        GLenum modeAlpha);
        DLL_CHECK_EXT(draw_buffers_blend, 20);
        DLL_IMPORT_FUNC(void, glBlendFuncIndexed,
                        GLuint buf,
                        GLenum src,
                        GLenum dst);
        DLL_IMPORT_FUNC(void, glBlendFuncSeparateIndexed,
                        GLuint buf,
                        GLenum srcRGB,
                        GLenum dstRGB,
                        GLenum srcAlpha,
                        GLenum dstAlpha);
        DLL_IMPORT_FUNC(void, glBlendEquationIndexed,
                        GLuint buf,
                        GLenum mode);
        DLL_IMPORT_FUNC(void, glBlendEquationSeparateIndexed,
                        GLuint buf,
                        GLenum modeRGB,
                        GLenum modeAlpha);
        //===framebuffer objects============
        DLL_CHECK_EXT(framebuffer_object, 20);
        DLL_IMPORT_FUNC(void, glGenFramebuffers,
                        GLsizei,
                        GLuint * ids);
        DLL_IMPORT_FUNC(void, glDeleteFramebuffers,
                        GLsizei,
                        GLuint * ids);
        DLL_IMPORT_FUNC(void, glBindFramebuffer,
                        GLenum target,
                        GLuint framebuffer);
        DLL_IMPORT_FUNC(void, glFramebufferTexture2D,
                        GLenum target,
                        GLenum attachment,
                        GLenum textarget,
                        GLuint texture,
                        GLint level);
        DLL_IMPORT_FUNC(GLenum, glCheckFramebufferStatus,
                        GLenum target);
        DLL_IMPORT_FUNC(void, glReadPixels,
                        GLint x,
                        GLint y,
                        GLsizei width,
                        GLsizei height,
                        GLenum format,
                        GLenum type,
                        GLvoid * data);
        DLL_IMPORT_FUNC(void, glReadBuffer,
                        GLenum mode);
        //===texture objects================
        DLL_FAKE_EXT(textures, 20);
        DLL_IMPORT_FUNC(void, glGenTextures,
                        GLsizei n,
                        GLuint* textures);
        DLL_IMPORT_FUNC(void, glDeleteTextures,
                        GLsizei n,
                        GLuint* textures);
        DLL_IMPORT_FUNC(void, glBindTexture,
                        GLenum target,
                        GLuint texture);
        DLL_IMPORT_FUNC(void, glActiveTexture,
                        GLenum texture);
        DLL_CHECK_EXT(texture_rectangle, 20);
        DLL_CHECK_EXT(texture_float, 20);
        DLL_CHECK_EXT(texture_rg, 20);
        DLL_CHECK_EXT(texture_rgb10_a2ui, 20);
        DLL_CHECK_EXT(texture_stencil8, 44);
        DLL_CHECK_EXT(vertex_type_10f_11f_11f_rev, 44);
        DLL_CHECK_EXT(stencil_texturing, 20);
        DLL_CHECK_EXT(clear_texture, 44);
        DLL_IMPORT_FUNC(void, glClearTexImage,
                        GLuint texture,
                        GLint level,
                        GLenum format,
                        GLenum type,
                        const GLvoid * data);
        DLL_IMPORT_FUNC(void, glClearTexSubImage,
                        GLuint texture,
                        GLint level,
                        GLint xoffset,
                        GLint yoffset,
                        GLint zoffset,
                        GLsizei width,
                        GLsizei height,
                        GLsizei depth,
                        GLenum format,
                        GLenum type,
                        const GLvoid * data);
        DLL_IMPORT_FUNC(void, glTexImage1D,
                        GLenum target,
                        GLint level,
                        GLint internalFormat,
                        GLsizei width,
                        GLint border,
                        GLenum format,
                        GLenum type, const
                        GLvoid *pixels);
        DLL_IMPORT_FUNC(void, glTexSubImage1D,
                        GLenum target,
                        GLint level,
                        GLint xoffset,
                        GLsizei width,
                        GLenum format,
                        GLenum type,
                        const GLvoid * data);
        DLL_IMPORT_FUNC(void, glTexImage2D,
                        GLenum target,
                        GLint level,
                        GLint internalFormat,
                        GLsizei width,
                        GLsizei height,
                        GLint border,
                        GLenum format,
                        GLenum type,
                        const GLvoid * data);
        DLL_IMPORT_FUNC(void, glTexSubImage2D,
                        GLenum target,
                        GLint level,
                        GLint xoffset,
                        GLint yoffset,
                        GLsizei width,
                        GLsizei height,
                        GLenum format,
                        GLenum type,
                        const GLvoid * data);
        DLL_IMPORT_FUNC(void, glTexImage3D,
                        GLenum target,
                        GLint level,
                        GLint internalFormat,
                        GLsizei width,
                        GLsizei height,
                        GLsizei depth,
                        GLint border,
                        GLenum format,
                        GLenum type,
                        const GLvoid *pixels);
        DLL_IMPORT_FUNC(void, glTexSubImage3D,
                        GLenum target,
                        GLint level,
                        GLint xoffset,
                        GLint yoffset,
                        GLint zoffset,
                        GLsizei width,
                        GLsizei height,
                        GLsizei depth,
                        GLenum format,
                        GLenum type,
                        const GLvoid * data);
        //===integer texture======================
        DLL_CHECK_EXT(texture_integer, 20);
        DLL_IMPORT_FUNC(void, glClearColorIiEXT,
                        GLint r,
                        GLint g,
                        GLint b,
                        GLint a);
        DLL_IMPORT_FUNC(void, glClearColorIuiEXT,
                        GLuint r,
                        GLuint g,
                        GLuint b,
                        GLuint a);
        //===texture storage======================
        DLL_CHECK_EXT(texture_storage, 42);
        DLL_IMPORT_FUNC(void, glTexStorage1D,
                        GLenum target,
                        GLsizei levels,
                        GLenum internalformat,
                        GLsizei width);
        DLL_IMPORT_FUNC(void, glTexStorage2D,
                        GLenum target,
                        GLsizei levels,
                        GLenum internalformat,
                        GLsizei width,
                        GLsizei height);
        DLL_IMPORT_FUNC(void, glTexStorage3D,
                        GLenum target,
                        GLsizei levels,
                        GLenum internalformat,
                        GLsizei width,
                        GLsizei height,
                        GLsizei depth);

        //===texture buffer range=============
        DLL_CHECK_EXT(texture_buffer_range, 43);
        DLL_IMPORT_FUNC(void, glTexBufferRange,
                        GLenum target,
                        GLenum internalformat,
                        GLuint buffer,
                        GLintptr offset,
                        GLsizeiptr size);
        //===sampler objects==================
        DLL_CHECK_EXT(sampler_objects, 20);
        DLL_IMPORT_FUNC(void, glGenSamplers,
                        GLsizei count,
                        GLuint *samplers);
        DLL_IMPORT_FUNC(void, glDeleteSamplers,
                        GLsizei count,
                        const GLuint * samplers);
        DLL_IMPORT_FUNC(void, glSamplerParameteri,
                        GLuint sampler,
                        GLenum pname,
                        GLint param);
        DLL_IMPORT_FUNC(void, glSamplerParameterf,
                        GLuint sampler,
                        GLenum pname,
                        GLfloat param);
        DLL_IMPORT_FUNC(void, glSamplerParameteriv,
                        GLuint sampler,
                        GLenum pname,
                        const GLint *params);
        DLL_IMPORT_FUNC(void, glSamplerParameterfv,
                        GLuint sampler,
                        GLenum pname,
                        const GLfloat *params);
        //===shader objects=================
        DLL_CHECK_EXT(shader_objects, 20);
        DLL_CHECK_EXT(fragment_shader, 20);
        DLL_CHECK_EXT(vertex_shader, 20);
        DLL_CFAKE_EXT(shaders, 20,
                shader_objects &
                fragment_shader &
                vertex_shader);
        DLL_IMPORT_FUNC(GLuint, glCreateShader,
                        GLenum type);
        DLL_IMPORT_FUNC(void, glDeleteShader,
                        GLuint shader);
        DLL_IMPORT_FUNC(void, glCompileShader,
                        GLuint shader);
        DLL_IMPORT_FUNC(void, glGetShaderInfoLog,
                        GLuint shader,
                        GLsizei maxLength,
                        GLsizei *length,
                        GLchar *infoLog);
        //===shader programs=================
        DLL_IMPORT_FUNC(GLuint, glCreateProgram);
        DLL_IMPORT_FUNC(void, glDeleteProgram,
                        GLuint program);
        DLL_IMPORT_FUNC(void, glAttachShader,
                        GLuint program,
                        GLuint shader);
        DLL_IMPORT_FUNC(void, glLinkProgram,
                        GLuint program);
        DLL_IMPORT_FUNC(void, glGetProgramInfoLog,
                        GLuint program,
                        GLsizei maxLength,
                        GLsizei *length,
                        GLchar *infoLog);
        DLL_IMPORT_FUNC(void, glValidateProgram,
                        GLuint program);
        DLL_IMPORT_FUNC(void, glUseProgram,
                        GLuint program);
        DLL_IMPORT_FUNC(void, glBindAttribLocation,
                        GLuint program,
                        GLuint index,
                        const GLchar *name);
        DLL_IMPORT_FUNC(GLint, glGetUniformLocation,
                        GLuint program,
                        const GLchar *name);
        DLL_IMPORT_FUNC(void, glUniform1f,
                        GLint location,
                        GLfloat v0);
        DLL_IMPORT_FUNC(void, glUniform2f,
                        GLint location,
                        GLfloat v0,
                        GLfloat v1);
        DLL_IMPORT_FUNC(void, glUniform3f,
                        GLint location,
                        GLfloat v0,
                        GLfloat v1,
                        GLfloat v2);
        DLL_IMPORT_FUNC(void, glUniform4f,
                        GLint location,
                        GLfloat v0,
                        GLfloat v1,
                        GLfloat v2,
                        GLfloat v3);
        DLL_IMPORT_FUNC(void, glUniform1i,
                        GLint location,
                        GLint v0);
        DLL_IMPORT_FUNC(void, glUniform2i,
                        GLint location,
                        GLint v0,
                        GLint v1);
        DLL_IMPORT_FUNC(void, glUniform3i,
                        GLint location,
                        GLint v0,
                        GLint v1,
                        GLint v2);
        DLL_IMPORT_FUNC(void, glUniform4i,
                        GLint location,
                        GLint v0,
                        GLint v1,
                        GLint v2,
                        GLint v3);
        DLL_IMPORT_FUNC(void, glUniformMatrix2fv,
                        GLint location,
                        GLsizei count,
                        GLboolean transpose,
                        const GLfloat *value);
        DLL_IMPORT_FUNC(void, glUniformMatrix3fv,
                        GLint location,
                        GLsizei count,
                        GLboolean transpose,
                        const GLfloat *value);
        DLL_IMPORT_FUNC(void, glUniformMatrix4fv,
                        GLint location,
                        GLsizei count,
                        GLboolean transpose,
                        const GLfloat *value);
        DLL_COND_EXT(geometry_shader4, 33,
                     shaders);
        //===misc features==================
        DLL_COND_EXT(conservative_depth, 30, shaders);
        DLL_COND_EXT(shader_precision, 41, shaders);
        DLL_COND_EXT(shader_atomic_counters, 42, shaders);
        DLL_COND_EXT(shader_integer_functions, 20, shaders);
        DLL_COND_EXT(fragment_layer_viewport, 43, shaders);
        DLL_COND_EXT(texture_gather, 40, shaders);
        DLL_COND_EXT(shading_language_packing, 43, shaders);
        DLL_COND_EXT(shader_stencil_export, 20, shaders);
        DLL_COND_EXT(shader_integer_mix, 20, shaders);
        DLL_COND_EXT(gpu_shader5, 40, shaders);
        DLL_COND_EXT(gpu_shader_fp64, 40, shaders);
        //===fp64 features===================
        DLL_IMPORT_FUNC(void, glUniform1d,
                        GLint location,
                        GLdouble x);
        DLL_IMPORT_FUNC(void, glUniform2d,
                        GLint location,
                        GLdouble x,
                        GLdouble y);
        DLL_IMPORT_FUNC(void, glUniform3d,
                        GLint location,
                        GLdouble x,
                        GLdouble y,
                        GLdouble z);
        DLL_IMPORT_FUNC(void, glUniform4d,
                        GLint location,
                        GLdouble x,
                        GLdouble y,
                        GLdouble z,
                        GLdouble w);
        DLL_IMPORT_FUNC(void, glUniformMatrix2dv,
                        GLint location,
                        GLsizei count,
                        GLboolean transpose,
                        const GLdouble *value);
        DLL_IMPORT_FUNC(void, glUniformMatrix3dv,
                        GLint location,
                        GLsizei count,
                        GLboolean transpose,
                        const GLdouble *value);
        DLL_IMPORT_FUNC(void, glUniformMatrix4dv,
                        GLint location,
                        GLsizei count,
                        GLboolean transpose,
                        const GLdouble *value);
        DLL_IMPORT_FUNC(void, glProgramUniform1d,
                        GLuint program,
                        GLint location,
                        GLdouble x);
        DLL_IMPORT_FUNC(void, glProgramUniform2d,
                        GLuint program,
                        GLint location,
                        GLdouble x,
                        GLdouble y);
        DLL_IMPORT_FUNC(void, glProgramUniform3d,
                        GLuint program,
                        GLint location,
                        GLdouble x,
                        GLdouble y,
                        GLdouble z);
        DLL_IMPORT_FUNC(void, glProgramUniform4d,
                        GLuint program,
                        GLint location,
                        GLdouble x,
                        GLdouble y,
                        GLdouble z,
                        GLdouble w);
        DLL_IMPORT_FUNC(void, glProgramUniformMatrix2dv,
                        GLuint program,
                        GLint location,
                        GLsizei count,
                        GLboolean transpose,
                        const GLdouble *value);
        DLL_IMPORT_FUNC(void, glProgramUniformMatrix3dv,
                        GLuint program,
                        GLint location,
                        GLsizei count,
                        GLboolean transpose,
                        const GLdouble *value);
        DLL_IMPORT_FUNC(void, glProgramUniformMatrix4dv,
                        GLuint program,
                        GLint location,
                        GLsizei count,
                        GLboolean transpose,
                        const GLdouble *value);
        //===tesselation====================
        DLL_COND_EXT(tessellation_shader, 40, shaders);
        DLL_IMPORT_FUNC(void, glPatchParameteri,
                        GLenum pname,
                        GLint value);
        DLL_IMPORT_FUNC(void, glPatchParameterfv,
                        GLenum pname,
                        const GLfloat *values);
        //===compute shaders================
        DLL_COND_EXT(compute_shader, 43, shaders);
        DLL_IMPORT_FUNC(void, glDispatchCompute,
                        GLuint num_groups_x,
                        GLuint num_groups_y,
                        GLuint num_groups_z);
        DLL_IMPORT_FUNC(void, glDispatchComputeIndirect,
                        GLintptr indirect);
        //===draw===========================
        DLL_FAKE_EXT(draw, 20);
        DLL_IMPORT_FUNC(void, glDrawArrays,
                        GLenum mode,
                        GLint first,
                        GLsizei count);
        DLL_IMPORT_FUNC(void, glDrawElements,
                        GLenum mode,
                        GLsizei count,
                        GLenum type,
                        const GLvoid * indices);
        DLL_IMPORT_FUNC(void, glDrawRangeElements,
                        GLenum mode,
                        GLuint start,
                        GLuint end,
                        GLsizei count,
                        GLenum type,
                        const GLvoid * indices);
        //===draw instanced=================
        DLL_CHECK_EXT(draw_instanced, 20);
        DLL_IMPORT_FUNC(void, glDrawArraysInstanced,
                        GLenum mode,
                        GLint first,
                        GLsizei count,
                        GLsizei primcount);
        DLL_IMPORT_FUNC(void, glDrawElementsInstanced,
                        GLenum mode,
                        GLsizei count,
                        GLenum type,
                        const GLvoid *indices,
                        GLsizei primcount);
        //===base instance==================
        DLL_COND_EXT(base_instance, 42, draw_instanced);
        DLL_IMPORT_FUNC(void, glDrawArraysInstancedBaseInstance,
                        GLenum mode,
                        GLint first,
                        GLsizei count,
                        GLsizei primcount,
                        GLuint baseinstance);
        DLL_IMPORT_FUNC(void, glDrawElementsInstancedBaseInstance,
                        GLenum mode,
                        GLsizei count,
                        GLenum type,
                        const GLvoid *indices,
                        GLsizei primcount,
                        GLuint baseinstance);
        DLL_IMPORT_FUNC(void, glDrawElementsInstancedBaseVertexBaseInstance,
                        GLenum mode,
                        GLsizei count,
                        GLenum type,
                        const GLvoid *indices,
                        GLsizei primcount,
                        GLint basevertex,
                        GLuint baseinstance);
        //===indirect render=================
        DLL_CHECK_EXT(draw_indirect, 40);
        DLL_IMPORT_FUNC(void, glDrawArraysIndirect,
                        GLenum mode,
                        const GLvoid *indirect);
        DLL_IMPORT_FUNC(void, glDrawElementsIndirect,
                        GLenum mode,
                        GLenum type,
                        const GLvoid *indirect);
        DLL_CHECK_EXT(multi_draw_indirect, 43);
        DLL_IMPORT_FUNC(void, glMultiDrawArraysIndirect,
                        GLenum mode,
                        const GLvoid *indirect,
                        GLsizei drawcount,
                        GLsizei stride);
        DLL_IMPORT_FUNC(void, glMultiDrawElementsIndirect,
                        GLenum mode,
                        GLenum type,
                        const GLvoid *indirect,
                        GLsizei drawcount,
                        GLsizei stride);
        DLL_COND_EXT(indirect_parameters, 46, draw_indirect);
        DLL_IMPORT_FUNC(void, glMultiDrawArraysIndirectCount,
                        GLenum mode,
                        const GLvoid *indirect,
                        GLintptr drawcount,
                        GLsizei maxdrawcount,
                        GLsizei stride);
        DLL_IMPORT_FUNC(void, glMultiDrawElementsIndirectCount,
                        GLenum mode,
                        GLenum type,
                        const GLvoid *indirect,
                        GLintptr drawcount,
                        GLsizei maxdrawcount,
                        GLsizei stride);
        //===primitive restart===============
        DLL_CHECK_EXT(primitive_restart, 20);
        DLL_IMPORT_FUNC(void, glPrimitiveRestartIndex,
                        GLuint index);
        //===conditional render==============
        DLL_CHECK_EXT(occlusion_query, 20);
        DLL_IMPORT_FUNC(void, glBeginQuery,
                        GLenum target,
                        GLuint id);
        DLL_IMPORT_FUNC(void, glEndQuery,
                        GLenum target);
        DLL_IMPORT_FUNC(void, glGetQueryObjectiv,
                        GLuint id,
                        GLenum pname,
                        GLint * params);
        DLL_IMPORT_FUNC(void, glGetQueryObjectuiv,
                        GLuint id,
                        GLenum pname,
                        GLuint * params);
        DLL_COND_EXT(conditional_render, 20, occlusion_query);
        DLL_IMPORT_FUNC(void, glBeginConditionalRender,
                        GLuint id,
                        GLenum mode);
        DLL_IMPORT_FUNC(void, glEndConditionalRender);
        //===texture barriers================
        //nvidia driver behaves like shit if not used properly
        DLL_CHECK_EXT(texture_barrier, 45);
        DLL_IMPORT_FUNC(void, glTextureBarrier);
        //===load store======================
        DLL_CHECK_EXT(shader_image_load_store, 42);
        DLL_IMPORT_FUNC(void, glMemoryBarrier,
                        GLbitfield barriers);
        DLL_IMPORT_FUNC(void, glBindImageTexture,
                        GLuint unit,
                        GLuint texture,
                        GLint level,
                        GLboolean layered,
                        GLint layer,
                        GLenum access,
                        GLenum format);
        //===shader storage===================
        DLL_CHECK_EXT(shader_storage_buffer_object, 43);
        DLL_IMPORT_FUNC(void, glShaderStorageBlockBinding,
                        GLuint program,
                        GLuint storageBlockIndex,
                        GLuint storageBlockBinding);
        //===multibind========================
        DLL_CHECK_EXT(multi_bind, 44);
        DLL_IMPORT_FUNC(void, glBindBuffersBase,
                        GLenum target,
                        GLuint first,
                        GLsizei count,
                        const GLuint *buffers);
        DLL_IMPORT_FUNC(void, glBindBuffersRange,
                        GLenum target,
                        GLuint first,
                        GLsizei count,
                        const GLuint *buffers,
                        const GLintptr *offsets,
                        const GLsizeiptr *sizes);
        DLL_IMPORT_FUNC(void, glBindTextures,
                        GLuint first,
                        GLsizei count,
                        const GLuint *textures);
        DLL_IMPORT_FUNC(void, glBindSamplers,
                        GLuint first,
                        GLsizei count,
                        const GLuint *samplers);
        DLL_IMPORT_FUNC(void, glBindImageTextures,
                        GLuint first,
                        GLsizei count,
                        const GLuint *textures);
        DLL_IMPORT_FUNC(void, glBindVertexBuffers,
                        GLuint first,
                        GLsizei count,
                        const GLuint *buffers,
                        const GLintptr *offsets,
                        const GLsizei *strides);

        DLL_CFAKE_EXT(gl20, 20,
                      shaders &
                      buffer_object &
                      framebuffer_object &
                      pixel_buffer_object &
                      texture_rectangle &
                      draw & sync);

private:
    GLContext();//you cannot instantiate this class this way
    static void beginJourney(int version);
    static int  completeJourney();
    bool  checkExtension(const char*ext, int version, bool cc);
    bool  fakeExtension(const char*ext, int version, bool cc);
    void *import(const char*);
    int   limit(GLenum value);
    void * getProcAddress(const char * name);
    static const int * gl_versions;
};

#undef DLL_IMPORT_FUNC
