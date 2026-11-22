/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024, 2025, 2026
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include "flextGL.h"

#ifdef __cplusplus
extern "C" {
#endif

/* GL_VERSION_1_0 */
#undef glBlendFunc
#undef glClear
#undef glClearColor
#undef glClearDepth
#undef glClearStencil
#undef glColorMask
#undef glCullFace
#undef glDepthFunc
#undef glDepthMask
#undef glDepthRange
#undef glDisable
#undef glDrawBuffer
#undef glEnable
#undef glFinish
#undef glFlush
#undef glFrontFace
#undef glGetBooleanv
#undef glGetDoublev
#undef glGetError
#undef glGetFloatv
#undef glGetIntegerv
#undef glGetString
#undef glGetTexImage
#undef glGetTexLevelParameterfv
#undef glGetTexLevelParameteriv
#undef glGetTexParameterfv
#undef glGetTexParameteriv
#undef glHint
#undef glIsEnabled
#undef glLineWidth
#undef glLogicOp
#undef glPixelStoref
#undef glPixelStorei
#undef glPointSize
#undef glPolygonMode
#undef glReadBuffer
#undef glReadPixels
#undef glScissor
#undef glStencilFunc
#undef glStencilMask
#undef glStencilOp
#undef glTexImage1D
#undef glTexImage2D
#undef glTexParameterf
#undef glTexParameterfv
#undef glTexParameteri
#undef glTexParameteriv
#undef glViewport
extern void APIENTRY glBlendFunc(GLenum, GLenum);
extern void APIENTRY glClear(GLbitfield);
extern void APIENTRY glClearColor(GLfloat, GLfloat, GLfloat, GLfloat);
extern void APIENTRY glClearDepth(GLdouble);
extern void APIENTRY glClearStencil(GLint);
extern void APIENTRY glColorMask(GLboolean, GLboolean, GLboolean, GLboolean);
extern void APIENTRY glCullFace(GLenum);
extern void APIENTRY glDepthFunc(GLenum);
extern void APIENTRY glDepthMask(GLboolean);
extern void APIENTRY glDepthRange(GLdouble, GLdouble);
extern void APIENTRY glDisable(GLenum);
extern void APIENTRY glDrawBuffer(GLenum);
extern void APIENTRY glEnable(GLenum);
extern void APIENTRY glFinish(void);
extern void APIENTRY glFlush(void);
extern void APIENTRY glFrontFace(GLenum);
extern void APIENTRY glGetBooleanv(GLenum, GLboolean *);
extern void APIENTRY glGetDoublev(GLenum, GLdouble *);
extern GLenum APIENTRY glGetError(void);
extern void APIENTRY glGetFloatv(GLenum, GLfloat *);
extern void APIENTRY glGetIntegerv(GLenum, GLint *);
extern const GLubyte * APIENTRY glGetString(GLenum);
extern void APIENTRY glGetTexImage(GLenum, GLint, GLenum, GLenum, void *);
extern void APIENTRY glGetTexLevelParameterfv(GLenum, GLint, GLenum, GLfloat *);
extern void APIENTRY glGetTexLevelParameteriv(GLenum, GLint, GLenum, GLint *);
extern void APIENTRY glGetTexParameterfv(GLenum, GLenum, GLfloat *);
extern void APIENTRY glGetTexParameteriv(GLenum, GLenum, GLint *);
extern void APIENTRY glHint(GLenum, GLenum);
extern GLboolean APIENTRY glIsEnabled(GLenum);
extern void APIENTRY glLineWidth(GLfloat);
extern void APIENTRY glLogicOp(GLenum);
extern void APIENTRY glPixelStoref(GLenum, GLfloat);
extern void APIENTRY glPixelStorei(GLenum, GLint);
extern void APIENTRY glPointSize(GLfloat);
extern void APIENTRY glPolygonMode(GLenum, GLenum);
extern void APIENTRY glReadBuffer(GLenum);
extern void APIENTRY glReadPixels(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *);
extern void APIENTRY glScissor(GLint, GLint, GLsizei, GLsizei);
extern void APIENTRY glStencilFunc(GLenum, GLint, GLuint);
extern void APIENTRY glStencilMask(GLuint);
extern void APIENTRY glStencilOp(GLenum, GLenum, GLenum);
extern void APIENTRY glTexImage1D(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const void *);
extern void APIENTRY glTexImage2D(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *);
extern void APIENTRY glTexParameterf(GLenum, GLenum, GLfloat);
extern void APIENTRY glTexParameterfv(GLenum, GLenum, const GLfloat *);
extern void APIENTRY glTexParameteri(GLenum, GLenum, GLint);
extern void APIENTRY glTexParameteriv(GLenum, GLenum, const GLint *);
extern void APIENTRY glViewport(GLint, GLint, GLsizei, GLsizei);

/* GL_VERSION_1_1 */
#undef glBindTexture
#undef glCopyTexImage1D
#undef glCopyTexImage2D
#undef glCopyTexSubImage1D
#undef glCopyTexSubImage2D
#undef glDeleteTextures
#undef glDrawArrays
#undef glDrawElements
#undef glGenTextures
#undef glIsTexture
#undef glPolygonOffset
#undef glTexSubImage1D
#undef glTexSubImage2D
extern void APIENTRY glBindTexture(GLenum, GLuint);
extern void APIENTRY glCopyTexImage1D(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
extern void APIENTRY glCopyTexImage2D(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
extern void APIENTRY glCopyTexSubImage1D(GLenum, GLint, GLint, GLint, GLint, GLsizei);
extern void APIENTRY glCopyTexSubImage2D(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern void APIENTRY glDeleteTextures(GLsizei, const GLuint *);
extern void APIENTRY glDrawArrays(GLenum, GLint, GLsizei);
extern void APIENTRY glDrawElements(GLenum, GLsizei, GLenum, const void *);
extern void APIENTRY glGenTextures(GLsizei, GLuint *);
extern GLboolean APIENTRY glIsTexture(GLuint);
extern void APIENTRY glPolygonOffset(GLfloat, GLfloat);
extern void APIENTRY glTexSubImage1D(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const void *);
extern void APIENTRY glTexSubImage2D(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *);

FlextGL flextGL{};

#ifdef __cplusplus
}
#endif

namespace {

/* Assigned before main() so flextGL can be zero-initialized. The stores are
   volatile because clang would otherwise move them into its initializer. */
struct GL11FunctionPointerInitializer {
    GL11FunctionPointerInitializer() {

        /* GL_VERSION_1_0 */
        static_cast<volatile FlextGL&>(flextGL).BlendFunc = glBlendFunc;
        static_cast<volatile FlextGL&>(flextGL).Clear = glClear;
        static_cast<volatile FlextGL&>(flextGL).ClearColor = glClearColor;
        static_cast<volatile FlextGL&>(flextGL).ClearDepth = glClearDepth;
        static_cast<volatile FlextGL&>(flextGL).ClearStencil = glClearStencil;
        static_cast<volatile FlextGL&>(flextGL).ColorMask = glColorMask;
        static_cast<volatile FlextGL&>(flextGL).CullFace = glCullFace;
        static_cast<volatile FlextGL&>(flextGL).DepthFunc = glDepthFunc;
        static_cast<volatile FlextGL&>(flextGL).DepthMask = glDepthMask;
        static_cast<volatile FlextGL&>(flextGL).DepthRange = glDepthRange;
        static_cast<volatile FlextGL&>(flextGL).Disable = glDisable;
        static_cast<volatile FlextGL&>(flextGL).DrawBuffer = glDrawBuffer;
        static_cast<volatile FlextGL&>(flextGL).Enable = glEnable;
        static_cast<volatile FlextGL&>(flextGL).Finish = glFinish;
        static_cast<volatile FlextGL&>(flextGL).Flush = glFlush;
        static_cast<volatile FlextGL&>(flextGL).FrontFace = glFrontFace;
        static_cast<volatile FlextGL&>(flextGL).GetBooleanv = glGetBooleanv;
        static_cast<volatile FlextGL&>(flextGL).GetDoublev = glGetDoublev;
        static_cast<volatile FlextGL&>(flextGL).GetError = glGetError;
        static_cast<volatile FlextGL&>(flextGL).GetFloatv = glGetFloatv;
        static_cast<volatile FlextGL&>(flextGL).GetIntegerv = glGetIntegerv;
        static_cast<volatile FlextGL&>(flextGL).GetString = glGetString;
        static_cast<volatile FlextGL&>(flextGL).GetTexImage = glGetTexImage;
        static_cast<volatile FlextGL&>(flextGL).GetTexLevelParameterfv = glGetTexLevelParameterfv;
        static_cast<volatile FlextGL&>(flextGL).GetTexLevelParameteriv = glGetTexLevelParameteriv;
        static_cast<volatile FlextGL&>(flextGL).GetTexParameterfv = glGetTexParameterfv;
        static_cast<volatile FlextGL&>(flextGL).GetTexParameteriv = glGetTexParameteriv;
        static_cast<volatile FlextGL&>(flextGL).Hint = glHint;
        static_cast<volatile FlextGL&>(flextGL).IsEnabled = glIsEnabled;
        static_cast<volatile FlextGL&>(flextGL).LineWidth = glLineWidth;
        static_cast<volatile FlextGL&>(flextGL).LogicOp = glLogicOp;
        static_cast<volatile FlextGL&>(flextGL).PixelStoref = glPixelStoref;
        static_cast<volatile FlextGL&>(flextGL).PixelStorei = glPixelStorei;
        static_cast<volatile FlextGL&>(flextGL).PointSize = glPointSize;
        static_cast<volatile FlextGL&>(flextGL).PolygonMode = glPolygonMode;
        static_cast<volatile FlextGL&>(flextGL).ReadBuffer = glReadBuffer;
        static_cast<volatile FlextGL&>(flextGL).ReadPixels = glReadPixels;
        static_cast<volatile FlextGL&>(flextGL).Scissor = glScissor;
        static_cast<volatile FlextGL&>(flextGL).StencilFunc = glStencilFunc;
        static_cast<volatile FlextGL&>(flextGL).StencilMask = glStencilMask;
        static_cast<volatile FlextGL&>(flextGL).StencilOp = glStencilOp;
        static_cast<volatile FlextGL&>(flextGL).TexImage1D = glTexImage1D;
        static_cast<volatile FlextGL&>(flextGL).TexImage2D = glTexImage2D;
        static_cast<volatile FlextGL&>(flextGL).TexParameterf = glTexParameterf;
        static_cast<volatile FlextGL&>(flextGL).TexParameterfv = glTexParameterfv;
        static_cast<volatile FlextGL&>(flextGL).TexParameteri = glTexParameteri;
        static_cast<volatile FlextGL&>(flextGL).TexParameteriv = glTexParameteriv;
        static_cast<volatile FlextGL&>(flextGL).Viewport = glViewport;

        /* GL_VERSION_1_1 */
        static_cast<volatile FlextGL&>(flextGL).BindTexture = glBindTexture;
        static_cast<volatile FlextGL&>(flextGL).CopyTexImage1D = glCopyTexImage1D;
        static_cast<volatile FlextGL&>(flextGL).CopyTexImage2D = glCopyTexImage2D;
        static_cast<volatile FlextGL&>(flextGL).CopyTexSubImage1D = glCopyTexSubImage1D;
        static_cast<volatile FlextGL&>(flextGL).CopyTexSubImage2D = glCopyTexSubImage2D;
        static_cast<volatile FlextGL&>(flextGL).DeleteTextures = glDeleteTextures;
        static_cast<volatile FlextGL&>(flextGL).DrawArrays = glDrawArrays;
        static_cast<volatile FlextGL&>(flextGL).DrawElements = glDrawElements;
        static_cast<volatile FlextGL&>(flextGL).GenTextures = glGenTextures;
        static_cast<volatile FlextGL&>(flextGL).IsTexture = glIsTexture;
        static_cast<volatile FlextGL&>(flextGL).PolygonOffset = glPolygonOffset;
        static_cast<volatile FlextGL&>(flextGL).TexSubImage1D = glTexSubImage1D;
        static_cast<volatile FlextGL&>(flextGL).TexSubImage2D = glTexSubImage2D;
    }
} gl11FunctionPointerInitializer;

}
