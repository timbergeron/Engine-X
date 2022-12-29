/*
Copyright (C) 1996-1997 Id Software, Inc.

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
// gl_manager.c -- To make it so we can keep track of what crap we have enabled


#define __FILE_ALLOWED_FULL_EGL_FUNCTION_LIST__
#include "quakedef.h"


#ifndef _DEBUG

void mglPushStates (void) {}
void mglFinishedStates (void) {}
void mglPopStates (void) {}
#else

// I want the egls to always be used

// If debug,   the Megls hook to the megls
// If release, the Megls hook to the egls

// The main code isn't aware of egls for Megls

static pushlevel = 0;
void mglPushStates (void)
{
	// Signals to initiate a new state
	
	pushlevel ++;

}


void mglFinishedStates (void)
{
	// Signals to that new state is now defined

	// Do nothing for now
}


void mglPopStates (void)
{
	// Signals to restore to previous state (usually the "base state)
	pushlevel --;

	// Level 0: Renderer
	// Level 1: Frame 
	// Level 2: Viewport or canvas (later)
	// Level 2: In model, particle, graphic or draw func of some sort

	// Do something about fog.  Make it part of the frame?

//	if (pushlevel > 2)
//		Con_Printf ("Drawing function did not reset everything\n");
}


/////

void MglDefineRendererDefaultState (void)
{
	// Do this at OpenGL startup before anything else
	// Maybe some GL get ints or something

}

void MglBegin_Define_BaseState (void)
{
	// May do this at GLSetup
	// But in reality, this should be done in R_SetupGL


}

void MglEnd_Define_BaseState (void)
{
	// May do this at GLSetup
	// But in reality, this should be done in R_SetupGL

}


// gl_manager.c

void APIENTRY managed_eglAlphaFunc (GLenum func, GLclampf ref)
{

	
	
	
	// Pass to renderer
	eglAlphaFunc (func, ref);
}


void APIENTRY managed_eglBlendFunc (GLenum sfactor, GLenum dfactor)
{

	// Pass to renderer
	eglBlendFunc (sfactor, dfactor);
}


void APIENTRY managed_eglClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{

	// Pass to renderer
	eglClearColor (red, green, blue, alpha);
}





void APIENTRY managed_eglColor3f (GLfloat red, GLfloat green, GLfloat blue)
{

	// Pass to renderer
	eglColor3f (red, green, blue);
}


void APIENTRY managed_eglColor3fv (const GLfloat *v)
{

	// Pass to renderer
	eglColor3fv (v);
}


void APIENTRY managed_eglColor3ubv (const GLubyte *v)
{

	// Pass to renderer
	eglColor3ubv (v);
}


void APIENTRY managed_eglColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{

	// Pass to renderer
	eglColor4f (red, green, blue, alpha);
}


void APIENTRY managed_eglColor4fv (const GLfloat *v)
{

	// Pass to renderer
	eglColor4fv (v);
}


void APIENTRY managed_eglColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{

	// Pass to renderer
	eglColor4ub (red, green, blue, alpha);
}

void APIENTRY managed_eglColor4ubv (const GLubyte *v)
{

	// Pass to renderer
	eglColor4ubv (v);
}


void APIENTRY managed_eglColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{

	// Pass to renderer
	eglColorMask (red, green, blue, alpha);
}


void APIENTRY managed_eglCullFace (GLenum mode)
{

	// Pass to renderer
	eglCullFace (mode);
}


void APIENTRY managed_eglDepthFunc (GLenum func)
{

	// Pass to renderer
	eglDepthFunc (func);
}


void APIENTRY managed_eglDepthMask (GLboolean flag)
{

	// Pass to renderer
	eglDepthMask (flag);
}


void APIENTRY managed_eglDepthRange (GLclampd zNear, GLclampd zFar)
{

	// Pass to renderer
	eglDepthRange (zNear, zFar);
}


void APIENTRY managed_eglDisable (GLenum cap)
{

	// Pass to renderer
	eglDisable (cap);
}


void APIENTRY managed_eglEnable (GLenum cap)
{

	// Pass to renderer
	eglEnable (cap);
}


void APIENTRY managed_eglFogf (GLenum pname, GLfloat param)
{

	// Pass to renderer
	eglFogf (pname, param);
}


void APIENTRY managed_eglFogfv (GLenum pname, const GLfloat *params)
{

	// Pass to renderer
	eglFogfv (pname, params);
}


void APIENTRY managed_eglFogi (GLenum pname, GLint param)
{

	// Pass to renderer
	eglFogi (pname, param);
}


void APIENTRY managed_eglFogiv (GLenum pname, const GLint *params)
{

	// Pass to renderer
	eglFogiv (pname, params);
}


void APIENTRY managed_eglPolygonMode (GLenum face, GLenum mode)
{

	// Pass to renderer
	eglPolygonMode (face, mode);
}


void APIENTRY managed_eglPolygonOffset (GLfloat factor, GLfloat units)
{

	// Pass to renderer
	eglPolygonOffset (factor, units);
}


void APIENTRY managed_eglShadeModel (GLenum mode)
{

	// Pass to renderer
	eglShadeModel (mode);
}


void APIENTRY managed_eglTexEnvf (GLenum target, GLenum pname, GLfloat param)
{

	// Pass to renderer
	eglTexEnvf (target, pname, param);
}


void APIENTRY managed_eglTexEnvi (GLenum target, GLenum pname, GLint param)
{

	// Pass to renderer
	eglTexEnvi (target, pname, param);
}


void APIENTRY managed_eglTexParameterf (GLenum target, GLenum pname, GLfloat param)
{

	// Pass to renderer
	eglTexParameterf (target, pname, param);
}


void APIENTRY managed_eglTexParameteri (GLenum target, GLenum pname, GLint param)
{

	// Pass to renderer
	eglTexParameteri (target, pname, param);
}








// GL_SetupState  ...	once per OpenGL initialization
// R_SetupGL ........	once per frame


// Baker:				We may need to find out the default values of some of this stuff

/*
We can do

#ifdef _DEBUG
#define DeglClear eglClear or something
#endif

To eliminate debug cruft during compile
*/

