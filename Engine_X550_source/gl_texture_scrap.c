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
// gl_texture_scrap.c -- scrap textures ... the little ones

#include "quakedef.h"



/*
=============================================================================

  scrap allocation

  Allocate all the little status bar objects into a single texture

  to crutch up stupid hardware / drivers

=============================================================================
*/

// some cards have low quality of alpha pics, so load the pics
// without transparent pixels into a different scrap block.
// scrap 0 is solid pics, 1 is transparent
#define	SCRAP_BLOCK_WIDTH		256
#define	SCRAP_BLOCK_HEIGHT	256

/*static */ int	scrap_allocated[MAX_SCRAPS][SCRAP_BLOCK_WIDTH];
/*static */  byte	scrap_texels[MAX_SCRAPS][SCRAP_BLOCK_WIDTH*SCRAP_BLOCK_HEIGHT*4];
/*static */  int	scrap_dirty = 0;	// bit mask


// returns false if allocation failed
qbool Scrap_AllocBlock (int scrapnum, int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;

	best = SCRAP_BLOCK_HEIGHT;

	for (i=0 ; i < SCRAP_BLOCK_WIDTH - w ; i++)
	{
		best2 = 0;

		for (j=0 ; j<w ; j++)
		{
			if (scrap_allocated[scrapnum][i+j] >= best)
				break;
			if (scrap_allocated[scrapnum][i+j] > best2)
				best2 = scrap_allocated[scrapnum][i+j];
		}
		if (j == w)
		{	// this is a valid spot
			*x = i;
			*y = best = best2;
		}
	}

	if (best + h > SCRAP_BLOCK_HEIGHT)
		return false;

	for (i=0 ; i<w ; i++)
		scrap_allocated[scrapnum][*x+i] = best + h;

	scrap_dirty |= (1 << scrapnum);

	return true;
}

static int	scrap_uploads;

void Scrap_Upload (void)
{
	int	itexnum;

	scrap_uploads++;

	ImageWork_Start ("Scrap upload", "scrap");

	for (itexnum=0 ; itexnum<2 ; itexnum++)
	{
		if (!(scrap_dirty & (1 << itexnum)))
			continue;
		scrap_dirty &= ~(1 << itexnum);
		GL_Bind (scrap_texnum + itexnum);
		GL_Upload8 (scrap_texels[itexnum], SCRAP_BLOCK_WIDTH, SCRAP_BLOCK_HEIGHT, TEX_ALPHA_TEST);	// UPLOAD
	}

	ImageWork_Finish ();
}
