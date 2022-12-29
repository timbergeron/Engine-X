/*
Copyright (C) 2002-2003 A Nourai

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
// vid_common_gl.c -- Common code for vid_wgl.c and vid_glx.c

#include "quakedef.h"

#ifdef _WIN32 // ProcAddress
#define qglGetProcAddress ewglGetProcAddress
#else
#define qglGetProcAddress Sys_GetProcAddress // OSX
// Linux and friends
//#define qglGetProcAddress glXGetProcAddressARB
#endif


const char		*gl_vendor;
const char		*gl_renderer;
const char		*gl_version;
const char		*gl_extensions;
qbool			gl_mtexable = false;
int				gl_textureunits = 1;
qbool			gl_add_ext = false;

lpMTexFUNC		qglMultiTexCoord2f = NULL;
lpSelTexFUNC	qglActiveTexture = NULL;


// Color constants

byte		color_white[4] =	{255, 255, 255, 0};
byte		color_black[4] =	{0, 0, 0, 0};
byte		color_orange[4] =	{255, 127, 0, 0};
byte		color_red[4] =		{255, 0, 0, 0};
byte		color_green[4] =	{0, 255, 0, 0};
byte		color_blue[4] =		{0, 0, 255, 0};
byte		color_yellow[4] =	{255, 255, 0, 0};
byte		color_gray[4] =		{127, 127, 127, 0};

float		color_white_f[4] =	{1.0, 1.0, 1.0, 0.0};
float		color_black_f[4] =	{0.0, 0.0, 0.0, 0.0};


void GL_PrintExtensions_f(void)
{
#ifdef DO_WGL
	extern const char *wgl_extensions;
#endif
	Con_Printf ("GL_EXTENSIONS: %s\n", gl_extensions);
#ifdef DO_WGL
	Con_Printf ("WGL_EXTENSIONS: %s\n", wgl_extensions);
#endif

}


static qbool CheckExtension (const char *extension)
{
	char		*where, *terminator;
	const	char	*start;

	if (!gl_extensions && !(gl_extensions = eglGetString(GL_EXTENSIONS)))
		return false;

	if (!extension || *extension == 0 || strchr(extension, ' '))
		return false;

	for (start = gl_extensions ; where = strstr(start, extension) ; start = terminator)
	{
		terminator = where + strlen(extension);
		if ((where == start || *(where - 1) == ' ') && (*terminator == 0 || *terminator == ' '))
			return true;
	}

	return false;
}

#ifdef DO_WGL
static qbool CheckWGLExtension (const char *extension)
{
	extern const char *wgl_extensions;
	char		*where, *terminator;
	const	char	*start;

	if (!wgl_extensions || wgl_extensions[0] == 0)
		return false;

	if (!extension || *extension == 0 || strchr(extension, ' '))
		return false;

	for (start = wgl_extensions ; where = strstr(start, extension) ; start = terminator)
	{
		terminator = where + strlen(extension);
		if ((where == start || *(where - 1) == ' ') && (*terminator == 0 || *terminator == ' '))
			return true;
	}

	return false;
}
#endif


qbool vid_gl_vsync_ext_exists = false;
void CheckVsyncControlExtensions (void)
{
#ifdef _WIN32 // Bakerdanger .. Fix this
	if (engine.Renderer->graphics_api ==  RENDERER_DIRECT3D)
	{
		return; // This isn't how we do it in Direct3D
	}

//	Cvar_RegisterVariable (&vid_vsync, NULL);			// Baker: register it anyway

	if (COM_CheckParm("-noswapctrl"))
	{
		Con_Warning ("Vertical sync control disabled via command line\n");
		return;
	}

	if (!CheckExtension("WGL_EXT_swap_control") && !CheckExtension("GL_WIN_swap_hint")
#ifdef DO_WGL
		&& !CheckWGLExtension("WGL_EXT_swap_control")
#endif
		) // Baker: Add GL_WIN_swap_hint
	{
		Con_Warning ("Vertical sync control disabled, extension not found\n");
		return;
	}

	if (!(wglSwapIntervalEXT = (void *)ewglGetProcAddress("wglSwapIntervalEXT")))
	{
		Con_Warning ("Vertical sync control disabled, couldn't get procedure address wglSwapIntervalEXT\n");
		return;
	}

	vid_gl_vsync_ext_exists = true;
	Cvar_KickOnChange (&vid_vsync);
	Con_Success ("Vertical sync control extensions found\n");
#endif
}


static void CheckMultiTextureExtensions (void)
{
	// Default

	gl_mtexable = false;
	gl_textureunits = 1;

	gl_add_ext = false;

	if (COM_CheckParm("-nomtex"))
	{
		Con_Warning ("Multitexture disabled via command line\n");
		return;
	}

	if (!CheckExtension("GL_ARB_multitexture"))
	{
		Con_Warning ("Multitexture extension not found\n");
		return;
	}

	if (strstr(gl_renderer, "Savage"))
	{
		Con_Warning ("Multitexture disabled due to lack of vendor support (Savage)\n");
		return;
	}

	// Wire up our functions

	qglMultiTexCoord2f = (void *)qglGetProcAddress ("glMultiTexCoord2fARB");
	qglActiveTexture = (void *)qglGetProcAddress ("glActiveTextureARB");

	if (!qglMultiTexCoord2f || !qglActiveTexture)
	{
		Con_Warning ("Multitexture extensions; couldn't get procedure address\n");
		return;
	}

	gl_mtexable = true;
	Con_Success ("Multitexture extensions found\n");

	// Get texture units from video card

	eglGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &gl_textureunits);
	gl_textureunits = min(gl_textureunits, 4);

	if (COM_CheckParm("-maxtmu2"))
	{
		gl_textureunits = 2; // FORCE! //min(gl_textureunits, 2);
		Con_Warning ("Max texture units on hardware set to 2 via command line\n");
		goto tmuforced;
	}

	if (COM_StringMatch (gl_vendor, "ATI Technologies Inc."))
	{
		gl_textureunits = min(gl_textureunits, 2);
		Con_Warning ("Max texture units on hardware set to 2 due to vendor (ATI)\n");
	}

	if (gl_textureunits < 2)
	{
		gl_textureunits = 1;
		gl_mtexable = false;
		Con_Warning ("Multitexture disabled due to max texture units <2\n");
		return;
	}

tmuforced:
	Con_Success ("Enabled %i texture units on hardware\n", gl_textureunits);
}

/*
===============
GL_SetupState -- johnfitz

does all the stuff from GL_Init that needs to be done every time a new GL render context is created
GL_Init will still do the stuff that only needs to be done once
===============
*/
void GL_SetupState (void)
{
	MeglClearColor (0, 0, 0, 0);
	MeglCullFace (GL_FRONT);
	MeglEnable (GL_TEXTURE_2D);
	MeglEnable (GL_ALPHA_TEST);		// Has this always been here?
	MeglAlphaFunc (GL_GREATER, 0.666);

	// Get rid of Z-fighting for textures by offsetting the drawing of entity models compared to normal polygons.
	// Only works if gl_ztrick is turned off, but we've dropped support for that.

	MeglPolygonOffset(gl_polygonoffset_factor.floater, gl_polygonoffset_offset.floater);	// D3D wrapper does not support

	MeglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	MeglShadeModel (GL_FLAT);
	MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	MeglDepthRange (0, 1); //johnfitz -- moved here becuase gl_ztrick is gone.
	MeglDepthFunc (GL_LEQUAL); //johnfitz -- moved here becuase gl_ztrick is gone.

}

