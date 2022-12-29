/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the included (GNU.txt) GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// gl_texture.h

#ifndef __GL_TEXTURE_H__
#define __GL_TEXTURE_H__

// Baker ...
/*
Basically texture related functions and variables go here
What doesn't go here is general OpenGL stuff.

And Cvars really shouldn't go here except in the
most exception of circumstances, right?
*/



// GL constants and API stuffs (not internal vars)

#define	MAX_GLTEXTURES	2048

#define	TEX_COMPLAIN		1
#define TEX_MIPMAP			2
#define TEX_ALPHA_TEST		4
#define TEX_LUMA			8
#define TEX_FULLBRIGHT		16
#define	TEX_NOSCALE			32
#define	TEX_BRIGHTEN		64
#define TEX_NOCOMPRESS		128
#define TEX_WORLD			256

//multitexturing
#define	GL_TEXTURE0_ARB 		0x84C0
#define	GL_TEXTURE1_ARB 		0x84C1
#define	GL_TEXTURE2_ARB 		0x84C2
#define	GL_TEXTURE3_ARB 		0x84C3
#define GL_MAX_TEXTURE_UNITS_ARB	0x84E2

// GL external vars, functions


typedef void (APIENTRY *lpMTexFUNC)(GLenum, GLfloat, GLfloat);
typedef void (APIENTRY *lpSelTexFUNC)(GLenum);
extern lpMTexFUNC qglMultiTexCoord2f;
extern lpSelTexFUNC qglActiveTexture;



// Engine internal vars

extern qbool gl_mtexable;
extern int gl_textureunits;
extern	int	gl_max_size_default;
//extern	cvar_t	gl_max_size;
// Baker: this is in gl_local.h, not needed ?
//extern	cvar_t	gl_externalTextures_world, gl_externalTextures_bmodels;
// textures, lightmaps, skytexture, etc.

extern	int	texture_extension_number;

extern	texture_t	*r_notexture_mip;
extern	int		d_lightstylevalue[256];	// 8.8 fraction of base light value

extern	int	current_texture_num;





extern	int		classic_particletexture;	// little dot for particles
extern	int		classic_particletexture_blocky;	// little dot for particles

extern	int		underwatertexture;
extern int		damagetexture;
extern int		poweruptexture;
extern int		glasstexture;


extern	int		detailtexture;
#if SUPPORTS_TEXTURE_POINTER
extern	int		selectiontexture;
#endif



// Fixed slot textures.  These get reuploaded
extern	int		playertextures;				// Color translated skins per scoreboard slot x 2 for fullbright mask
extern	int		solidskytexture;			// Only rendered with cl.worldmodel; loaded on map load
extern	int		alphaskytexture;			// Only rendered with cl.worldmodel; loaded on map load
extern	int		lightmap_textures;			// Only rendered with cl.worldmodel, generated
extern	int		skyboxtextures;				// Only renderer with cl.worldmodel; reset on map load
extern	int		scrap_texnum;				// 2D aggregate tiny pics, reuploaded.

// Gets reuploaded but not predefined slot
extern	int		char_texture;


//extern	int		mirrortexturenum;	// quake texturenum, not gltexturenum
//extern	qbool	mirror;
//extern	mplane_t	*mirror_plane;

//extern	int		lightmode;		// set to gl_lightmode on mapchange


extern	int	gl_solid_bytes_per_pixel;
extern	int	gl_alpha_bytes_per_pixel;

extern	const int	gl_lightmap_format;


// Engine internal functions

void GL_Bind (int texnum);

void GL_SelectTexture (GLenum target);
void GL_DisableMultitexture (void);
void GL_EnableMultitexture (void);
void GL_EnableTMU (GLenum target);
void GL_DisableTMU (GLenum target);

void GL_Upload8 (byte *data, int width, int height, int mode);
void GL_Upload32 (unsigned *data, int width, int height, int mode);
int GL_LoadTexture						(const char *identifier, int width, int height, byte *data, int mode, const int bytesperpixel);


byte *GL_LoadExternalImage_RGBAPixels	(const char* filename, int *owidth, int *oheight, int mode, const char *media_owner_path);
// Models, sprite and brush models?
int GL_LoadExternalTextureImage			(const char *filename, const char *identifier, int mode, const char *media_owner_path);
// LMPs
mpic_t *GL_LoadQPicExternalImage		(const char *filename, const char *id, int mode, const char *media_owner_path);
// conchars is in gfx.wad so I guess it is a WAD2 image?
int GL_LoadExternalCharsetImage			(const char *filename, const char *identifier, const char *media_owner_path);
int GL_LoadQPicTexture					(const char *name, mpic_t *pic, byte *data);

void Scrap_Upload (void);


int GL_LoadPreprocess_Texture_RGBA_Pixels_And_Upload (byte *data, const char *identifier, int width, int height, int mode);

#if SUPPORTS_GL_DELETETEXTURES
void TexMgr_FreeTextures_With_ClearMemory (void);
#endif

#endif __GL_TEXTURE_H__









