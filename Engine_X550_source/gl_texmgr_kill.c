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

extern	qbool		no24bit;


/*
================
GL_LoadQPicTexture
================
*/
// Apparently "Texture" is the texture and image is external in the nomenclature
// Gotta go.  Contains preprocessing.
int GL_LoadQPicTexture (const char *name, mpic_t *pic, byte *data)
{
	int glwidth  = Find_Power_Of_Two_Size (pic->width);
	int glheight = Find_Power_Of_Two_Size (pic->height);
	char		fullname[MAX_QPATH] = "pic:";

	strlcpy (fullname + 4, name, sizeof(fullname) - 4);

	if (glwidth == pic->width && glheight == pic->height)
	{
		pic->texnum = GL_LoadTexture (fullname, glwidth, glheight, data, TEX_ALPHA_TEST, QPAL_BYTES_PER_PIXEL_IS_1);
		pic->sl = 0;
		pic->sh = 1;
		pic->tl = 0;
		pic->th = 1;
	}
	else
	{
		byte	*buf  = ImageWork_malloc (glwidth * glheight * 1, "Load Q Pic");

		Copy_Byte_Pixels_Into_Larger_Buffer (data, buf, pic->width, pic->height, glwidth, glheight);

		pic->texnum = GL_LoadTexture (fullname, glwidth, glheight, buf, TEX_ALPHA_TEST, QPAL_BYTES_PER_PIXEL_IS_1);
		pic->sl = 0;
		pic->sh = (float)pic->width / glwidth;
		pic->tl = 0;
		pic->th = (float)pic->height / glheight;

		ImageWork_free (buf);
	}

	return pic->texnum;
}

// Gotta go
int GL_LoadPreprocess_Texture_RGBA_Pixels_And_Upload (byte *data, const char *identifier, int width, int height, int mode)
{
	qbool	should_apply_gamma = (vid_palette_gamma != 1);

	if (mode & TEX_LUMA)
	{
		should_apply_gamma = false;

//		RGBA_Fill_AlphaChannel_On_Non_Black ((unsigned int*)data, width *height);
//		mode |= TEX_ALPHA_TEST;	// Force alpha channel
//		mode &= ~TEX_LUMA;	// Destroy luma texture flag
//		mode |= TEX_FULLBRIGHT;	// Destroy luma texture flag
	} else if (mode & TEX_ALPHA_TEST)
		if (Check_RGBA_Pixels_For_AlphaChannel (data, width * height) == false)
			mode &= ~TEX_ALPHA_TEST;	// Remove alpha channel since it was not found

	if (should_apply_gamma)
		Palette_Apply_Texture_Gamma_To_RGBA_Pixels (data, width * height);

	return GL_LoadTexture (identifier, width, height, data, mode, RGBA_BYTES_PER_PIXEL_IS_4);
}


// Must be destroyed, particularly evil it returns a static mpic_t.  Potentially a bad problem.  Look through code.
// And what is with the match width shit
mpic_t *GL_LoadQPicExternalImage (const char *filename, const char *id, int mode, const char *media_owner_path)
{
	int			width, height;//, i;
	char		identifier[MAX_QPATH] = "pic_ext:";
	byte		*data, *buf; // , *src, *dest
	static	mpic_t	pic;
	int			mpicwidth, mpicheight;

	if (no24bit)
		return NULL;

//	Con_Printf("Wanted filename to load is: %s\n", filename);
	if (!(data = GL_LoadExternalImage_RGBAPixels(filename, &mpicwidth, &mpicheight, /*matchwidth, matchheight,*/ 0, media_owner_path)))
		return NULL;

	pic.width = mpicwidth;
	pic.height = mpicheight;

	if (mode & TEX_ALPHA_TEST)
		if (Check_RGBA_Pixels_For_AlphaChannel (data, pic.width * pic.height) == false)
			mode &= ~TEX_ALPHA_TEST;	// Remove TEX_ALPHA because no alpha channel found

	width  = Find_Power_Of_Two_Size (pic.width);
	height = Find_Power_Of_Two_Size (pic.height);

	strlcpy (identifier + 4, id ? id : filename, sizeof(identifier) - 4);

	if (width == pic.width && height == pic.height)
	{
		pic.texnum = GL_LoadTexture (identifier, pic.width, pic.height, data, mode, RGBA_BYTES_PER_PIXEL_IS_4);
		pic.sl = 0;
		pic.sh = 1;
		pic.tl = 0;
		pic.th = 1;
	}
	else
	{
		buf = ImageWork_malloc (width * height * 4, "Q Pic");

		Copy_RGBA_Pixels_Into_Larger_Buffer (data, buf, pic.width, pic.height, width, height);

		pic.texnum = GL_LoadTexture (identifier, width, height, buf, mode & ~TEX_MIPMAP, RGBA_BYTES_PER_PIXEL_IS_4);
		pic.sl = 0;
		pic.sh = (float)pic.width / width;
		pic.tl = 0;
		pic.th = (float)pic.height / height;
		ImageWork_free (buf);
	}

	ImageWork_free (data);
	return &pic;
}

// Die has preprocessing
// Transparent check and zero fill of alpha channel
// Double spacing
int GL_LoadExternalCharsetImage (const char *filename, const char *identifier, const char *media_owner_path)
{
	int			texnum;
	byte		*data;
	qbool		transparent = false;
	int			cwidth, cheight;

	if (no24bit)																						return 0;
	if (!(data = GL_LoadExternalImage_RGBAPixels(filename, &cwidth, &cheight, 0, media_owner_path)))	return 0;	// PATH LIMITED to location of gfx.wad ?

	if (!identifier)
		identifier = filename;

	transparent = Check_RGBA_Pixels_For_AlphaChannel(data, cwidth * cheight);

	if (!transparent)
		RGBA_Zero_Fill_AlphaChannel (data, cwidth * cheight);

	{
		int		charset_width			= cwidth;
		int		charset_height			= cheight;
		int		charset_height_doubled	= charset_height * 2; // 256
		int		charset_row_height		= charset_height / 16;
		byte	*double_spaced_charset;

		double_spaced_charset = ImageWork_malloc (charset_width * charset_height_doubled * RGBA_BYTES_PER_PIXEL_IS_4, "Double spaced charset"); /* bpp = 4 */

		SpaceOutCharset (data, double_spaced_charset, charset_width, charset_height, charset_row_height, RGBA_BYTES_PER_PIXEL_IS_4);

		texnum = GL_LoadTexture (identifier, charset_width, charset_height_doubled, double_spaced_charset, TEX_ALPHA_TEST, RGBA_BYTES_PER_PIXEL_IS_4);
		ImageWork_free (double_spaced_charset);
	}

	ImageWork_free (data);
	return texnum;
}
