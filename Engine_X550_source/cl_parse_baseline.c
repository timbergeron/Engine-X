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
// cl_parse.c -- parse a message received from the server

#include "quakedef.h"


/*
==================
CL_ParseBaseline
==================
*/
#if FITZQUAKE_PROTOCOL
void CL_ParseBaseline (entity_t *ent, int version) //johnfitz -- added argument
#else
void CL_ParseBaseline (entity_t *ent)
#endif
{
	int	i;
#if FITZQUAKE_PROTOCOL
	int bits; //johnfitz

	//johnfitz -- PROTOCOL_FITZQUAKE
	bits = (version == 2) ? MSG_ReadByte() : 0;
	ent->baseline.modelindex = (bits & B_LARGEMODEL) ? MSG_ReadShort() : MSG_ReadByte();
	ent->baseline.frame = (bits & B_LARGEFRAME) ? MSG_ReadShort() : MSG_ReadByte();
		//johnfitz
#else

	ent->baseline.modelindex = MSG_ReadByte ();
	ent->baseline.frame = MSG_ReadByte ();
#endif

	ent->baseline.colormap = MSG_ReadByte ();
	ent->baseline.skin = MSG_ReadByte ();
	for (i=0 ; i<3 ; i++)
	{
		ent->baseline.origin[i] = MSG_ReadCoord ();
		ent->baseline.angles[i] = MSG_ReadAngle ();
	}

#if FITZQUAKE_PROTOCOL
	ent->baseline.alpha = (bits & B_ALPHA) ? MSG_ReadByte() : ENTALPHA_DEFAULT; //johnfitz -- PROTOCOL_FITZQUAKE
#endif
// Baker: FitzQuake interpolation LERP reset
// Baker: I believe this one is the ONE that fixes loadgame problems
//	ent->frame_start_time = ent->rotate_start_time = ent->translate_start_time = 0;
#if 0 // OLD INTERPOLATE
	ent->lastframevalid = 0;
#endif
}
