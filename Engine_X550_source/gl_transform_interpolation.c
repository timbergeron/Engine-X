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
//

#include "quakedef.h"



/*
=============
R_BlendedRotateForEntity

fenix@io.com: model transform interpolation
=============
*/
void R_BlendedRotateForEntity (entity_t *ent)
{
	float		lerpfrac;
	vec3_t		d;
	int			i;
	float		timepassed;

	// positional interpolation
	timepassed = cl.time - ent->movelerpstart;

	if (ent->movelerpstart == 0 || timepassed > 1)
	{
		ent->movelerpstart = cl.time;
		VectorCopy (ent->origin, ent->previousorigin);
		VectorCopy (ent->origin, ent->currentorigin);
	}

	if (!VectorCompare (ent->origin, ent->currentorigin))
	{
		ent->movelerpstart = cl.time;
		VectorCopy (ent->currentorigin, ent->previousorigin);
		VectorCopy (ent->origin,  ent->currentorigin);
		lerpfrac = 0;
	}
	else
	{
		lerpfrac = timepassed / 0.1;
		if (cl.paused || lerpfrac > 1)
			lerpfrac = 1;
	}

	VectorSubtract (ent->currentorigin, ent->previousorigin, d);

	eglTranslatef (ent->previousorigin[0] + (lerpfrac * d[0]), ent->previousorigin[1] + (lerpfrac * d[1]), ent->previousorigin[2] + (lerpfrac * d[2]));

	// orientation interpolation (Euler angles, yuck!)
	timepassed = cl.time - ent->movelerpstart;

	if (ent->movelerpstart == 0 || timepassed > 1)
	{
		ent->movelerpstart = cl.time;
		VectorCopy (ent->angles, ent->previousangles);
		VectorCopy (ent->angles, ent->currentangles);
	}

	if (!VectorCompare (ent->angles, ent->currentangles))
	{
		ent->movelerpstart = cl.time;
		VectorCopy (ent->currentangles, ent->previousangles);
		VectorCopy (ent->angles,  ent->currentangles);
		lerpfrac = 0;
	}
	else
	{
		lerpfrac = timepassed / 0.1;
		if (cl.paused || lerpfrac > 1)
			lerpfrac = 1;
	}

	VectorSubtract (ent->currentangles, ent->previousangles, d);

	// always interpolate along the shortest path
	for (i=0 ; i<3 ; i++)
	{
		if (d[i] > 180)
			d[i] -= 360;
		else if (d[i] < -180)
			d[i] += 360;
	}

	eglRotatef (ent->previousangles[1] + (lerpfrac * d[1]), 0, 0, 1);
	eglRotatef (-ent->previousangles[0] + (-lerpfrac * d[0]), 0, 1, 0);
	eglRotatef (ent->previousangles[2] + (lerpfrac * d[2]), 1, 0, 0);



}
