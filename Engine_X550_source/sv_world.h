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
// sv_world.h

#ifndef __SV_WORLD_H__
#define __SV_WORLD_H__


#define	MOVE_NORMAL	0
#define	MOVE_NOMONSTERS	1
#define	MOVE_MISSILE	2

typedef struct areanode_s
{
	int	axis;		// -1 = leaf node
	float	dist;
	struct areanode_s	*children[2];
	link_t	trigger_edicts;
	link_t	solid_edicts;
} areanode_t;

typedef struct
{
	vec3_t	normal;
	float	dist;
} plane_t;

typedef struct
{
	qbool		allsolid;	// if true, plane is not valid
	qbool		startsolid;	// if true, the initial point was in a solid area
	qbool		inopen, inwater;
	float		fraction;	// time completed, 1.0 = didn't hit anything
	vec3_t		endpos;		// final position
	plane_t		plane;		// surface normal at impact
	edict_t		*ent;		// entity the surface is on
} trace_t;

#define	AREA_DEPTH	4
#define	AREA_NODES	32

//extern	entity_t	r_worldentity;


void SV_ClearWorld (void);
// called after the world model has been loaded, before linking any entities

void SV_UnlinkEdict (edict_t *ent);
// call before removing an entity, and before trying to move one,
// so it doesn't clip against itself
// flags ent->v.modified

void SV_LinkEdict (edict_t *ent, qbool touch_triggers);
// Needs to be called any time an entity changes origin, mins, maxs, or solid
// flags ent->v.modified
// sets ent->v.absmin and ent->v.absmax
// if touchtriggers, calls prog functions for the intersected triggers

int SV_HullPointContents (const hull_t *hull, const int num, const vec3_t p);
int SV_PointContents (const vec3_t p);

// returns the CONTENTS_* value from the world at the given point.
// does not check any entities at all

qbool SV_RecursiveHullCheck (const hull_t *hull, const int num, const float p1f, const float p2f, const vec3_t p1, const vec3_t p2, trace_t *trace);
edict_t	*SV_TestEntityPosition (edict_t *ent);

trace_t SV_Move (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int type, edict_t *passedict);
// mins and maxs are relative

// if the entire move stays in a solid volume, trace.allsolid will be set

// if the starting point is in a solid, it will be allowed to move out
// to an open area

// nomonsters is used for line of sight or edge testing, where mosnters
// shouldn't be considered solid objects

// passedict is explicitly excluded from clipping checks (normally NULL)

#endif // __SV_WORLD_H__

