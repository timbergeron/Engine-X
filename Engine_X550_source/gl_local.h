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
// gl_local.h -- private refresh defs

#ifndef __GL_LOCAL_H__
#define __GL_LOCAL_H__

// disable data conversion warnings

#pragma warning (disable : 4244)     // MIPS
#pragma warning (disable : 4136)     // X86
#pragma warning (disable : 4051)     // ALPHA

#ifdef _WIN32 // Headers, in this case windows.h
#include <windows.h>
#endif



// Baker start
// Baker: we want to include gl stuff
// *unless* doing a Direct3D only build
// which is mostly to test stuff out

#ifdef MACOSX
#include 	<OpenGL/gl.h>
#include 	<OpenGL/glu.h>
#include	<OpenGL/glext.h>
#include	<math.h>
#endif

#ifdef _WIN32
#ifndef DIRECT3D_ONLY
#include <gl/gl.h>
#include <GL/glu.h>
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glu32.lib")
#endif
#endif

// We want the wrapper *unless* specifically
// doing an open GL only build
#ifndef OPENGL_ONLY
#include "dx8_fakegl.h"
#endif

#ifdef OPENGL_ONLY
#define RENDERER_DIRECT3D_AVAILABLE 0
static void D3D_ResetMode (int width, int height, int bpp, int *Window_X, int *Window_Y, int windowed_, const int DesktopWidth, const int DesktopHeight) {}; // Code unreachable in this cirumstance anyway
static void FakeSwapBuffers (void) {};				// Code unreachable in this cirumstance anyway

#else
#define RENDERER_DIRECT3D_AVAILABLE 1
#endif

#ifdef DIRECT3D_ONLY
#define RENDERER_OPENGL_AVAILABLE 0
#else
#define RENDERER_OPENGL_AVAILABLE 1
#endif
// Baker end

#ifndef APIENTRY	// MACOSX for sure
#define APIENTRY
#endif

#include "renderer.h"




#include "gl_texture.h"


void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);

typedef struct {
	float	x, y, z;
	float	s, t;
	float	r, g, b;
} glvert_t;

extern	glvert_t	glv;

extern	int	glx, gly, glwidth, glheight;


//#define SKYSHIFT		7
//#define	SKYSIZE			(1 << SKYSHIFT)
//#define SKYMASK			(SKYSIZE - 1)





void QMB_InitParticles (void);
void QMB_ClearParticles (void);
void QMB_DrawParticles (void);

void QMB_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count);
void QMB_RocketTrail (vec3_t start, vec3_t end, vec3_t *trail_origin, trail_type_t type);
void QMB_BlobExplosion (vec3_t org);
void QMB_ParticleExplosion (vec3_t org);
void QMB_LavaSplash (vec3_t org);
void QMB_TeleportSplash (vec3_t org);
void QMB_InfernoFlame (vec3_t org);
void QMB_StaticBubble (entity_t *ent);
void QMB_ColorMappedExplosion (vec3_t org, int colorStart, int colorLength);
void QMB_TorchFlame (vec3_t org, float size, float time);
void QMB_MissileFire (vec3_t org, vec3_t start, vec3_t end);
void QMB_ShamblerCharge (vec3_t org);
void QMB_LightningBeam (vec3_t start, vec3_t end);
void QMB_GenSparks (vec3_t org, byte col[3], float count, float size, float life);

extern	qbool	qmb_initialized;

void CheckParticles (void);

//====================================================


//extern	vec3_t		modelorg; //, r_entorigin;
//extern	vec3_t		modelorg;

//extern	entity_t	*the_currententity;


extern	int			r_visframecount;
extern	int			r_framecount;
extern	mplane_t	frustum[4];
extern	int			c_brush_polys, c_alias_polys, c_md3_polys;

// view origin
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

// screen size info
extern	refdef_t	r_refdef;
extern	mleaf_t		*r_viewleaf, *r_oldviewleaf;
extern	mleaf_t		*r_viewleaf2, *r_oldviewleaf2;	// for watervis hack



extern	float	r_world_matrix[16];

extern	const char *gl_vendor;
extern	const char *gl_renderer;
extern	const char *gl_version;
extern	const char *gl_extensions;



// gl_draw.c
void GL_Set2D (void);

// gl_rmain.c
qbool R_CullBox (const vec3_t mins, const vec3_t maxs);
qbool R_CullSphere (const vec3_t centre, const float radius);
qbool R_CullForEntity (const entity_t *ent/*, vec3_t returned_center*/);

void R_RotateForEntity (vec3_t origin, vec3_t angles);
//void R_PolyBlend (void);
void R_BrightenScreen (void);




// gl_scene_fog.c
void Fog_NewMap (void);
void Fog_Init (void);
void Fog_SetupFrame (void);
//void Fog_Waterfog_SetupFrame (int contents);
void Fog_ParseServerMessage (void);
void Fog_DisableGFog (void);
void Fog_EnableGFog (void);



qbool SetFlameModelState (entity_t *CurModel);

// gl_palette_gamma.c

extern	byte	vid_palette_gamma_table[256];
extern	float	vid_palette_gamma;

void Palette_Check_Gamma (unsigned char *pal);
void Palette_Apply_Texture_Gamma_To_RGBA_Pixels (byte *data, const int imagesize); // Baker: This is for texture gamma (not normally used)





// gl_rmisc.c
//void R_TimeRefresh_f (void);
void R_ReadPointFile_f (void);
void R_InitOtherTextures (void);
//#define ISTRANSPARENT(ent)	((ent)->istransparent && (ent)->transparency > 0 && (ent)->transparency < 1)
#define ISTRANSPARENT(ent) ((ent)->alpha > 1) // back2forwards

// gl_rpart.c

//vid_common_gl.c


extern qbool	gl_add_ext;
//extern qbool gl_allow_ztrick;

extern lpMTexFUNC qglMultiTexCoord2f;
extern lpSelTexFUNC qglActiveTexture;

#ifndef MACOSX	// Windows video
typedef BOOL (APIENTRY *SWAPINTERVALFUNCPTR)(int);
extern SWAPINTERVALFUNCPTR wglSwapIntervalEXT;
#endif

void GL_SetupState (void);
//qbool CheckExtension (const char *extension);



//palette stuffs
void Check_Gamma (unsigned char *pal);
void GL_Init (void);

//extern	float	gldepthmin, gldepthmax;
extern	byte	color_white[4], color_black[4], color_orange[4];


extern	vec_rgba_t color_white_f;
extern	vec_rgba_t color_black_f;

void GL_PrintExtensions_f(void);

// Stack of stuffs

void Frustum_ViewSetup_SetFrustum (const float fovx, const float fovy);
void Frustum_ViewSetup_MarkVisibleLeaves (void);
void Frustum_NewMap (void);

void R_SetupGL (void);


void R_Clear (void);
void R_DrawViewModel (void);

void Set_Interpolated_Weapon_f (void);
void R_BlendedRotateForEntity (entity_t *ent);
//int DoWeaponInterpolation (entity_t *this_entity);


void Insertions_Draw (void);
void R_EmitBox (const vec_rgba_t myColor, const vec3_t centerpoint, const vec3_t xyz_mins, const vec3_t xyz_maxs, const qbool bTopMost, const qbool bLines, const float Expandsize);
void R_EmitCaption (const vec3_t location, const char *caption);
void R_EmitWirePoint (const vec_rgba_t myColor, const vec3_t origin);

#endif  //__GL_LOCAL_H__
