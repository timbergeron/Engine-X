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
// chase.c -- chase camera code

#include "quakedef.h"
//#include "d3d_model.h"
//#include "d3d_quake.h"

//bool SV_RecursiveHullCheck (hull_t *hull, int num, float p1f, float p2f, vec3_t p1, vec3_t p2, trace_t *trace);

vec3_t	chase_dest;





void Chase_Reset (void)
{
	// for respawning and teleporting
	//	start position 12 units behind head
}

void CL_TraceLine (vec3_t start, vec3_t end, vec3_t impact)
{
	trace_t	trace;

	memset (&trace, 0, sizeof (trace));
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);

	VectorCopy (trace.endpos, impact);
}

/*
static qbool ChaseCam_CollisionSpot_Recursive (const mnode_t *node, const vec3_t start, const vec3_t end, vec3_t impact)
{
	float	front, back, frac;
	vec3_t	mid;

	do
	{

		// Baker: For this we do want collisions with liquids or sky
		if (node->contents == CONTENTS_EMPTY)
			return false;
		
		if (node->contents == CONTENTS_SOLID)	
		{
			// Worst kind of hit.
			VectorCopy (start, impact);
			return false;		// didn't hit anything
		
		}
	// calculate mid point
		if (node->plane->type < 3)
		{
			front	= start[node->plane->type] - node->plane->dist;
			back	= end[node->plane->type] - node->plane->dist;
		}
		else
		{
			front	= DotProduct(start, node->plane->normal) - node->plane->dist;
			back	= DotProduct(end, node->plane->normal) - node->plane->dist;
		}

		// optimized recursion
		if ((back < 0) == (front < 0))
		{
			node = node->children[front < 0];
			continue; // goto loc0;
		}

		frac = front / (front-back);
		mid[0] = start[0] + (end[0] - start[0]) * frac;
		mid[1] = start[1] + (end[1] - start[1]) * frac;
		mid[2] = start[2] + (end[2] - start[2]) * frac;

		// go down front side
		if (ChaseCam_CollisionSpot_Recursive(node->children[front < 0], start, mid, impact))
			return true;	// hit something
	
		// didn't hit something
	
		{
			// check for impact on this node
			int		i, ds, dt;
			msurface_t	*surf =  cl.worldmodel->surfaces + node->firstsurface;

	
			VectorCopy (mid, impact);
		
			for (i = 0 ; i < node->numsurfaces ; i++, surf++)
			{
				// Baker: We want all surfaces including sky and water, etc.
	//			if (surf->flags & SURF_DRAWTILED)
	//				continue;	// no lightmaps
	#if 1
				ds = (int)((float)DotProduct (mid, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3]);
				dt = (int)((float)DotProduct (mid, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3]);

				if (ds < surf->texturemins[0] || dt < surf->texturemins[1])
					continue;

				ds -= surf->texturemins[0];
				dt -= surf->texturemins[1];

				if (ds > surf->extents[0] || dt > surf->extents[1])
					continue;
	#endif
				// At this point we have a collision with this surface

				return true;	// success
			}
		}

		// go down back side
		return ChaseCam_CollisionSpot_Recursive (node->children[front >= 0], mid, end, impact);

		break;
	} while (1);
}

static qbool ChaseCam_TraceLine (const vec3_t viewer, const vec3_t chasecam_tryspot, vec3_t impact)
{
	int		viewcontents	= (Mod_PointInLeaf (viewer, cl.worldmodel))->contents;
	qbool	hitwall			= ChaseCam_CollisionSpot_Recursive(cl.worldmodel->nodes, viewer, chasecam_tryspot, impact);

	// If we didn't hit a wall, then the tryspot is what we've got
	if (!hitwall)	VectorCopy (chasecam_tryspot, impact);

	return hitwall; 
/*
	if ((Mod_PointInLeaf (impact, cl.worldmodel))->contents != viewcontents)
	{
		Con_Printf ("Chase dest problem.  Hitwall is %i\n", hitwall);
	}
	else
	{
		Con_Printf ("No chasedest problem.  Hitwall is %i\n", hitwall);
	}

	if (!hitwall) return false;

	// Hit wall


	return true;

}
*/

