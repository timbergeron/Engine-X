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
// gl_draw.c -- this is the only file outside the refresh that touches the vid buffer

#include "quakedef.h"

int		current_texture_num = -1;		// to avoid unnecessary texture sets

void GL_Bind (int texnum)
{
	if (current_texture_num == texnum)
		return;

	current_texture_num = texnum;
	eglBindTexture (GL_TEXTURE_2D, texnum);
}



static	GLenum	currenttarget = GL_TEXTURE0_ARB;
static	int	cnttextures[4] = {-1, -1, -1, -1};     // cached
static qbool	mtexenabled = false;

void GL_SelectTexture (GLenum target)
{
	if (target == currenttarget)
		return;

	qglActiveTexture (target);

	cnttextures[currenttarget-GL_TEXTURE0_ARB] = current_texture_num;
	current_texture_num = cnttextures[target-GL_TEXTURE0_ARB];
	currenttarget = target;
}

void GL_DisableMultitexture (void)
{
	if (mtexenabled)
	{
		MeglDisable (GL_TEXTURE_2D);
		GL_SelectTexture (GL_TEXTURE0_ARB);
		mtexenabled = false;
	}
}

void GL_EnableMultitexture (void)
{
	if (gl_mtexable)
	{
		GL_SelectTexture (GL_TEXTURE1_ARB);
		MeglEnable (GL_TEXTURE_2D);
		mtexenabled = true;
	}
}

void GL_EnableTMU (GLenum target)
{
	GL_SelectTexture (target);
	MeglEnable (GL_TEXTURE_2D);
}

void GL_DisableTMU (GLenum target)
{
	GL_SelectTexture (target);
	MeglDisable (GL_TEXTURE_2D);
}
