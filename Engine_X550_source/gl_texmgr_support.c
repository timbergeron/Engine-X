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
// gl_draw_textures.c -- drawing functions involving textures

#include "quakedef.h"


typedef struct
{
	int			texnum;
	char		identifier[MAX_QPATH];
	char		*pathname_explicit;
	int			width, height;
	int			scaled_width, scaled_height;
	int			texmode;
	unsigned 	crc;
	int			bpp;
} gltexture_t;

extern	gltexture_t	gltextures[MAX_GLTEXTURES];
extern int		numgltextures;

texture_t	*r_notexture_mip;


// Special textures

int		classic_particletexture;	// little dot for particles
int		classic_particletexture_blocky;	// little dot for particles

int		underwatertexture;

int		poweruptexture;
int		damagetexture;
int		glasstexture;


int		detailtexture;
#if SUPPORTS_TEXTURE_POINTER
int		selectiontexture;
#endif



// Fixed slot textures.  These get reuploaded
int		playertextures;				// Color translated skins per scoreboard slot x 2 for fullbright mask
int		solidskytexture;			// Only rendered with cl.worldmodel; loaded on map load
int		alphaskytexture;			// Only rendered with cl.worldmodel; loaded on map load
int		lightmap_textures;			// Only rendered with cl.worldmodel, generated
int		skyboxtextures;				// Only renderer with cl.worldmodel; reset on map load
int		scrap_texnum;				// 2D aggregate tiny pics, reuploaded.

// Gets reuploaded but not predefined slot
int		char_texture;

unsigned	d_8to24table[256];

#if 0
qbool OnChange_gl_max_size (cvar_t *var, const char *string)
{
	int	i;
	float	newval = atof(string);

	if (newval > gl_max_size_default)
	{
		Con_Printf ("Your hardware doesn't support texture sizes bigger than %dx%d\n", gl_max_size_default, gl_max_size_default);
		return true;
	}

	for (i = 1 ; i < newval ; i <<= 1)
		;

	if (i != newval)
	{
		Con_Printf ("Valid values for %s are powers of 2 only\n", var->name);
		return true;
	}

	return false;
}
#endif


typedef struct
{
	char	*name;
	int		minimize, maximize;
} glmode_t;

glmode_t modes[] = {
	{"GL_NEAREST", GL_NEAREST, GL_NEAREST},
	{"GL_LINEAR", GL_LINEAR, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}
};

static int num_modes =	(sizeof(modes) / sizeof(glmode_t));


extern int		gl_filter_min; // = GL_LINEAR_MIPMAP_NEAREST;
extern	int		gl_filter_max; // = GL_LINEAR;



qbool OnChange_gl_texturemode (cvar_t *var, const char *string)
{
	int		i;
	gltexture_t	*glt;

	for (i=0 ; i<num_modes ; i++)
	{
		if (COM_StringMatchCaseless (modes[i].name, string))
			break;
	}

	if (i == num_modes)
	{
		Con_Printf ("bad filter name: %s\n", string);
		return true;
	}

	gl_filter_min = modes[i].minimize;
	gl_filter_max = modes[i].maximize;

	// change all the existing mipmap (3d environment) texture objects
	for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
	{
		if (glt->texmode & TEX_MIPMAP)
		{
			Con_DevPrintf (DEV_OPENGL, "Doing texture %s\n", glt->identifier);

			GL_Bind (glt->texnum);
			MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
			MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
		}
	}
	return false;
}




void TexMgr_BuildPalette (unsigned char *palette)
{
	byte		*pal;
	int		i;
	unsigned	r, g, b, *table;

// 8 8 8 encoding
	pal = palette;
	table = d_8to24table;
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;
		*table++ =  (r<<0) + (g<<8) + (b<<16) + (255<<24);
	}
	d_8to24table[255] = 0;	// 255 is transparent

// Baker: This doesn't seem to be used anywhere
// Tonik: create a brighter palette for bmodel textures
//	pal = palette;
//	table = d_8to24table2;
//	for (i=0 ; i<256 ; i++)
//	{
//		r = min(pal[0] * (2.0 / 1.5), 255);
//		g = min(pal[1] * (2.0 / 1.5), 255);
//		b = min(pal[2] * (2.0 / 1.5), 255);
//		pal += 3;
//		*table++ = (255<<24) + (r<<0) + (g<<8) + (b<<16);
//	}
//	d_8to24table2[255] = 0;	// 255 is transparent
}

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
// gl_rmisc.c
// Baker: Validated 6-27-2011.  Boring functionary changes.


