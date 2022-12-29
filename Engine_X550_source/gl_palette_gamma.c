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
// gl_palette_gamma.c -- Where gamma, contrast and maybe viewblend palette alteration meets the code

#include "quakedef.h"

float	vid_palette_gamma = 1.0;		// Applied to textures before upload
byte	vid_palette_gamma_table[256];	// Ditto

// We only do this for external images
void Palette_Apply_Texture_Gamma_To_RGBA_Pixels (byte *data, const int imagesize)
{
	int i;
	for (i=0 ; i<imagesize ; i++)
	{
		data[4*i+0] = vid_palette_gamma_table[data[4*i+0]];
		data[4*i+1] = vid_palette_gamma_table[data[4*i+1]];
		data[4*i+2] = vid_palette_gamma_table[data[4*i+2]];
	}
}


// Baker: This is palette gamma.  As opposed to hardware gamma.
// It builds a gamma table
void Palette_Check_Gamma (unsigned char *pal)
{
	int		i;
	float		inf;
	unsigned char	palette[768];

	if ((i = COM_CheckParm("-gamma")) != 0 && i+1 < com_argc)
	{
		vid_palette_gamma = CLAMP (0.3, atof(com_argv[i+1]), 1);
		Con_DevPrintf (DEV_VIDEO, "Palette gamma set to %1.1f via command line\n", vid_palette_gamma);
	}
	else
		vid_palette_gamma = 1;

//	Cvar_CmdLineCheckForceFloatByRef_Maybe (false, "-gamma", &vid_hwgamma, vid_gamma, ("Gamma setting %1.1f", vid_gamma));

	if (vid_palette_gamma != 1)
	{
		for (i=0 ; i<256 ; i++)
		{
			inf = min(255 * powf((i + 0.5) / 255.5, vid_palette_gamma) + 0.5, 255);
			vid_palette_gamma_table[i] = inf;
		}
	}
	else
	{
		for (i=0 ; i<256 ; i++)
			vid_palette_gamma_table[i] = i;
	}

	for (i=0 ; i<768 ; i++)
		palette[i] = vid_palette_gamma_table[pal[i]];

	memcpy (pal, palette, sizeof(palette));
}
