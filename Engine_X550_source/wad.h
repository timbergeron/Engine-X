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
// wad.h

#ifndef __WAD_H__
#define __WAD_H__
//===============
//   TYPES
//===============

//#define	CMP_NONE		0
//#define	CMP_LZSS		1

//#define	TYP_NONE		0
//#define	TYP_LABEL		1

//#define	TYP_LUMPY		64		// 64 + grab command number
//#define	TYP_PALETTE		64
//#define	TYP_QTEX		65
#define	TYP_QPIC		66
//#define	TYP_SOUND		67
//#define	TYP_MIPTEX		68

typedef struct
{
	int		width, height;
	byte		data[4];		// variably sized
} qpic_t;


typedef struct
{
	char		identification[4];	// should be WAD2 or 2DAW
	int		numlumps;
	int		infotableofs;
} wadinfo_t;

typedef struct
{
	int			filepos;
	int			disksize;
	int			size;			// uncompressed
	char		type;
	char		compression;
	char		pad1, pad2;
	char		name[16];		// must be null terminated
} lumpinfo_t;

//extern	int		wad_numlumps;
//extern	lumpinfo_t	*wad_lumps;
//extern	byte		*wad_base;

extern	qfs_loadinfo_t		wad_base_loadinfo;

void W_LoadWadFile				(const char *filename);
void W_CleanupName				(const char *in, char *out);
lumpinfo_t *W_GetLumpinfo		(const char *name);
void *W_GetLumpName				(const char *name);
void *W_GetLumpNum				(const int num);

void Endianness_Adjust_Pic_Width_Height (qpic_t *pic);

#if SUPPORTS_HLBSP
#if SUPPORTS_HLBSP_EXTWADS
void WAD3_LoadTextureWadFile	(const char *filename);
#endif
byte *WAD3_LoadTexture			(miptex_t *mt);
#endif


#endif // __WAD_H__