#ifdef DO_WGL
const char *wgl_extensions; //johnfitz
void GetWGLExtensions (void)
{
	extern HDC		maindc;
	const char *(*wglGetExtensionsStringARB) (HDC hdc);
	const char *(*wglGetExtensionsStringEXT) ();

	if (wglGetExtensionsStringARB = (void *) ewglGetProcAddress ("wglGetExtensionsStringARB"))
		wgl_extensions = wglGetExtensionsStringARB (maindc);
	else if (wglGetExtensionsStringEXT = (void *) ewglGetProcAddress ("wglGetExtensionsStringEXT"))
		wgl_extensions = wglGetExtensionsStringEXT ();
	else
		wgl_extensions = "";
}
#endif

/*
===============
GL_Init
===============
*/
void GL_Init (void)
{
	// Initialization gets GL information, extensions
	gl_vendor = eglGetString (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);

	gl_renderer = eglGetString (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);

	gl_version = eglGetString (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gl_version);

	gl_extensions = eglGetString (GL_EXTENSIONS);

#ifdef DO_WGL
	GetWGLExtensions ();
#endif

//	if (!strncasecmp((char *)gl_renderer, "PowerVR", 7))
//		fullsbardraw = true;

	CheckMultiTextureExtensions ();

	gl_add_ext = CheckExtension ("GL_ARB_texture_env_add");

	eglGetIntegerv (GL_MAX_TEXTURE_SIZE, &gl_max_size_default);
	if (gl_max_size_default > 1024) gl_max_size_default = 1024; // 1024 is plenty big

//#if 0
//	Cvar_SetDefaultFloatByRef (&gl_max_size, gl_max_size_default);
//
//	// 3dfx can only handle 256 wide textures
//	if (!strncasecmp((char *)gl_renderer, "3dfx", 4) || strstr((char *)gl_renderer, "Glide"))
//		Cvar_SetDefaultFloatByRef (&gl_max_size, 256);
//#endif


	// Baker: we should not be doing this here ....
	GL_SetupState (); //johnfitz
}