#include "quakedef.h"
//#include "gl_texture.h" .. Baker: is in gl_local.h


/*
==================
R_InitTextures
==================
*/
void R_InitTextures (void)
{
	int	x, y, m;
	byte	*dest;

// create a simple checkerboard texture for the default
	r_notexture_mip = Hunk_AllocName (1, sizeof(texture_t) + 16*16+8*8+4*4+2*2, "notexture");

	r_notexture_mip->width = r_notexture_mip->height = 16;
	r_notexture_mip->offsets[0] = sizeof(texture_t);
	r_notexture_mip->offsets[1] = r_notexture_mip->offsets[0] + 16*16;
	r_notexture_mip->offsets[2] = r_notexture_mip->offsets[1] + 8*8;
	r_notexture_mip->offsets[3] = r_notexture_mip->offsets[2] + 4*4;

	for (m=0 ; m<4 ; m++)
	{
		dest = (byte *)r_notexture_mip + r_notexture_mip->offsets[m];
		for (y=0 ; y<(16>>m) ; y++)
			for (x=0 ; x<(16>>m) ; x++)
			{
				*dest++ = ((y < (8 >> m)) ^ (x < (8 >> m))) ? 0 : 0x0e;
			}
	}
}

int		fb_skins[MAX_SCOREBOARD];	// by joe

/*
===============
R_TranslatePlayerSkin

Translates a skin texture by the per-player color lookup
===============
*/
void R_TranslatePlayerSkin (int playernum)
{
	int			shirt_color		= cl.scores[playernum].colors & 0xf0;
	int			pants_color		= (cl.scores[playernum].colors & 15) << 4;
	entity_t	*myPlayerEntity = &cl_entities[1+playernum];
	model_t		*myModel		= myPlayerEntity->model;
//	int			i;

	if (!myModel)                    return;  // player doesn't have a model yet
	if ( myModel->modelformat != mod_alias) return;  // only translate skins on alias models

	{
		aliashdr_t	*pAliasHeader = (aliashdr_t *)Mod_Extradata (myModel);
		int			size = pAliasHeader->skinwidth * pAliasHeader->skinheight;
		qbool		skin_invalid = (myPlayerEntity->skinnum < 0 || myPlayerEntity->skinnum >= pAliasHeader->numskins);

		if (size & 3) Sys_Error ("R_TranslatePlayerSkin: bad size (%d)", size);
		if (skin_invalid)  Con_DevPrintf (DEV_MODEL, "(%d): Invalid player skin #%d\n", playernum, myPlayerEntity->skinnum);

		GL_DisableMultitexture ();
		GL_Bind (playertextures + playernum);

		ImageWork_Start ("Translation", "skin");
		{
			// locate the original skin pixels
			byte		*original_skin						= skin_invalid ? (byte *)pAliasHeader + pAliasHeader->texels[0] : (byte *)pAliasHeader + pAliasHeader->texels[myPlayerEntity->skinnum];
			int 		inwidth								= pAliasHeader->skinwidth;
			int 		inheight							= pAliasHeader->skinheight;
			byte		*translated							= ImageWork_malloc (inwidth * inheight, "Temp Skin translation"); // 8 bit
			qbool		translated_skin_has_fullbrights		= false;
			int			picmip_flag							= tex_picmip_allmodels.integer ? TEX_MIPMAP : 0;

			translated_skin_has_fullbrights = QPAL_Colorize_Skin_To_QPAL (original_skin, translated, inwidth * inheight, shirt_color, pants_color);
			GL_Upload8 (translated, inwidth, inheight, picmip_flag | 0);

			fb_skins[playernum] = 0;
			// Kurok ... no fullbright skins ever
			if (!game_kurok.integer && translated_skin_has_fullbrights)
			{
				fb_skins[playernum] = playertextures + playernum + MAX_SCOREBOARD;

				GL_Bind (fb_skins[playernum]);
				GL_Upload8 (translated, inwidth, inheight, picmip_flag | TEX_FULLBRIGHT);

			}

			ImageWork_free (translated);
		}
		ImageWork_Finish ();
	}
}
