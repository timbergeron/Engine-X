/*

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// renderer.c -- Renderer neutral modular component

// Baker: Let this access all the function egls
#define __FILE_ALLOWED_FULL_EGL_FUNCTION_LIST__
#include "quakedef.h"
//#include "glquake.h"	// Baker: Needed
//#include "renderer.h"

void Renderer_D3D (void)
{
#if RENDERER_DIRECT3D_AVAILABLE
	eglAlphaFunc            = d3dmh_glAlphaFunc;
	eglBegin                = d3dmh_glBegin;
	eglBindTexture          = d3dmh_glBindTexture;
	eglBlendFunc            = d3dmh_glBlendFunc;
	eglClear                = d3dmh_glClear;
	eglClearColor           = d3dmh_glClearColor;
	eglClearStencil         = d3dmh_glClearStencil;
	eglColor3f              = d3dmh_glColor3f;
	eglColor3fv             = d3dmh_glColor3fv;
	eglColor3ubv            = d3dmh_glColor3ubv;
	eglColor4f              = d3dmh_glColor4f;
	eglColor4fv             = d3dmh_glColor4fv;
	eglColor4ub             = d3dmh_glColor4ub;
	eglColor4ubv            = d3dmh_glColor4ubv;
	eglColorMask            = d3dmh_glColorMask;

	eglCopyTexSubImage2D	= NULL;  // Not implemented in wrapper!  Should really create a Sys_Error function for it.

	eglCullFace             = d3dmh_glCullFace;
	eglDeleteTextures       = d3dmh_glDeleteTextures;
	eglDepthFunc            = d3dmh_glDepthFunc;
	eglDepthMask            = d3dmh_glDepthMask;
	eglDepthRange           = d3dmh_glDepthRange;
	eglDisable              = d3dmh_glDisable;
	eglDrawBuffer           = d3dmh_glDrawBuffer;
	eglEnable               = d3dmh_glEnable;
	eglEnd                  = d3dmh_glEnd;
	eglFinish               = d3dmh_glFinish;
	eglFogf                 = d3dmh_glFogf;
	eglFogfv                = d3dmh_glFogfv;
	eglFogi                 = d3dmh_glFogi;
	eglFogiv                = d3dmh_glFogiv;
	eglFrontFace            = d3dmh_glFrontFace;
	eglFrustum              = d3dmh_glFrustum;
	eglGenTextures          = d3dmh_glGenTextures;
	eglGetFloatv            = d3dmh_glGetFloatv;
	eglGetIntegerv          = d3dmh_glGetIntegerv;
	eglGetString            = d3dmh_glGetString;
	eglGetTexImage          = d3dmh_glGetTexImage;
	eglGetTexParameterfv    = d3dmh_glGetTexParameterfv;
	eglHint                 = d3dmh_glHint;
	eglLineWidth            = NULL;						// Baker: Doesn't exist in the wrapper, don't call it in code.
	eglLoadIdentity         = d3dmh_glLoadIdentity;
	eglLoadMatrixf          = d3dmh_glLoadMatrixf;
	eglMatrixMode           = d3dmh_glMatrixMode;
	eglMultMatrixf          = d3dmh_glMultMatrixf;
	eglNormal3f             = d3dmh_glNormal3f;
	eglOrtho                = d3dmh_glOrtho;
	eglPolygonMode          = d3dmh_glPolygonMode;
	eglPolygonOffset        = d3dmh_glPolygonOffset;
	eglPopMatrix            = d3dmh_glPopMatrix;
	eglPushMatrix           = d3dmh_glPushMatrix;
	eglReadBuffer           = d3dmh_glReadBuffer;
	eglReadPixels           = d3dmh_glReadPixels;
	eglRotatef              = d3dmh_glRotatef;
	eglScalef               = d3dmh_glScalef;
	eglScissor              = d3dmh_glScissor;			// Not used in this codebase, no implemented in wrapper
	eglShadeModel           = d3dmh_glShadeModel;
	eglStencilFunc          = d3dmh_glStencilFunc;
	eglStencilOp            = d3dmh_glStencilOp;
	eglTexCoord2f           = d3dmh_glTexCoord2f;
	eglTexCoord2fv          = d3dmh_glTexCoord2fv;
	eglTexEnvf              = d3dmh_glTexEnvf;
	eglTexEnvi              = d3dmh_glTexEnvi;
	eglTexImage2D           = d3dmh_glTexImage2D;		// Upload
	eglTexParameterf        = d3dmh_glTexParameterf;
	eglTexParameteri        = d3dmh_glTexParameteri;
	eglTexSubImage2D        = d3dmh_glTexSubImage2D;	// The glTexSubImage2D function specifies a portion of an existing one-dimensional texture image. You cannot define a new texture with glTexSubImage2D.


	eglTranslatef           = d3dmh_glTranslatef;		// The glTranslated and glTranslatef functions multiply the current matrix by a translation matrix.
	eglVertex2f             = d3dmh_glVertex2f;			// These functions specify a vertex.
	eglVertex2fv            = d3dmh_glVertex2fv;
	eglVertex3f             = d3dmh_glVertex3f;
	eglVertex3fv            = d3dmh_glVertex3fv;
	eglViewport             = d3dmh_glViewport;			// The glViewport function sets the viewport.




	ewglCreateContext       = d3dmh_wglCreateContext;
	ewglDeleteContext       = d3dmh_wglDeleteContext;
	ewglGetCurrentContext   = d3dmh_wglGetCurrentContext;
	ewglGetCurrentDC        = d3dmh_wglGetCurrentDC;
	ewglMakeCurrent         = d3dmh_wglMakeCurrent;
	ewglGetProcAddress		= d3dmh_wglGetProcAddress;

	eSetPixelFormat         = d3dmh_SetPixelFormat;

	eChangeDisplaySettings  = ChangeDisplaySettings_FakeGL;

	myRenderer.graphics_api	= RENDERER_DIRECT3D;
	strcpy (myRenderer.RendererText, "DX8");
#endif
}

void Renderer_OpenGL (void)
{

#if RENDERER_OPENGL_AVAILABLE

	eglAlphaFunc            = glAlphaFunc;
	eglBegin                = glBegin;
	eglBindTexture          = glBindTexture;
	eglBlendFunc            = glBlendFunc;
	eglClear                = glClear;
	eglClearColor           = glClearColor;
	eglClearStencil         = glClearStencil;
	eglColor3f              = glColor3f;
	eglColor3fv             = glColor3fv;
	eglColor3ubv            = glColor3ubv;
	eglColor4f              = glColor4f;
	eglColor4fv             = glColor4fv;
	eglColor4ub             = glColor4ub;
	eglColor4ubv            = glColor4ubv;
	eglColorMask            = glColorMask;

	eglCopyTexSubImage2D	= glCopyTexSubImage2D;

	eglCullFace             = glCullFace;
	eglDeleteTextures       = glDeleteTextures;
	eglDepthFunc            = glDepthFunc;
	eglDepthMask            = glDepthMask;
	eglDepthRange           = glDepthRange;
	eglDisable              = glDisable;
	eglDrawBuffer           = glDrawBuffer;
	eglEnable               = glEnable;
	eglEnd                  = glEnd;
	eglFinish               = glFinish;
	eglFogf                 = glFogf;
	eglFogfv                = glFogfv;
	eglFogi                 = glFogi;
	eglFogiv                = glFogiv;
	eglFrontFace            = glFrontFace;
	eglFrustum              = glFrustum;
	eglGenTextures          = glGenTextures;
	eglGetFloatv            = glGetFloatv;
	eglGetIntegerv          = glGetIntegerv;
	eglGetString            = glGetString;
	eglGetTexImage          = glGetTexImage;
	eglGetTexParameterfv    = glGetTexParameterfv;
	eglHint                 = glHint;
	eglLineWidth            = glLineWidth;
	eglLoadIdentity         = glLoadIdentity;
	eglLoadMatrixf          = glLoadMatrixf;
	eglMatrixMode           = glMatrixMode;
	eglMultMatrixf          = glMultMatrixf;
	eglNormal3f             = glNormal3f;
	eglOrtho                = glOrtho;
	eglPolygonMode          = glPolygonMode;
	eglPolygonOffset        = glPolygonOffset;
	eglPopMatrix            = glPopMatrix;
	eglPushMatrix           = glPushMatrix;
	eglReadBuffer           = glReadBuffer;
	eglReadPixels           = glReadPixels;
	eglRotatef              = glRotatef;
	eglScalef               = glScalef;
	eglScissor              = glScissor;
	eglShadeModel           = glShadeModel;
	eglStencilFunc          = glStencilFunc;
	eglStencilOp            = glStencilOp;
	eglTexCoord2f           = glTexCoord2f;
	eglTexCoord2fv          = glTexCoord2fv;
	eglTexEnvf              = glTexEnvf;
	eglTexEnvi              = glTexEnvi;
	eglTexImage2D           = glTexImage2D;
	eglTexParameterf        = glTexParameterf;
	eglTexParameteri        = glTexParameteri;
	eglTexSubImage2D        = glTexSubImage2D;
	eglTranslatef           = glTranslatef;
	eglVertex2f             = glVertex2f;
	eglVertex2fv            = glVertex2fv;
	eglVertex3f             = glVertex3f;
	eglVertex3fv            = glVertex3fv;
	eglViewport             = glViewport;
#ifdef _WIN32
	ewglCreateContext       = wglCreateContext;
	ewglDeleteContext       = wglDeleteContext;
	ewglGetCurrentContext   = wglGetCurrentContext;
	ewglGetCurrentDC        = wglGetCurrentDC;
	ewglMakeCurrent         = wglMakeCurrent;
	ewglGetProcAddress		= wglGetProcAddress;

	eSetPixelFormat         = SetPixelFormat;

	eChangeDisplaySettings  = ChangeDisplaySettings;
#endif

	myRenderer.graphics_api	= RENDERER_OPENGL;
	strcpy (myRenderer.RendererText, "GL");
#endif
}

void Renderer_WireUp_Managed_Functions (void)
{
	// Set Megls to egls if release
	// Set Megls to mgls if debug

#ifdef _DEBUG
	MeglAlphaFunc =  		managed_eglAlphaFunc;
	MeglBlendFunc =  		managed_eglBlendFunc;
	MeglClearColor =  		managed_eglClearColor;
	MeglColor3f =  			managed_eglColor3f;
	MeglColor3fv =  		managed_eglColor3fv;
	MeglColor3ubv =  		managed_eglColor3ubv;
	MeglColor4f =  			managed_eglColor4f;
	MeglColor4fv =  		managed_eglColor4fv;
	MeglColor4ub =  		managed_eglColor4ub;
	MeglColor4ubv =  		managed_eglColor4ubv;
	MeglColorMask =  		managed_eglColorMask;
	MeglCullFace =  		managed_eglCullFace;
	MeglDepthFunc =  		managed_eglDepthFunc;
	MeglDepthMask =  		managed_eglDepthMask;
	MeglDepthRange =  		managed_eglDepthRange;
	MeglDisable =  			managed_eglDisable;
	MeglEnable =  			managed_eglEnable;
	MeglFogf =  			managed_eglFogf;
	MeglFogfv =  			managed_eglFogfv;
	MeglFogi =  			managed_eglFogi;
	MeglFogiv =  			managed_eglFogiv;
	MeglPolygonMode =  		managed_eglPolygonMode;
	MeglPolygonOffset =  	managed_eglPolygonOffset;
	MeglShadeModel =  		managed_eglShadeModel;
	MeglTexEnvf =  			managed_eglTexEnvf;
	MeglTexEnvi =  			managed_eglTexEnvi;
	MeglTexParameterf =  	managed_eglTexParameterf;
	MeglTexParameteri =		managed_eglTexParameteri;
#else
	MeglAlphaFunc =  		eglAlphaFunc;
	MeglBlendFunc =  		eglBlendFunc;
	MeglClearColor =  		eglClearColor;
	MeglColor3f =  			eglColor3f;
	MeglColor3fv =  		eglColor3fv;
	MeglColor3ubv =  		eglColor3ubv;
	MeglColor4f =  			eglColor4f;
	MeglColor4fv =  		eglColor4fv;
	MeglColor4ub =  		eglColor4ub;
	MeglColor4ubv =  		eglColor4ubv;
	MeglColorMask =  		eglColorMask;
	MeglCullFace =  		eglCullFace;
	MeglDepthFunc =  		eglDepthFunc;
	MeglDepthMask =  		eglDepthMask;
	MeglDepthRange =  		eglDepthRange;
	MeglDisable =  			eglDisable;
	MeglEnable =  			eglEnable;
	MeglFogf =  			eglFogf;
	MeglFogfv =  			eglFogfv;
	MeglFogi =  			eglFogi;
	MeglFogiv =  			eglFogiv;
	MeglPolygonMode =  		eglPolygonMode;
	MeglPolygonOffset =  	eglPolygonOffset;
	MeglShadeModel =  		eglShadeModel;
	MeglTexEnvf =  			eglTexEnvf;
	MeglTexEnvi =  			eglTexEnvi;
	MeglTexParameterf =  	eglTexParameterf;
	MeglTexParameteri =		eglTexParameteri;
#endif

}

static renderer_com_t *sRenderer_Init (int RendererType)
{
	if (RendererType == RENDERER_OPENGL)
		Renderer_OpenGL ();
	else
		Renderer_D3D ();
	
	// Baker: Wire up the managed functions
	Renderer_WireUp_Managed_Functions ();

	myRenderer.initialized  = true;
	return &myRenderer;

}

void Renderer_AskInit (void)
{
#if RENDERER_DIRECT3D_AVAILABLE && RENDERER_OPENGL_AVAILABLE
		
	if (COM_CheckParm ("-opengl"))
		engine.Renderer = sRenderer_Init (RENDERER_OPENGL);
	else if (COM_CheckParm ("-direct3d"))
		engine.Renderer = sRenderer_Init (RENDERER_DIRECT3D);
	else // Ask
	{
		switch (MessageBox(NULL, "Do you want to use Direct3D as the renderer?\n\nSelecting No will use OpenGL", "Select Renderer", MB_YESNOCANCEL))
		{
		case IDYES:		engine.Renderer = sRenderer_Init (RENDERER_DIRECT3D); break;
		case IDNO:		engine.Renderer = sRenderer_Init (RENDERER_OPENGL);   break;
		case IDCANCEL:
		default:
						Sys_Quit ();
		}
	}
	session_startup_time = Sys_DoubleTime (); // Don't let dialog box count against startup time

#else
#if RENDERER_DIRECT3D_AVAILABLE
	engine.Renderer = sRenderer_Init (RENDERER_DIRECT3D);
#else
	engine.Renderer = sRenderer_Init (RENDERER_OPENGL);
#endif
#endif

}
