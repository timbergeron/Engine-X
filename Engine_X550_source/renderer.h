// renderer.h

#ifndef __RENDERER_H__
#define __RENDERER_H__


// gl_manager.c
void (APIENTRY *MeglAlphaFunc) (GLenum func, GLclampf ref);
void (APIENTRY *MeglBlendFunc) (GLenum sfactor, GLenum dfactor);
void (APIENTRY *MeglClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void (APIENTRY *MeglColor3f) (GLfloat red, GLfloat green, GLfloat blue);
void (APIENTRY *MeglColor3fv) (const GLfloat *v);
void (APIENTRY *MeglColor3ubv) (const GLubyte *v);
void (APIENTRY *MeglColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void (APIENTRY *MeglColor4fv) (const GLfloat *v);
void (APIENTRY *MeglColor4ub) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
void (APIENTRY *MeglColor4ubv) (const GLubyte *v);
void (APIENTRY *MeglColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void (APIENTRY *MeglCullFace) (GLenum mode);
void (APIENTRY *MeglDepthFunc) (GLenum func);
void (APIENTRY *MeglDepthMask) (GLboolean flag);
void (APIENTRY *MeglDepthRange) (GLclampd zNear, GLclampd zFar);
void (APIENTRY *MeglDisable) (GLenum cap);
void (APIENTRY *MeglEnable) (GLenum cap);
void (APIENTRY *MeglFogf) (GLenum pname, GLfloat param);
void (APIENTRY *MeglFogfv) (GLenum pname, const GLfloat *params);
void (APIENTRY *MeglFogi) (GLenum pname, GLint param);
void (APIENTRY *MeglFogiv) (GLenum pname, const GLint *params);
void (APIENTRY *MeglPolygonMode) (GLenum face, GLenum mode);
void (APIENTRY *MeglPolygonOffset) (GLfloat factor, GLfloat units);
void (APIENTRY *MeglShadeModel) (GLenum mode);
void (APIENTRY *MeglTexEnvf) (GLenum target, GLenum pname, GLfloat param);
void (APIENTRY *MeglTexEnvi) (GLenum target, GLenum pname, GLint param);
void (APIENTRY *MeglTexParameterf) (GLenum target, GLenum pname, GLfloat param);
void (APIENTRY *MeglTexParameteri) (GLenum target, GLenum pname, GLint param);

void mglPushStates (void);
void mglFinishedStates (void);
void mglPopStates (void);

#ifdef _DEBUG
void APIENTRY managed_eglAlphaFunc (GLenum func, GLclampf ref);
void APIENTRY managed_eglBlendFunc (GLenum sfactor, GLenum dfactor);
void APIENTRY managed_eglClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void APIENTRY managed_eglColor3f (GLfloat red, GLfloat green, GLfloat blue);
void APIENTRY managed_eglColor3fv (const GLfloat *v);
void APIENTRY managed_eglColor3ubv (const GLubyte *v);
void APIENTRY managed_eglColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void APIENTRY managed_eglColor4fv (const GLfloat *v);
void APIENTRY managed_eglColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
void APIENTRY managed_eglColor4ubv (const GLubyte *v);
void APIENTRY managed_eglColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void APIENTRY managed_eglCullFace (GLenum mode);
void APIENTRY managed_eglDepthFunc (GLenum func);
void APIENTRY managed_eglDepthMask (GLboolean flag);
void APIENTRY managed_eglDepthRange (GLclampd zNear, GLclampd zFar);
void APIENTRY managed_eglDisable (GLenum cap);
void APIENTRY managed_eglEnable (GLenum cap);
void APIENTRY managed_eglFogf (GLenum pname, GLfloat param);
void APIENTRY managed_eglFogfv (GLenum pname, const GLfloat *params);
void APIENTRY managed_eglFogi (GLenum pname, GLint param);
void APIENTRY managed_eglFogiv (GLenum pname, const GLint *params);
void APIENTRY managed_eglPolygonMode (GLenum face, GLenum mode);
void APIENTRY managed_eglPolygonOffset (GLfloat factor, GLfloat units);
void APIENTRY managed_eglShadeModel (GLenum mode);
void APIENTRY managed_eglTexEnvf (GLenum target, GLenum pname, GLfloat param);
void APIENTRY managed_eglTexEnvi (GLenum target, GLenum pname, GLint param);
void APIENTRY managed_eglTexParameterf (GLenum target, GLenum pname, GLfloat param);
void APIENTRY managed_eglTexParameteri (GLenum target, GLenum pname, GLint param);
#endif

#define RENDERER_OPENGL 1
#define RENDERER_DIRECT3D 2





renderer_com_t myRenderer;

//renderer_com_t *Renderer_Init (int RendererType);
void Renderer_AskInit (void);

/////////////////////////////////////////////////////////////////////////////


// Baker: Note that functions prefixed by "M" I don't allow in OpenGL or Direct3D to directly talk
//        to the function.  I need an equivalent of PushMatrix and PopMatrix like PushState and PopState
//        because trying to figure out where a single missing function reset should be
//        takes too much time to figure out.




// The function set provided by the wrapper
#ifdef __FILE_ALLOWED_FULL_EGL_FUNCTION_LIST__
void (APIENTRY *eglAlphaFunc) (GLenum func, GLclampf ref);
void (APIENTRY *eglBlendFunc) (GLenum sfactor, GLenum dfactor);
void (APIENTRY *eglClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void (APIENTRY *eglColor3f) (GLfloat red, GLfloat green, GLfloat blue);
void (APIENTRY *eglColor3fv) (const GLfloat *v);
void (APIENTRY *eglColor3ubv) (const GLubyte *v);
void (APIENTRY *eglColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void (APIENTRY *eglColor4fv) (const GLfloat *v);
void (APIENTRY *eglColor4ub) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
void (APIENTRY *eglColor4ubv) (const GLubyte *v);
void (APIENTRY *eglColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void (APIENTRY *eglCullFace) (GLenum mode);
void (APIENTRY *eglDepthFunc) (GLenum func);
void (APIENTRY *eglDepthMask) (GLboolean flag);
void (APIENTRY *eglDepthRange) (GLclampd zNear, GLclampd zFar);
void (APIENTRY *eglDisable) (GLenum cap);
void (APIENTRY *eglEnable) (GLenum cap);
void (APIENTRY *eglFogf) (GLenum pname, GLfloat param);
void (APIENTRY *eglFogfv) (GLenum pname, const GLfloat *params);
void (APIENTRY *eglFogi) (GLenum pname, GLint param);
void (APIENTRY *eglFogiv) (GLenum pname, const GLint *params);
void (APIENTRY *eglPolygonMode) (GLenum face, GLenum mode);
void (APIENTRY *eglPolygonOffset) (GLfloat factor, GLfloat units);
void (APIENTRY *eglShadeModel) (GLenum mode);
void (APIENTRY *eglTexEnvf) (GLenum target, GLenum pname, GLfloat param);
void (APIENTRY *eglTexEnvi) (GLenum target, GLenum pname, GLint param);
void (APIENTRY *eglTexParameterf) (GLenum target, GLenum pname, GLfloat param);
void (APIENTRY *eglTexParameteri) (GLenum target, GLenum pname, GLint param);
#endif

void (APIENTRY *eglBegin) (GLenum mode);
void (APIENTRY *eglBindTexture) (GLenum target, GLuint texture);
void (APIENTRY *eglClear) (GLbitfield mask);
void (APIENTRY *eglClearStencil) (GLint s);

// Baker: eglCopyTexSubImage2D is not implemented in wrapper.  I suspect because r_oldwater 0 in Fitz achieves what it is doing
// in a very strange way that would require an unreasonable amount of work and this function was only used there.  I doubt
// this function presented any kind of challenge.

void (APIENTRY *eglCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

void (APIENTRY *eglDeleteTextures) (GLsizei n, const GLuint *textures);
void (APIENTRY *eglDrawBuffer) (GLenum mode);
void (APIENTRY *eglEnd) (void);
void (APIENTRY *eglFinish) (void);
void (APIENTRY *eglFrontFace) (GLenum mode);
void (APIENTRY *eglFrustum) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void (APIENTRY *eglGenTextures) (GLsizei n, GLuint *textures);
void (APIENTRY *eglGetFloatv) (GLenum pname, GLfloat *params);
void (APIENTRY *eglGetIntegerv) (GLenum pname, GLint *params);
const GLubyte* (APIENTRY *eglGetString) (GLenum name);
void (APIENTRY *eglGetTexImage) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
void (APIENTRY *eglGetTexParameterfv) (GLenum target, GLenum pname, GLfloat *params);void (APIENTRY *eglHint) (GLenum target, GLenum mode);
void (APIENTRY *eglLineWidth) (GLfloat width); // Baker: does not exist in Direct3D wrapper (currently, maybe I'll write)
void (APIENTRY *eglLoadIdentity) (void);
void (APIENTRY *eglLoadMatrixf) (const GLfloat *m);
void (APIENTRY *eglMatrixMode) (GLenum mode);
void (APIENTRY *eglMultMatrixf) (const GLfloat *m);
void (APIENTRY *eglNormal3f) (GLfloat nx, GLfloat ny, GLfloat nz);
void (APIENTRY *eglOrtho) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void (APIENTRY *eglPopMatrix) (void);
void (APIENTRY *eglPushMatrix) (void);
void (APIENTRY *eglReadBuffer) (GLenum mode);
void (APIENTRY *eglReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
void (APIENTRY *eglRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void (APIENTRY *eglScalef) (GLfloat x, GLfloat y, GLfloat z);
void (APIENTRY *eglScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
void (APIENTRY *eglSelectBuffer) (GLsizei size, GLuint *buffer);
void (APIENTRY *eglStencilFunc) (GLenum func, GLint ref, GLuint mask);
void (APIENTRY *eglStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
void (APIENTRY *eglTexCoord2f) (GLfloat s, GLfloat t);
void (APIENTRY *eglTexCoord2fv) (const GLfloat *v);
void (APIENTRY *eglTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void (APIENTRY *eglTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void (APIENTRY *eglTranslatef) (GLfloat x, GLfloat y, GLfloat z);
void (APIENTRY *eglVertex2f) (GLfloat x, GLfloat y);
void (APIENTRY *eglVertex2fv) (const GLfloat *v);
void (APIENTRY *eglVertex3f) (GLfloat x, GLfloat y, GLfloat z);
void (APIENTRY *eglVertex3fv) (const GLfloat *v);
void (APIENTRY *eglViewport) (GLint x, GLint y, GLsizei width, GLsizei height);

#ifdef _WIN32 // These are silly on another operating system

LONG (WINAPI *eChangeDisplaySettings) (LPDEVMODE lpDevMode, DWORD dwflags);

HGLRC (WINAPI *ewglCreateContext) (HDC);
BOOL  (WINAPI *ewglDeleteContext) (HGLRC);
HGLRC (WINAPI *ewglGetCurrentContext) (VOID);
HDC   (WINAPI *ewglGetCurrentDC) (VOID);
PROC  (WINAPI *ewglGetProcAddress)(LPCSTR);
BOOL  (WINAPI *ewglMakeCurrent) (HDC, HGLRC);
BOOL  (WINAPI *eSetPixelFormat) (HDC, int, CONST PIXELFORMATDESCRIPTOR *);
#endif

#endif // __RENDERER_H__
