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



qbool OnChange_gl_smoothfont (cvar_t *var, const char *string)
{
	float	newval;

	newval = atof (string);
	if (!newval == !scr_con_font_smooth.integer || !char_texture)
		return false;

	GL_Bind (char_texture);

	if (newval)
	{
		MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	return false;
}



extern	qbool		no24bit;

static byte			*draw_chars;			// 8*8 graphic characters
mpic_t				*draw_disc;
static mpic_t		*draw_backtile;			// Backtile endangered

static int			translate_texture;





static	mpic_t		crosshairpic;
static qbool		crosshairimage_loaded = false;



#define		NUMCROSSHAIRS	5
static int		crosshairtextures[NUMCROSSHAIRS];

static byte crosshairdata[NUMCROSSHAIRS][64] = {
	0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff,
	0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
	0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff,
	0xff, 0xff, 0xfe, 0xff, 0xff, 0xfe, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xfe, 0xff, 0xff, 0xfe, 0xff, 0xff,
	0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff,
	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,

	0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff,
	0xfe, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xfe, 0xff,
	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

//=============================================================================
/* Support Routines */

typedef struct cachepic_s
{
	char			name[MAX_QPATH];
	mpic_t			pic;
	qfs_loadinfo_t	loadinfo;
} cachepic_t;

//static mpic_t		conback_data;
static cachepic_t	conback;
mpic_t				*conback_sizer = &conback.pic;


#define	MAX_CACHED_PICS		128
static cachepic_t	cachepics[MAX_CACHED_PICS];
static int			numcachepics;

static byte		menuplyr_pixels[4096];

static int		pic_texels;
static int		pic_count;

mpic_t *Draw_PicFromWad (const char *qpic_name)	// Disc, backtile, ...
{
	qpic_t	*p;
	mpic_t	*pic, *pic_24bit;

	p = W_GetLumpName (qpic_name);
	pic = (mpic_t *)p;

	// Baker:
	// At this point, we loaded the .lmp which has the Quake width and height resolution
	// If the .lmp doesn't exist, it still loads the external gfx but displays foobared

	// Perhaps we should detect this situation or do an error

	ImageWork_Start ("qpic draw", qpic_name);

	if ((pic_24bit = GL_LoadQPicExternalImage(va("gfx/%s", qpic_name), qpic_name, TEX_ALPHA_TEST, wad_base_loadinfo.searchpath)))
	{
		memcpy (&pic->texnum, &pic_24bit->texnum, sizeof(mpic_t) - 8);
		goto done_qpic;
	}

	// load little ones into the scrap
	if (p->width < 64 && p->height < 64)
	{
// Baker: This should be split out into a scrap upload function
		qbool Scrap_AllocBlock (int scrapnum, int w, int h, int *x, int *y);

#define	SCRAP_BLOCK_WIDTH		256
#define	SCRAP_BLOCK_HEIGHT	256

/*static */ extern int	scrap_allocated[MAX_SCRAPS][SCRAP_BLOCK_WIDTH];
/*static */  extern byte	scrap_texels[MAX_SCRAPS][SCRAP_BLOCK_WIDTH*SCRAP_BLOCK_HEIGHT*4];
/*static */  extern int	scrap_dirty;	// bit mask
///*static */  extern int	scrap_texnum;



		int		x, y;
		int		i, j, k;
		int		texnum;

		texnum = memchr (p->data, 255, p->width*p->height) != NULL;
		if (!Scrap_AllocBlock (texnum, p->width, p->height, &x, &y))
		{
			GL_LoadQPicTexture (qpic_name, pic, p->data);
			goto done_qpic;
		}
		k = 0;
		for (i=0 ; i<p->height ; i++)
			for (j=0 ; j<p->width ; j++, k++)
				scrap_texels[texnum][(y+i)*SCRAP_BLOCK_WIDTH+x+j] = p->data[k];
		texnum += scrap_texnum;
		pic->texnum = texnum;
		pic->sl = (x + 0.01) / (float)SCRAP_BLOCK_WIDTH;
		pic->sh = (x + p->width - 0.01) / (float)SCRAP_BLOCK_WIDTH;
		pic->tl = (y + 0.01) / (float)SCRAP_BLOCK_WIDTH;
		pic->th = (y + p->height - 0.01) / (float)SCRAP_BLOCK_WIDTH;

		pic_count++;
		pic_texels += p->width * p->height;
	}
	else
	{
		GL_LoadQPicTexture (qpic_name, pic, p->data);
	}
done_qpic:
	ImageWork_Finish ();

	return pic;

}

/*
================
Draw_CachePic
================
*/
mpic_t *Draw_CachePic (const char *path_to_qpic)
{
	cachepic_t	*pic;
	int		i;
	qpic_t		*dat;
	mpic_t		*pic_24bit;

	for (pic = cachepics, i = 0 ; i < numcachepics ; pic++, i++)
		if (COM_StringMatch (path_to_qpic, pic->name))
			return &pic->pic;

	if (numcachepics == MAX_CACHED_PICS)
		Sys_Error ("numcachepics == MAX_CACHED_PICS");
	numcachepics++;
	StringLCopy (pic->name, path_to_qpic);

	// load the pic from disk
	if (!(dat = (qpic_t *)QFS_LoadTempFile(path_to_qpic, NULL /*PATH LIMIT ME*/)))
		Sys_Error ("Draw_CachePic: failed to load %s", path_to_qpic);

	Endianness_Adjust_Pic_Width_Height (dat);	// Just does littlelong stuff on the data for endianess

	// Update the path
	StringLCopy (pic->loadinfo.searchpath, qfs_lastload.datapath);

	// HACK HACK HACK --- we need to keep the bytes for
	// the translatable player picture just for the menu
	// configuration dialog
	if (COM_StringMatch (path_to_qpic, "gfx/menuplyr.lmp"))
		memcpy (menuplyr_pixels, dat->data, dat->width * dat->height);

	pic->pic.width = dat->width;
	pic->pic.height = dat->height;

	ImageWork_Start ("Draw_CachePic", path_to_qpic);

	if ((pic_24bit = GL_LoadQPicExternalImage (path_to_qpic, NULL,  TEX_ALPHA_TEST, pic->loadinfo.searchpath)))
		memcpy (&pic->pic.texnum, &pic_24bit->texnum, sizeof(mpic_t) - 8);
	else
		GL_LoadQPicTexture (path_to_qpic, &pic->pic, dat->data);

	ImageWork_Finish ();

	return &pic->pic;
}

/* Baker: Isn't actually used
void Draw_CharToConback (int num, byte *dest)
{
	int		row, col, drawline, x;
	byte	*source;

	row = num >> 4;
	col = num & 15;
	source = draw_chars + (row<<10) + (col<<3);

	drawline = 8;

	while (drawline--)
	{
		for (x=0 ; x<8 ; x++)
			if (source[x] != 255)
				dest[x] = 0x60 + source[x];
		source += 128;
		dest += 320;
	}
}
*/

/*

===============

Draw_LoadPics -- johnfitz

===============

*/

void Draw_LoadPics (void)

{
	draw_disc = Draw_PicFromWad ("disc");
	draw_backtile = Draw_PicFromWad ("backtile");		// Backtile endangered
}




static qbool Draw_LoadCharset (const char *charset_name)
{
	int	texnum;

	ImageWork_Start ("Draw_LoadCharset", charset_name);

	if (COM_StringMatchCaseless (charset_name, "original"))
	{
		// Double space "leaving empty space between the rows so that chars don't stumble on each other because of texture smoothing"
		int		charset_width			= 128;
		int		charset_height			= 128;
		int		charset_height_doubled	= 128 * 2; // 256
		int		charset_row_height		= 8;
		byte	*double_spaced_charset;

		double_spaced_charset = ImageWork_malloc (charset_width*charset_height_doubled * QPAL_BYTES_PER_PIXEL_IS_1, "Double spaced charset"); /* bpp = 1 */

		SpaceOutCharset (draw_chars, double_spaced_charset, charset_width, charset_height, charset_row_height, QPAL_BYTES_PER_PIXEL_IS_1);

		char_texture = GL_LoadTexture ("pic:charset", 128, 256, double_spaced_charset, TEX_ALPHA_TEST, QPAL_BYTES_PER_PIXEL_IS_1);

		ImageWork_free (double_spaced_charset);
		goto done;
	}


	// Baker: default means load the "conchars.tga"
	//        The official replacement character set
	if (COM_StringMatchCaseless (charset_name, "default") && (texnum = GL_LoadExternalCharsetImage("gfx/conchars", "pic:charset", wad_base_loadinfo.searchpath)))
	{
		char_texture = texnum;
		goto done;
	}

	// Not path limited to allow testing of charsets
	if ((texnum = GL_LoadExternalCharsetImage(va("textures/charsets/%s", charset_name), "pic:charset", NULL)))
	{
		char_texture = texnum;
		goto done;
	}

	Con_Printf ("Couldn't load charset \"%s\"\n", charset_name);
	ImageWork_Finish ();

	return false;

done:
	if (!scr_con_font_smooth.integer)
	{
		MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	// Baker: Neat ... so IF gl_smoothfont, we do whatever GLUpload32 decided
		MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	ImageWork_Finish ();
	return true;
}

qbool OnChange_gl_consolefont (cvar_t *var, const char *string)
{
	return (Draw_LoadCharset (string) == false);	// Cancel if failed to load
}

void Draw_LoadCharset_f (void)
{
	switch (Cmd_Argc())
	{
	case 1:
		Con_Printf ("Current charset is \"%s\"\n", scr_con_font.string);
		break;

	case 2:
		Cvar_SetStringByRef (&scr_con_font, Cmd_Argv(1));
		break;

	default:
		Con_Printf ("Usage: %s [charset]\n", Cmd_Argv(0));
	}
}

void Draw_InitCharset (void)
{
	int	i;

	draw_chars = W_GetLumpName ("conchars");

	// Go through the charset and convert black to color 255
	for (i=0 ; i<256*64 ; i++)
		if (draw_chars[i] == 0)
			draw_chars[i] = 255;	// proper transparent color

	Draw_LoadCharset (scr_con_font.string);

	// Baker: If it didn't load ... wait.  Because we haven't registered the cvar yet
//	if (!char_texture)
//		Cvar_SetStringByRef (&scr_con_font, "original");

//	if (!char_texture)
//		Sys_Error ("Draw_InitCharset: Couldn't load charset");
}


qbool OnChange_gl_crosshairimage (cvar_t *var, const char *string)
{
	mpic_t	*pic;

	ImageWork_Start ("crosshairimage", string);

	do
	{
		if (!string[0])
		{
			crosshairimage_loaded = false;
				break;
		}
	
		if (!(pic = GL_LoadQPicExternalImage(va("crosshairs/%s", string), "crosshair",  TEX_ALPHA_TEST, NULL)))
		{
			crosshairimage_loaded = false;
			Con_Printf ("Couldn't load crosshair \"%s\"\n", string);
	
				break;
		}
	
		MeglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		MeglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		crosshairpic = *pic;
		crosshairimage_loaded = true;

	} while (0);

	ImageWork_Finish ();
	return false;
}


void Draw_InitConback (void)
{
	qpic_t		*dat;
	int			start;
	mpic_t		*pic_24bit;

	start = Hunk_LowMark ();

	if (!(dat = (qpic_t *)QFS_LoadHunkFile("gfx/conback.lmp", NULL /*PATH LIMIT ME*/)))
		Sys_Error ("Couldn't load gfx/conback.lmp");

	Endianness_Adjust_Pic_Width_Height (dat);

#ifndef _DEBUG		// Baker: Not sure this is important
	if (dat->width != 320 || dat->height != 200)
		Sys_Error ("Draw_InitConback: conback.lmp size is not 320x200");
#endif

	// Update the path
	StringLCopy (conback.loadinfo.searchpath, qfs_lastload.datapath);

	ImageWork_Start ("Conback", "Conback"); 
	if ((pic_24bit = GL_LoadQPicExternalImage("gfx/conback", "conback", 0, conback.loadinfo.searchpath)))
		memcpy (&conback.pic.texnum, &pic_24bit->texnum, sizeof(mpic_t) - 8);	// Baker: Updates most of the mpic_t fields (not width/height)
	else
	{
		conback.pic.width = dat->width;
		conback.pic.height = dat->height;
		GL_LoadQPicTexture ("conback", &conback.pic, dat->data);
	}
	ImageWork_Finish ();

	conback.pic.width = vid.conwidth;
	conback.pic.height = vid.conheight;

	// free loaded console
	Hunk_FreeToLowMark (start);
}

/*
================
Draw_Character

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/


void Draw_Character (const int x, const int y, const int in_num)
{
	int			row, col;
	int			myNum = in_num;
	float		frow, fcol;

	if (y <= -8)
		return;			// totally off screen

	if (myNum == 32)
		return;		// space

	myNum &= 255;

	row = myNum >> 4;
	col = myNum & 15;

	frow = row * 0.0625;
	fcol = col * 0.0625;

	mglPushStates ();

	GL_Bind (char_texture);

	mglFinishedStates ();

	eglBegin (GL_QUADS);
	eglTexCoord2f (fcol, frow);
	eglVertex2f (x, y);
	eglTexCoord2f (fcol + 0.0625, frow);
	eglVertex2f (x+8, y);
	eglTexCoord2f (fcol + 0.0625, frow + 0.03125);
	eglVertex2f (x+8, y+8);
	eglTexCoord2f (fcol, frow + 0.03125);
	eglVertex2f (x, y+8);
	eglEnd ();

	mglPopStates ();
}

/*
================
Draw_String
================
*/
void Draw_String (const int in_x, const int y, const char *str)
{
	float		frow, fcol;
	int			num;
	int			use_x = in_x;

	if (y <= -8)
		return;			// totally off screen

	if (!*str)
		return;

	GL_Bind (char_texture);

	eglBegin (GL_QUADS);

	while (*str)		// stop rendering when out of characters
	{
		if ((num = *str++) != 32)	// skip spaces
		{
			frow = (float)(num >> 4) * 0.0625;
			fcol = (float)(num & 15) * 0.0625;

			eglTexCoord2f	(fcol,			frow);
			eglVertex2f		(use_x,			y);
			eglTexCoord2f	(fcol + 0.0625, frow);
			eglVertex2f		(use_x + 8,		y);
			eglTexCoord2f	(fcol + 0.0625, frow + 0.03125);
			eglVertex2f		(use_x + 8,		y + 8);
			eglTexCoord2f	(fcol,			frow + 0.03125);
			eglVertex2f		(use_x,			y + 8);
		}
		use_x += 8;
	}

	eglEnd ();
}

/*
================
Draw_Alt_String
================
*/
void Draw_Alt_String (const int in_x, const int y, const char *str)
{
	float		frow, fcol;
	int			use_x = in_x;
	int			num;

	if (y <= -8)
		return;			// totally off screen

	if (!*str)
		return;

	GL_Bind (char_texture);

	eglBegin (GL_QUADS);

	while (*str)		// stop rendering when out of characters
	{
		if ((num = *str++|0x80) != (32|0x80))	// skip spaces
		{
			frow = (float)(num >> 4) * 0.0625;
			fcol = (float)(num & 15) * 0.0625;
			
			eglTexCoord2f	(fcol,			frow);
			eglVertex2f		(use_x,			y);
			eglTexCoord2f	(fcol + 0.0625, frow);
			eglVertex2f		(use_x + 8,		y);
			eglTexCoord2f	(fcol + 0.0625, frow + 0.03125);
			eglVertex2f		(use_x + 8,		y + 8);
			eglTexCoord2f	(fcol,			frow + 0.03125);
			eglVertex2f		(use_x,			y + 8);
		}
		use_x += 8;
	}

	eglEnd ();
}

#if 0
/*
================
Draw_Crosshair		-- joe, from FuhQuake
================
*/
void Draw_Crosshair (void)
{
	float		x, y, ofs1, ofs2, ofsy3, ofsy4, sh, th, sl, tl;
	byte		*col;
	extern vrect_t	scr_vrect;
#if 1
	float adj;
	trace_t tr;
	vec3_t end;
	vec3_t start;
	vec3_t right, up, fwds;
#endif

	if (chase_active.integer)
		return;		// Baker: stupid to have chase active cam have crosshair

	if ((scr_crosshair.integer >= 2 && scr_crosshair.integer <= NUMCROSSHAIRS + 1) || (crosshairimage_loaded && scr_crosshair.integer))
	{
#if 0
		if (0)
		{
			void ML_Project (vec3_t in, vec3_t out, vec3_t viewangles, vec3_t vieworg, float wdivh, float fovy);
			AngleVectors(cl.viewangles, fwds, right, up);
			VectorCopy(cl_entities[cl.viewentity].origin, start);
			start[2]+=16;
			VectorMultiplyAdd(start, 4096, fwds, end);

			memset(&tr, 0, sizeof(tr));
			tr.fraction = 1;
			SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &tr);
			start[2]-=12;
			if (tr.fraction == 1)
			{
				x = scr_vrect.x + scr_vrect.width/2 + cl_crossx.integer;
				y = scr_vrect.y + scr_vrect.height/2 + cl_crossy.integer;
			}
			else
			{
				adj=cl.viewheight;
				start[2]+=adj;
				ML_Project(tr.endpos, end, cl.viewangles, start, (float)scr_vrect.width/scr_vrect.height, r_refdef.fov_y);//Entar
				x = scr_vrect.x + scr_vrect.width/2 + cl_crossx.integer;
				y = (scr_vrect.y+scr_vrect.height*(end[1]));
				y = scr_vrect.height - y; // Entar : HACKYNESS yes, but it works pretty well
				y -= (y - (scr_vrect.height / 2)) / 2;//
			}
		}
		else
#endif
		{
			x = scr_vrect.x + scr_vrect.width / 2 + cl_crossx.integer;
			y = scr_vrect.y + scr_vrect.height / 2 + cl_crossy.integer;
		}

		if (!gl_crosshairalpha.floater)
			return;

		MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		col = StringToRGB (cl_crosshaircolor.string);

		if (gl_crosshairalpha.floater)
		{
			MeglDisable (GL_ALPHA_TEST);
			MeglEnable (GL_BLEND);
			col[3] = CLAMP (0, gl_crosshairalpha.floater, 1) * 255;
			MeglColor4ubv (col);
		}
		else
		{
			MeglColor3ubv (col);
		}

		if (scr_crosshair.integer == 1 && crosshairimage_loaded)
		{
			sh = crosshairpic.sh;
			sl = crosshairpic.sl;
			th = crosshairpic.th;
			tl = crosshairpic.tl;

			GL_Bind (crosshairpic.texnum);

			if (cl_crosshairsize.floater == 0)
			{
				ofs1 = 4 - 4.0 / crosshairpic.width;
				ofs2 = 4 + 4.0 / crosshairpic.width;
				sh = crosshairpic.sh;
				sl = crosshairpic.sl;
				th = crosshairpic.th;
				tl = crosshairpic.tl;

				ofs1 *= (vid.width / 320.0f) ;// * CLAMP (0, cl_crosshairsize.floater, 20);
				ofs2 *= (vid.width / 320.0f) ; //* CLAMP (0, cl_crosshairsize.floater, 20);
				ofsy3 = ofs1;
				ofsy4 = ofs2;
			}
			else
			{
				// Baker: 640 x 480 is the crosshair standard for us

				// if gets larger than 640x480, we need to magnify the crosshair

				// otherwise we shrink it

				// Then let's neutralize crosshair size and crosshair alpha

				// Neither make any sense with out implementation

				ofs1 =  4 - 4.0f / (float)crosshairpic.width;
				ofs2 = 4 + 4.0f / (float)crosshairpic.width;

				ofsy3 = 4 - 4.0f / (float)crosshairpic.height;

				ofsy4 = 4 + 4.0f / (float)crosshairpic.height;



				// Why are we interjecting the console width here

				// That's crazy

				ofs1 *=  CLAMP (0, cl_crosshairsize.floater, 20);
				ofs2 *=  CLAMP (0, cl_crosshairsize.floater, 20);
				ofsy3 *= CLAMP (0, cl_crosshairsize.floater, 20);
				ofsy4 *= CLAMP (0, cl_crosshairsize.floater, 20);


				// Scale to width / 320 + height / 240



				ofs1 *= (vid.width / 320.0f) ;// * CLAMP (0, cl_crosshairsize.floater, 20);

				ofs2 *= (vid.width / 320.0f) ; //* CLAMP (0, cl_crosshairsize.floater, 20);

				ofsy3 *= (vid.height / 240.0f) * (4/3);

				ofsy4 *= (vid.height / 240.0f) * (4/3);



				// Magnify to correct for larger resolutions
				ofs1 *= ((float)scr_vrect.width / (float)vid.width);
				ofs2 *= ((float)scr_vrect.width / (float)vid.width);
				ofsy3 *= ((float)scr_vrect.height / (float)vid.height);
				ofsy4 *= ((float)scr_vrect.height / (float)vid.height);
			}

//			Con_Printf ("Cross is %i %i\n", crosshairpic.width, crosshairpic.height);

//			Con_Printf ("Size is scr_vrect.width %i %i\n", scr_vrect.width, scr_vrect.height);

//			Con_Printf ("ofs1, 2, 3, 4 --> %2.2f %2.2f %2.2f %2.2f\n", ofs1, ofs2, ofsy3, ofsy4);

		}

		else

		{
			GL_Bind (crosshairtextures[(int)scr_crosshair.integer-2]);
			ofs1 = 3.5;
			ofs2 = 4.5;
			tl = sl = 0;
			sh = th = 1;


			// Why are we interjecting the console width here

			// That's crazy ... only for "character crosshair" which looks dumb

			ofs1 *= (vid.width / 320) * CLAMP (0, cl_crosshairsize.floater, 20);

			ofs2 *= (vid.width / 320) * CLAMP (0, cl_crosshairsize.floater, 20);

			ofsy3 = ofs1;

			ofsy4 = ofs2;

		}



		eglBegin (GL_QUADS);
		eglTexCoord2f (sl, tl);
		eglVertex2f (x - ofs1, y - ofsy3);
		eglTexCoord2f (sh, tl);
		eglVertex2f (x + ofs2, y - ofsy3);
		eglTexCoord2f (sh, th);
		eglVertex2f (x + ofs2, y + ofsy4);
		eglTexCoord2f (sl, th);
		eglVertex2f (x - ofs1, y + ofsy4);
		eglEnd ();

		if (gl_crosshairalpha.floater)
		{
			MeglDisable (GL_BLEND);
			MeglEnable (GL_ALPHA_TEST);
		}

		MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		MeglColor4ubv (color_white);
	}
	else if (scr_crosshair.integer)
	{
		Draw_Character (scr_vrect.x + scr_vrect.width / 2 - 4 + cl_crossx.integer, scr_vrect.y + scr_vrect.height / 2 - 4 + cl_crossy.integer, '+');
	}
}
#else
/*
================
Draw_Crosshair		-- joe, from FuhQuake
================
*/
void Draw_Crosshair (void)
{
	if (chase_active.integer)			return;
	if (!scr_crosshair.integer)				return;
	if (!scr_crosshair_alpha.floater)	return;
	if (!scr_crosshair_size.floater)		return;
	if (cls.demofile && cls.titledemo)	return; // No crosshair or sbar if titlescreen

	do
	{
		extern	vrect_t	scr_vrect;

		float ofs_left, ofs_right, ofs_top, ofs_bottom;

		int crosshairtexture = crosshairpic.texnum;

		// Set crosshair s, t values
		float sh = crosshairpic.sh;
		float sl = crosshairpic.sl;
		float th = crosshairpic.th;
		float tl = crosshairpic.tl;

		float crosshairsize = CLAMP(0, scr_crosshair_size.floater, 100);

		// Determine middle of the screen
		const int	centerviewport_x = scr_vrect.x + scr_vrect.width / 2;
		const int	centerviewport_y = scr_vrect.y + scr_vrect.height / 2;

		// Add crosshair_x and crosshair_y into equation
		int			crosshair_pos_x  = centerviewport_x + scr_crosshair_offset_x.integer;
		int			crosshair_pos_y =  centerviewport_y + scr_crosshair_offset_y.integer;

		// Determine crosshair color;
		byte		*col = StringToRGB (scr_crosshair_color.string);

		if (!scr_crosshair_style.integer)	// Simple "+" crosshair (8x8)
		{
			Draw_Character (crosshair_pos_x -4, crosshair_pos_y - 4, '+');
			continue;
		}

		// Set alpha if specified
		if (scr_crosshair_alpha.floater)	col[3] = CLAMP (0, scr_crosshair_alpha.floater, 1) * 255.0f;

		// Texture coordinates if not an external image
		if (scr_crosshair_style.integer > 1 || !crosshairimage_loaded) { tl = sl = 0; sh = th = 1; }

		// Offsets if not an external image

		if (scr_crosshair_style.integer > 1 || !crosshairimage_loaded)
		{
			ofs_left    = ofs_top    = 3.5;
			ofs_right   = ofs_bottom = 4.5;

			crosshairtexture = crosshairtextures[CLAMP(0, scr_crosshair_style.integer-2, NUMCROSSHAIRS-1)];
		}
		else
		{	// This code assumes the crosshair is a square (width and height match) ... oh well

			ofs_left	= ofs_top    = 4 - 4.0f / (float)crosshairpic.width;
			ofs_right	= ofs_bottom = 4 + 4.0f / (float)crosshairpic.width;
		}

		// Add in crosshair size

		ofs_left    *= crosshairsize;
		ofs_right   *= crosshairsize;
		ofs_top     *= crosshairsize;
		ofs_bottom  *= crosshairsize;

		// Add in aspect ratio correction
		// Baker: Stick with the relative canvas and adjust for now
//		if (glwidth > 640 && glheight > 400)
//		{   // BAKER: My automatic consizing makes this rather unnecessary, actually..
//			// Static crosshair size on-screen
//			ofs_left	*= (glwidth  / 640) * 4/3;
//			ofs_right	*= (glwidth  / 640) * 4/3;
//			ofs_top     *= (glheight / 480) * 4/3;
//			ofs_bottom  *= (glheight / 480) ;
//		}
//		else
		{
			// Do nothing!

		}
		// Setup GL parameters

		mglPushStates ();

		MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		if (scr_crosshair_alpha.floater)
		{
			MeglDisable (GL_ALPHA_TEST);
			MeglEnable (GL_BLEND);
			MeglColor4ubv (col);
		}
		else
		{
			MeglColor3ubv (col);
		}

		GL_Bind (crosshairtexture);
//		MeglColor4ubv (col);

		mglFinishedStates ();

		eglBegin (GL_QUADS);
		eglTexCoord2f (sl, tl);
		eglVertex2f (crosshair_pos_x - ofs_left, crosshair_pos_y - ofs_top);
		eglTexCoord2f (sh, tl);
		eglVertex2f (crosshair_pos_x + ofs_right, crosshair_pos_y - ofs_top);
		eglTexCoord2f (sh, th);
		eglVertex2f (crosshair_pos_x + ofs_right, crosshair_pos_y + ofs_bottom);
		eglTexCoord2f (sl, th);
		eglVertex2f (crosshair_pos_x - ofs_left, crosshair_pos_y + ofs_bottom);
		eglEnd ();

		if (scr_crosshair_alpha.floater)
		{
			MeglDisable (GL_BLEND);
			MeglEnable (GL_ALPHA_TEST);
		}

		MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		MeglColor3ubv (color_white);
		mglPopStates ();

	} while (0);
}
#endif
/*
================
Draw_TextBox
================
*/
void Draw_TextBox (const int x, const int y, const int in_width, const int lines)
{
	mpic_t	*p;
	int		cx = x,
			cy = y,
			n;
	int		use_width = in_width;

	// draw left side
//	cx = x;
//	cy = y;
	p = Draw_CachePic ("gfx/box_tl.lmp");
	Draw_TransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_ml.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		Draw_TransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_bl.lmp");
	Draw_TransPic (cx, cy+8, p);

	// draw middle
	cx += 8;
	while (use_width > 0)
	{
		cy = y;
		p = Draw_CachePic ("gfx/box_tm.lmp");
		Draw_TransPic (cx, cy, p);
		p = Draw_CachePic ("gfx/box_mm.lmp");
		for (n = 0; n < lines; n++)
		{
			cy += 8;
			if (n == 1)
				p = Draw_CachePic ("gfx/box_mm2.lmp");
			Draw_TransPic (cx, cy, p);
		}
		p = Draw_CachePic ("gfx/box_bm.lmp");
		Draw_TransPic (cx, cy+8, p);
		use_width -= 2;
		cx += 16;
	}

	// draw right side
	cy = y;
	p = Draw_CachePic ("gfx/box_tr.lmp");
	Draw_TransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_mr.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		Draw_TransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_br.lmp");
	Draw_TransPic (cx, cy+8, p);
}

/*
================
Draw_DebugChar

Draws a single character directly to the upper right corner of the screen.
This is for debugging lockups by drawing different chars in different parts
of the code.
================
*/
void Draw_DebugChar (const char num)
{
}

/*
=============
Draw_AlphaPic
=============
*/
void Draw_AlphaPic (const int x, const int y, const mpic_t *pic, const float alpha)
{
	extern int scrap_dirty;

	if (scrap_dirty)
		Scrap_Upload ();	// Baker: Put the if into Scrap_Upload and rename to Scrap_Upload_Maybe
	MeglDisable(GL_ALPHA_TEST);
	MeglEnable (GL_BLEND);
	MeglCullFace (GL_FRONT);
	MeglColor4f (1, 1, 1, alpha);
	GL_Bind (pic->texnum);
	eglBegin (GL_QUADS);
	eglTexCoord2f (pic->sl, pic->tl);
	eglVertex2f (x, y);
	eglTexCoord2f (pic->sh, pic->tl);
	eglVertex2f (x+pic->width, y);
	eglTexCoord2f (pic->sh, pic->th);
	eglVertex2f (x+pic->width, y+pic->height);
	eglTexCoord2f (pic->sl, pic->th);
	eglVertex2f (x, y+pic->height);
	eglEnd ();
	MeglColor3ubv (color_white);
	MeglEnable(GL_ALPHA_TEST);
	MeglDisable (GL_BLEND);
}


/*
=============
Draw_Pic
=============
*/
void Draw_Pic (const int x, const int y, const mpic_t *pic)
{
	extern int scrap_dirty;

	if (scrap_dirty)
		Scrap_Upload ();
	GL_Bind (pic->texnum);
	eglBegin (GL_QUADS);
	eglTexCoord2f (pic->sl, pic->tl);
	eglVertex2f (x, y);
	eglTexCoord2f (pic->sh, pic->tl);
	eglVertex2f (x+pic->width, y);
	eglTexCoord2f (pic->sh, pic->th);
	eglVertex2f (x+pic->width, y+pic->height);
	eglTexCoord2f (pic->sl, pic->th);
	eglVertex2f (x, y+pic->height);
	eglEnd ();
}

void Draw_SubPic (const int x, const int y, const mpic_t *pic, const int srcx, const int srcy, const int width, const int height)
{
	float	newsl, newtl, newsh, newth, oldglwidth, oldglheight;

	extern int scrap_dirty;

	if (scrap_dirty)
		Scrap_Upload ();

	oldglwidth = pic->sh - pic->sl;
	oldglheight = pic->th - pic->tl;

	newsl = pic->sl + (srcx * oldglwidth) / pic->width;
	newsh = newsl + (width * oldglwidth) / pic->width;

	newtl = pic->tl + (srcy * oldglheight) / pic->height;
	newth = newtl + (height * oldglheight) / pic->height;

	GL_Bind (pic->texnum);
	eglBegin (GL_QUADS);
	eglTexCoord2f (newsl, newtl);
	eglVertex2f (x, y);
	eglTexCoord2f (newsh, newtl);
	eglVertex2f (x+width, y);
	eglTexCoord2f (newsh, newth);
	eglVertex2f (x+width, y+height);
	eglTexCoord2f (newsl, newth);
	eglVertex2f (x, y+height);
	eglEnd ();
}

/*
=============
Draw_TransPic
=============
*/
void Draw_TransPic (const int x, const int y, const mpic_t *pic)
{
	if (x < 0 || (unsigned)(x + pic->width) > vid.width || y < 0 || (unsigned)(y + pic->height) > vid.height)
		Sys_Error ("Draw_TransPic: bad coordinates");

	Draw_Pic (x, y, pic);
}


/*
=============
Draw_TransPicTranslate

Only used for the player color selection menu
=============
*/
void Draw_TransPicTranslate (const int x, const int y, const mpic_t *pic, const byte *translation)
{
	int			v, u, c;
	unsigned	trans[64*64], *dest;
	byte		*src;
	int			p;

	GL_Bind (translate_texture);

	c = pic->width * pic->height;

	dest = trans;
	for (v=0 ; v<64 ; v++, dest += 64)
	{
		src = &menuplyr_pixels[((v*pic->height)>>6)*pic->width];
		for (u=0 ; u<64 ; u++)
		{
			p = src[(u*pic->width)>>6];
			if (p == 255)
				dest[u] = p;
			else
				dest[u] =  d_8to24table[translation[p]];
		}
	}

	eglTexImage2D (GL_TEXTURE_2D, 0, gl_alpha_bytes_per_pixel, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);  // UPLOAD ROGUE

	MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	eglBegin (GL_QUADS);
	eglTexCoord2f (0, 0);
	eglVertex2f (x, y);
	eglTexCoord2f (1, 0);
	eglVertex2f (x+pic->width, y);
	eglTexCoord2f (1, 1);
	eglVertex2f (x+pic->width, y+pic->height);
	eglTexCoord2f (0, 1);
	eglVertex2f (x, y+pic->height);
	eglEnd ();
}


/*
================
Draw_ConsoleBackground
================
*/

void Draw_ConsoleBackground (const int lines)
{
	char	ver[80];

	if (!scr_con_alpha.floater && lines < vid.height)
		goto end;

	if (lines == vid.height)
		Draw_Pic (0, lines - vid.height, &conback.pic);
	else
		Draw_AlphaPic (0, lines - vid.height, &conback.pic, CLAMP (0, scr_con_alpha.floater, 1));

end:
	snprintf(ver, sizeof(ver), "%s %s %s", ENGINE_NAME, RENDERER_NAME, ENGINE_VERSION);
	Draw_Alt_String (vid.conwidth - strlen(ver) * 8 - 8, lines - 10, ver);
}


/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear (const int x, const int y, const int w, const int h)   // Backtile endangered
{
	GL_Bind (draw_backtile->texnum);
	eglBegin (GL_QUADS);
	eglTexCoord2f (x/64.0, y/64.0);
	eglVertex2f (x, y);
	eglTexCoord2f ((x+w)/64.0, y/64.0);
	eglVertex2f (x+w, y);
	eglTexCoord2f ((x+w)/64.0, (y+h)/64.0);
	eglVertex2f (x+w, y+h);
	eglTexCoord2f (x/64.0, (y+h)/64.0);
	eglVertex2f (x, y+h);
	eglEnd ();
}
//=============================================================================

/*
================
Draw_FadeScreen
================
*/
void Draw_FadeScreen (void)
{
	MeglEnable (GL_BLEND);
	MeglDisable (GL_TEXTURE_2D);
	MeglColor4f (0, 0, 0, 0.7);
	eglBegin (GL_QUADS);

	eglVertex2f (0,0);
	eglVertex2f (vid.width, 0);
	eglVertex2f (vid.width, vid.height);
	eglVertex2f (0, vid.height);

	eglEnd ();
	MeglColor3ubv (color_white);
	MeglEnable (GL_TEXTURE_2D);
	MeglDisable (GL_BLEND);

	Sbar_Changed ();
}

//=============================================================================

/*
================
Draw_BeginDisc

Draws the little blue disc in the corner of the screen.
Call before beginning any disc IO.
================
*/
void Draw_BeginDisc (void)
{
#if 0 //Part of intel fix

	if (!draw_disc)
		return;
	eglDrawBuffer  (GL_FRONT);
	Draw_Pic (vid.width - 24, 0, draw_disc);
	eglDrawBuffer  (GL_BACK);

#endif
}


/*
================
Draw_EndDisc

Erases the disc icon.
Call after completing any disc IO
================
*/
void Draw_EndDisc (void)
{
}

/*
================
GL_Set2D

Setup as if the screen was 320*200
================
*/
void GL_Set2D (void)
{
	eglViewport (glx, gly, glwidth, glheight);

	eglMatrixMode (GL_PROJECTION);
	eglLoadIdentity ();

	//       Left, Right, Bottom, Top, Near, Far
	eglOrtho (0, vid.width, vid.height, 0, -99999, 99999);

	eglMatrixMode (GL_MODELVIEW);
	eglLoadIdentity ();

	MeglDisable		(GL_DEPTH_TEST);
	MeglDisable		(GL_CULL_FACE);
	MeglDisable		(GL_BLEND);
	MeglEnable		(GL_ALPHA_TEST);
//	MeglDisable (GL_ALPHA_TEST);
//	MeglDisable		(GL_FOG);
	MeglColor3ubv	(color_white);
}



static void Draw_InitInternalCrosshairs (void)
{
	int	i;

	ImageWork_Start ("crosshairs", "crosshairs");
	// Load the crosshair pics
	for (i=0 ; i<NUMCROSSHAIRS ; i++)
	{
		crosshairtextures[i] = GL_LoadTexture (va("crosshair%i",i), 8, 8, crosshairdata[i], TEX_ALPHA_TEST, QPAL_BYTES_PER_PIXEL_IS_1);
		MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	ImageWork_Finish ();
}



/*
===============
Draw_Init
===============
*/
qbool qclassic=false;

void Draw_InitConback(void);
void Draw_Init (void)
{
	void Draw_LoadCharset_f (void);
	


	Cmd_AddCommand ("loadcharset", Draw_LoadCharset_f);

	Cvar_Registration_Client_Rendering_Options_2D ();

	Cvar_KickOnChange (&scr_con_font_smooth);	// Cvar needs to execute onchange event to take effect
	Cvar_KickOnChange (&tex_scene_texfilter);	// Cvar needs to execute onchange event to take effect

	qclassic = COM_CheckParm("-classic");
	no24bit = (COM_CheckParm("-no24bit") || qclassic) ? true : false;


//	Cvar_SetDefaultFloatByRef (&gl_max_size, 16);

//	Con_DevPrintf ("Baker: We set gl_max_size here so we shouldn't be uploading before here\n");



	// load the console background and the charset by hand, because we need to write the version
	// string into the background before turning it into a texture
	Draw_InitCharset ();		// Initialize charset
	Draw_InitConback ();		// Initialize console background

	// save a texture slot for translated picture
	translate_texture = texture_extension_number++;

	Draw_InitInternalCrosshairs ();

	// load game pics

	Draw_LoadPics ();		// Loads the disc and the backtile

//	Con_DevPrintf ("Baker: Remember, only now do we have our texture slots \"ok\" for uploading\n");

	// Baker: need to get here so after charset is uploaded
	if (!char_texture)
	{	// no 24bit type of circumstance
		Cvar_SetStringByRef (&scr_con_font, "original");

		if (!char_texture)
			Sys_Error ("Draw_InitCharset: Couldn't load charset");
	}


	Cvar_KickOnChange (&scr_crosshair_image);	// Cvar needs to execute onchange event to take effect


}