/*

YES		eglAlphaFunc            = d3dmh_glAlphaFunc;
NO		eglBegin                = d3dmh_glBegin;
NO		eglBindTexture          = d3dmh_glBindTexture;
YES		eglBlendFunc            = d3dmh_glBlendFunc;
NO		eglClear                = d3dmh_glClear;
KINDA	eglClearColor           = d3dmh_glClearColor;
UNUSED	eglClearStencil         = d3dmh_glClearStencil;
YES		eglColor3f              = d3dmh_glColor3f;
YES		eglColor3fv             = d3dmh_glColor3fv;
YES		eglColor3ubv            = d3dmh_glColor3ubv;
YES		eglColor4f              = d3dmh_glColor4f;
YES		eglColor4fv             = d3dmh_glColor4fv;
YES		eglColor4ub             = d3dmh_glColor4ub;
YES		eglColor4ubv            = d3dmh_glColor4ubv;
KINDA	eglColorMask            = d3dmh_glColorMask;

NO	eglCopyTexSubImage2D	= NULL;  // Not implemented in wrapper!  Should really create a Sys_Error function for it.

YES	eglCullFace             = d3dmh_glCullFace;
NO	eglDeleteTextures       = d3dmh_glDeleteTextures;
YES	eglDepthFunc            = d3dmh_glDepthFunc;
YES	eglDepthMask            = d3dmh_glDepthMask;
YES	eglDepthRange           = d3dmh_glDepthRange;
EGADS	eglDisable              = d3dmh_glDisable;
UNUSED	eglDrawBuffer           = d3dmh_glDrawBuffer;
EGADS	eglEnable               = d3dmh_glEnable;
NO	eglEnd                  = d3dmh_glEnd;
NO	eglFinish               = d3dmh_glFinish;
YES	eglFogf                 = d3dmh_glFogf;
YES	eglFogfv                = d3dmh_glFogfv;
YES	eglFogi                 = d3dmh_glFogi;
YES	eglFogiv                = d3dmh_glFogiv;
UNUSED	eglFrontFace            = d3dmh_glFrontFace;
NO	eglFrustum              = d3dmh_glFrustum;
UNUSED	eglGenTextures          = d3dmh_glGenTextures;
NO	eglGetFloatv            = d3dmh_glGetFloatv;
NO	eglGetIntegerv          = d3dmh_glGetIntegerv;
NO	eglGetString            = d3dmh_glGetString;
NO	eglGetTexImage          = d3dmh_glGetTexImage;
No	eglGetTexParameterfv    = d3dmh_glGetTexParameterfv;
YES	eglHint                 = d3dmh_glHint;
KINDA	eglLineWidth            = NULL;						// Baker: Doesn't exist in the wrapper, don't call it in code.
NO	eglLoadIdentity         = d3dmh_glLoadIdentity;
NO	eglLoadMatrixf          = d3dmh_glLoadMatrixf;
NO	eglMatrixMode           = d3dmh_glMatrixMode;
UNUSED	eglMultMatrixf          = d3dmh_glMultMatrixf;
UNUSED	eglNormal3f             = d3dmh_glNormal3f;
NO	eglOrtho                = d3dmh_glOrtho;
KINDA	eglPolygonMode          = d3dmh_glPolygonMode;
KINDA	eglPolygonOffset        = d3dmh_glPolygonOffset;
NO	eglPopMatrix            = d3dmh_glPopMatrix;
NO	eglPushMatrix           = d3dmh_glPushMatrix;
NO	eglReadBuffer           = d3dmh_glReadBuffer;
NO	eglReadPixels           = d3dmh_glReadPixels;
NO	eglRotatef              = d3dmh_glRotatef;
NO	eglScalef               = d3dmh_glScalef;
NO/UNUSED/UNIMPLEMENTED	eglScissor              = d3dmh_glScissor;			// Not used in this codebase, no implemented in wrapper
KINDA	eglShadeModel           = d3dmh_glShadeModel;
UNUSED	eglStencilFunc          = d3dmh_glStencilFunc;
UNUSED	eglStencilOp            = d3dmh_glStencilOp;
NO	eglTexCoord2f           = d3dmh_glTexCoord2f;
NO	eglTexCoord2fv          = d3dmh_glTexCoord2fv;
YES	eglTexEnvf              = d3dmh_glTexEnvf;
YES	eglTexEnvi              = d3dmh_glTexEnvi;
NO	eglTexImage2D           = d3dmh_glTexImage2D;		// Upload
YES	eglTexParameterf        = d3dmh_glTexParameterf;
UNUSED	eglTexParameteri        = d3dmh_glTexParameteri;
NO	eglTexSubImage2D        = d3dmh_glTexSubImage2D;	// The glTexSubImage2D function specifies a portion of an existing one-dimensional texture image. You cannot define a new texture with glTexSubImage2D.


NO	eglTranslatef           = d3dmh_glTranslatef;		// The glTranslated and glTranslatef functions multiply the current matrix by a translation matrix.
NO	eglVertex2f             = d3dmh_glVertex2f;			// These functions specify a vertex.
NO	eglVertex2fv            = d3dmh_glVertex2fv;
NO	eglVertex3f             = d3dmh_glVertex3f;
NO	eglVertex3fv            = d3dmh_glVertex3fv;
NO	eglViewport             = d3dmh_glViewport;			// The glViewport function sets the viewport.

*/