#define CHASE_DEST_OFFSET 4.0f
qbool chase_nodraw;
int chase_alpha;

qbool Chase_CheckBrushEdict (entity_t *e, vec3_t checkpoint, int viewcontents)
{
	vec3_t mins;
	vec3_t maxs;

	// don't check against self
	if (e == &cl_entities[cl.player_point_of_view_entity]) return true;

	// let's not clip against these types
	if (e->model->modelformat != mod_brush) return true;

	if (e->model->name[0] != '*') return true;

	// derive the bbox
	if (e->angles[0] || e->angles[1] || e->angles[2])
	{
		// copied from R_CullBox rotation code for inline bmodels, loop just unrolled
		// (this is no longer really valid - oh well; the quake chasecam is a hack anyway)
		mins[0] = e->origin[0] - e->model->radius;
		maxs[0] = e->origin[0] + e->model->radius;
		mins[1] = e->origin[1] - e->model->radius;
		maxs[1] = e->origin[1] + e->model->radius;
		mins[2] = e->origin[2] - e->model->radius;
		maxs[2] = e->origin[2] + e->model->radius;
	}
	else
	{
		VectorAdd (e->origin, e->model->mins, mins);
		VectorAdd (e->origin, e->model->maxs, maxs);
	}

	// check against bbox
	if (checkpoint[0] < mins[0]) return true;
	if (checkpoint[1] < mins[1]) return true;
	if (checkpoint[2] < mins[2]) return true;
	if (checkpoint[0] > maxs[0]) return true;
	if (checkpoint[1] > maxs[1]) return true;
	if (checkpoint[2] > maxs[2]) return true;

	// blocked
	return false;
}


qbool Chase_Check (vec3_t checkpoint, int viewcontents)
{
	int i;

	// check against world model - going into different contents
	if ((Mod_PointInLeaf (checkpoint, cl.worldmodel))->contents != viewcontents) return false;

	// check visedicts - this happens *after* CL_ReadFromServer so the list will be valid
	// (may not include static entities) (but only checks brush models)
	for (i = 0; i < cl_numvisedicts; i++)
		if (!Chase_CheckBrushEdict (cl_visedicts[i], checkpoint, viewcontents)) return false;

	// it's good now
	return true;
}


