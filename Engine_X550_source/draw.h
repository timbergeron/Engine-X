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

// draw.h -- these are the only functions outside the refresh allowed
// to touch the vid buffer

#ifndef __DRAW_H__
#define __DRAW_H__

typedef struct
{
	int				width, height;
	int				texnum;
	float			sl, tl, sh, th;
} mpic_t;

// Baker
extern	mpic_t		*draw_disc;	// also used on sbar

void Draw_Init					(void);
void Draw_Character				(const int x, const int y, const int num);
void Draw_DebugChar				(const char num);
void Draw_SubPic				(const int x, const int y, const mpic_t *pic, const int srcx, const int srcy, const int width, const int height);
void Draw_Pic					(const int x, const int y, const mpic_t *pic);
void Draw_TransPic				(const int x, const int y, const mpic_t *pic);
void Draw_TransPicTranslate		(const int x, const int y, const mpic_t *pic, const byte *translation);
void Draw_ConsoleBackground		(const int lines);
void Draw_BeginDisc				(void);
void Draw_EndDisc				(void);
void Draw_TileClear				(const int x, const int y, const int w, const int h);
void Draw_Fill					(const int x, const int y, const int w, const int h, const int c);
void Draw_AlphaFill				(const int x, const int y, const int w, const int h, const int c, const float alpha);

void Draw_FadeScreen			(void);
void Draw_String				(const int x, const int y, const char *str);
void Draw_Alt_String			(const int x, const int y, const char *str);
mpic_t *Draw_PicFromWad			(const char *name);
mpic_t *Draw_CachePic			(const char *path);
void Draw_Crosshair				(void);
void Draw_TextBox				(const int x, const int y, const int width, const int lines);

#endif // __DRAW_H__