/*
YES		eglAlphaFunc            = d3dmh_glAlphaFunc; // GL_ALWAYS Always passes. This is the default. 
YES		eglBlendFunc            = d3dmh_glBlendFunc; // GL_ONE/GLONE Docs doesn't seem to state the default value? Quake uses GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
YES		eglColor3f              = d3dmh_glColor3f;
YES		eglColor3fv             = d3dmh_glColor3fv;
YES		eglColor3ubv            = d3dmh_glColor3ubv;
YES		eglColor4f              = d3dmh_glColor4f;
YES		eglColor4fv             = d3dmh_glColor4fv;
YES		eglColor4ub             = d3dmh_glColor4ub;
YES		eglColor4ubv            = d3dmh_glColor4ubv;
YES	eglCullFace             = d3dmh_glCullFace;
YES	eglDepthFunc            = d3dmh_glDepthFunc;
YES	eglDepthMask            = d3dmh_glDepthMask;
YES	eglDepthRange           = d3dmh_glDepthRange;
YES	eglFogf                 = d3dmh_glFogf;
YES	eglFogfv                = d3dmh_glFogfv;
YES	eglFogi                 = d3dmh_glFogi;
YES	eglFogiv                = d3dmh_glFogiv;
YES	eglHint                 = d3dmh_glHint;
YES	eglTexEnvf              = d3dmh_glTexEnvf;
YES	eglTexEnvi              = d3dmh_glTexEnvi;
YES	eglTexParameterf        = d3dmh_glTexParameterf;  GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER, GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T
KINDA	eglShadeModel           = d3dmh_glShadeModel; (Flat or smooth)		// The default is GL_SMOOTH

done ..EGADS	eglDisable              = d3dmh_glDisable;
done ...EGADS	eglEnable               = d3dmh_glEnable;

 eglEnable (GL_ALPHA_TEST);
eglEnable (GL_BLEND);
eglEnable (GL_CULL_FACE);
eglEnable (GL_DEPTH_TEST);
eglEnable (GL_FOG);
eglEnable (GL_LINE_SMOOTH);
eglEnable (GL_POLYGON_OFFSET_FILL);
eglEnable (GL_POLYGON_OFFSET_LINE);
eglEnable (GL_POLYGON_SMOOTH);
eglEnable (GL_TEXTURE_2D);

  TexEnvf GL_DECAL, GL_REPLACE, "GL_ADD", GL_MODULATE, GL_BLEND 

*/
#if 0
typedef struct opengl_state_s
{
	// Capabilities
	qbool		cap_GL_ALPHA_TEST;
	qbool		cap_GL_BLEND;
	qbool		cap_GL_CULLFACE;
	qbool		cap_GL_DEPTH_TEST;
	qbool		cap_GL_FOG;
	qbool		cap_GL_LINE_SMOOTH;
	qbool		cap_GL_POLYGON_OFFSET_FILL;
	qbool		cap_GL_POLYGON_OFFSET_LINE;
	qbool		cap_GL_POLYGON_SMOOTH;
	qbool		cap_GL_TEXTURE_2D;

	// Alpha func

	GLenum		glAlphaFunc;				// GL_ALWAYS
	GLclampf	glAlphaFunc_ref;			// 0

	// Blend func

	GLenum		glBlendFunc_sfactor;		// GL_ONE
	GLenum		glBlendFunc_dfactor;		// GL_ONE

	// Color

	GLfloat		RGBA[3];					// defaults are probably 1,1,1,1

	// CullFace 

	GLenum		glCullFace;					// GL_BACK (default), GL_FRONT, GL_FRONT_AND_BACK 

	// DepthFunc

	GLenum		glDepthFunc;				// GL_ALWAYS default
		
	// GL_ALWAYS
	GLboolean	glDepthMask;				// Initially on

	// glDepthRange

	GLclampd	glDepthRange_znear;			// Default 0
	GLclampd	glDepthRange_zfar;			// Default 1

	// Fog

	GLfloat		glFog_GL_FOG_MODE;			//	GL_LINEAR, GL_EXP, and GL_EXP2.  Default is GL_EXP
	GLfloat		glFog_FOG_DENSITY;			//  The default fog density is 1.0
	GLfloat		glFog_FOG_START				//	Default 0.0
	GLfloat		glFog_FOG_END				//	Default 1.0
	GLfloat		glFog_FOG_INDEX				//	Default 0.0
	GLfloat		glFog_Color[3]				//	Probably defaults 1,1,1,1

// Baker: Nah.  Oldy cvar that defaults to off.  Skip it.
//	GLbool		glHint_PERSPECTIVE_CORR;	//  GL_DONT_CARE or GL_FASTEST (default is 

	Glenum		GL_TEXTURE_ENV_MODE			// GL_REPLACE does not appear to be valid (but all engines use it!)?  FitzQuake never used GL_BLEND
	// GL_REPLACE, GL_ADD, GL_BLEND, GL_DECAL, GL_MODULATE and some other functions that FitzQuake uses
	
	
	YES	eglTexEnvf              = d3dmh_glTexEnvf;

	//

	// Not worth it.
	//eglTexParameterf        = d3dmh_glTexParameterf;  GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER, GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T

	// GL_FLAT or GL_SMOOTH (<-default value)
	GLenum mode	eglShadeModel           = d3dmh_glShadeModel; (Flat or smooth)		// The default is GL_SMOOTH





//	GL_NEVER Never passes.  
//  GL_LESS Passes if the incoming alpha value is less than the reference value.  
//  GL_EQUAL Passes if the incoming alpha value is equal to the reference value.  
//  GL_LEQUAL Passes if the incoming alpha value is less than or equal to the reference value.  
//  GL_GREATER  Passes if the incoming alpha value is greater than the reference value.  
//  GL_NOTEQUAL Passes if the incoming alpha value is not equal to the reference value.  
//  GL_GEQUAL Passes if the incoming alpha value is greater than or equal to the reference value.  
//  GL_ALWAYS 

	

#endif
#endif