void Chase_Adjust (vec3_t chase_dest)
{
	// calculate distance between chasecam and original org to establish number of tests we need.
	// an int is good enough here.:)  add a cvar multiplier to this...
	int num_tests = sqrt ((r_refdef.vieworg[0] - chase_dest[0]) * (r_refdef.vieworg[0] - chase_dest[0]) +
		(r_refdef.vieworg[1] - chase_dest[1]) * (r_refdef.vieworg[1] - chase_dest[1]) +
		(r_refdef.vieworg[2] - chase_dest[2]) * (r_refdef.vieworg[2] - chase_dest[2])) * 1 /*chase_scale.floater*/;

	// take the contents of the view leaf
	int viewcontents = (Mod_PointInLeaf (r_refdef.vieworg, cl.worldmodel))->contents;
	int best;

	// move along path from r_refdef.vieworg to chase_dest
	for (best = 0; best < num_tests; best++)
	{
		vec3_t chase_newdest;

		chase_newdest[0] = r_refdef.vieworg[0] + (chase_dest[0] - r_refdef.vieworg[0]) * best / num_tests;
		chase_newdest[1] = r_refdef.vieworg[1] + (chase_dest[1] - r_refdef.vieworg[1]) * best / num_tests;
		chase_newdest[2] = r_refdef.vieworg[2] + (chase_dest[2] - r_refdef.vieworg[2]) * best / num_tests;

		// check for a leaf hit with different contents
		if (!Chase_Check (chase_newdest, viewcontents))
		{
			// go back to the previous best as this one is bad
			if (best > 1)
				best--;
			else best = num_tests;

			break;
		}
	}

	{
		// certain surfaces can be viewed at an oblique enough angle that they are partially clipped
		// by znear, so now we fix that too...
		int chase_vert[] = {0, 0, 1, 1, 2, 2};
		int dest_offset[] = {CHASE_DEST_OFFSET, -CHASE_DEST_OFFSET};

		// move along path from chase_dest to r_refdef.vieworg
		// this one will early-out the vast majority of cases
		for (; best >= 0; best--)
		{
			// number of matches
			int nummatches = 0;
			int	test;

			// adjust
			chase_dest[0] = r_refdef.vieworg[0] + (chase_dest[0] - r_refdef.vieworg[0]) * best / num_tests;
			chase_dest[1] = r_refdef.vieworg[1] + (chase_dest[1] - r_refdef.vieworg[1]) * best / num_tests;
			chase_dest[2] = r_refdef.vieworg[2] + (chase_dest[2] - r_refdef.vieworg[2]) * best / num_tests;

			// run 6 tests: -x/+x/-y/+y/-z/+z
			for (test = 0; test < 6; test++)
			{
				// adjust, test and put back.
				chase_dest[chase_vert[test]] -= dest_offset[test & 1];

				if (Chase_Check (chase_dest, viewcontents)) nummatches++;

				chase_dest[chase_vert[test]] += dest_offset[test & 1];
			}

			// test result, if all match we're done in here
			if (nummatches == 6) break;
		}
	}

	{
		float chase_length = (r_refdef.vieworg[0] - chase_dest[0]) * (r_refdef.vieworg[0] - chase_dest[0]);
		chase_length += (r_refdef.vieworg[1] - chase_dest[1]) * (r_refdef.vieworg[1] - chase_dest[1]);
		chase_length += (r_refdef.vieworg[2] - chase_dest[2]) * (r_refdef.vieworg[2] - chase_dest[2]);

		if (chase_length < 150)
		{
			chase_nodraw = true;
			chase_alpha = 255;
		}
		else
		{
			chase_nodraw = false;
			chase_alpha = (chase_length - 150);

			if (chase_alpha > 255) chase_alpha = 255;
		}
	}
}


void Chase_Update (void)
{
//	int		i;
	float	dist;
	vec3_t  forward, right, up;
	vec3_t	dest, stop;
	vec3_t	chase_dest;

	// if can't see player, reset
	AngleVectors (cl.viewangles, forward, right, up);

	// calc exact destination
	
	if (chase_active.integer >=2)
	{	// Hard location
		chase_dest[0] = r_refdef.vieworg[0] + chase_back.floater;
        chase_dest[1] = r_refdef.vieworg[1] + chase_right.floater;
	}
	else
	{	// Altered for player "looking at" point
		chase_dest[0] = r_refdef.vieworg[0] - forward[0] * chase_back.floater - right[0] * chase_right.floater;
		chase_dest[1] = r_refdef.vieworg[1] - forward[1] * chase_back.floater - right[1] * chase_right.floater;
	}
	chase_dest[2] = r_refdef.vieworg[2] + chase_up.floater;
	
	// Adjust it so it isn't sticking in a wall

	Chase_Adjust (chase_dest);

	// store alpha to entity
	cl_entities[cl.player_point_of_view_entity].alpha = chase_alpha;

	// Angles
	if (chase_active.integer >=2)
	{
        r_refdef.viewangles[ROLL] = chase_roll.floater;
        r_refdef.viewangles[PITCH] = chase_pitch.floater;
		r_refdef.viewangles[YAW] = chase_yaw.floater;
	}
	else
	{
		// find the spot the player is looking at
		VectorMultiplyAdd (r_refdef.vieworg, 4096, forward, dest);
		CL_TraceLine (r_refdef.vieworg, dest, stop);
//		ChaseCam_TraceLine (r_refdef.vieworg, dest, stop);

		// calculate pitch to look at the same spot from camera
		VectorSubtract (stop, r_refdef.vieworg, stop);
		dist = DotProduct (stop, forward);

		if (dist < 1) dist = 1;

		r_refdef.viewangles[PITCH] = -atan (stop[2] / dist) / M_PI * 180;
		r_refdef.viewangles[YAW] -= chase_yaw.floater;

	}

	// move towards destination
	VectorCopy (chase_dest, r_refdef.vieworg);
}


void Chase_Init (void)
{
	Cvar_Registration_Client_Chase ();

}