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
// gl_rmisc.c
// Baker: Validated 6-27-2011.  Boring functionary changes.


#include "quakedef.h"

#if SUPPORTS_TEXTURE_POINTER

// ///////////////////////////////////////////////////////////////////////////

// Baker: this takes the entity origin
// Now we need to get a surface collision and have an XYZ

static msurface_t	*test_collision_surface;
static vec3_t		test_collision_spot;


static qbool TexturePointer_SurfacePoint_NodeCheck_Recursive (mnode_t *node, vec3_t start, vec3_t end)
{
	float	front, back, frac;
	vec3_t	mid;

	

	if (!node)	return false;	// Baker: I guess ... why is this happening?
loc0:	
	if (node->contents <0 /*== CONTENTS_EMPTY || node->contents == CONTENTS_SOLID*/ /* < 0 */)	// Baker: special contents ... I'm not sure this should be a fail here except if contents empty
		return false;		// didn't hit anything

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
		goto loc0;
	}

	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0]) * frac;
	mid[1] = start[1] + (end[1] - start[1]) * frac;
	mid[2] = start[2] + (end[2] - start[2]) * frac;

// go down front side
	if (TexturePointer_SurfacePoint_NodeCheck_Recursive(node->children[front < 0], start, mid))
	{
//		Baker: We've got a clue here ... but only gives texture number
//		SetWindowText(engine.platform.mainwindow, va("%s i %i j %i", "string", i,j));
//		s->texinfo->texture->gl_texturenum
		return true;	// hit something
	}
	else
	{
		int		i, ds, dt;
		msurface_t	*surf;

	// check for impact on this node
		VectorCopy (mid, test_collision_spot);

		surf = cl.worldmodel->surfaces + node->firstsurface;
		for (i = 0 ; i < node->numsurfaces ; i++, surf++)
		{
//			if (surf->flags & SURF_PLANEBACK)
//				continue;	// no lightmaps

// Baker: We are trying to calculate a distance here.  Maybe even get a better SURF_PLANEBACK match

#if 0 // Light bleed through fix?
			float dist = DotProduct (/*light->origin*/ mid, surf->plane->normal) - surf->plane->dist;		// JT030305 - fix light bleed through
			int sidebit;

			if (dist >= 0)
				sidebit = 0;
			else
				sidebit = SURF_PLANEBACK;

			if ( (surf->flags & SURF_PLANEBACK) != sidebit ) //Discoloda
				continue;
#endif
			// Better impact point?  I'll settle for a better collision for the moment
//			for (j=0 ; j<3 ; j++)
//				impact[j] = light->origin[j] - surf->plane->normal[j]*dist;



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
			test_collision_surface = surf;

			return true;	// success
		}

	// go down back side
		return TexturePointer_SurfacePoint_NodeCheck_Recursive (node->children[front >= 0], mid, end);
	}
}

void VectorAdjustPairForEntity (const entity_t *pe, const vec3_t startpoint, const vec3_t endpoint, vec3_t adjusted_startpoint, vec3_t adjusted_endpoint, vec3_t net1, vec3_t net2)
{
	// No change scenario
	if (!(pe->origin[0] || pe->origin[1] || pe->origin[2] || pe->angles[0] || pe->angles[1] || pe->angles[2]))
	{
		VectorCopy (startpoint, adjusted_startpoint);
		VectorCopy (endpoint,   adjusted_endpoint);
		VectorClear (net1);
		VectorClear (net2);
		return;
	}
	
	// Add entity origin into startpoint and endpoint

	VectorSubtract (startpoint, pe->origin, adjusted_startpoint);
	VectorSubtract (endpoint,   pe->origin, adjusted_endpoint);

	// Make further adjustments if entity is rotated
	if (pe->angles[0] || pe->angles[1] || pe->angles[2])
	{
		vec3_t f, r, u, temp;
		AngleVectors(pe->angles, f, r, u);	// split entity angles to forward, right, up

		VectorCopy(adjusted_startpoint, temp);
		adjusted_startpoint[0] = DotProduct(temp, f);
		adjusted_startpoint[1] = -DotProduct(temp, r);
		adjusted_startpoint[2] = DotProduct(temp, u);

		VectorCopy(adjusted_endpoint, temp);
		adjusted_endpoint[0] = DotProduct(temp, f);
		adjusted_endpoint[1] = -DotProduct(temp, r);
		adjusted_endpoint[2] = DotProduct(temp, u);

	}

	VectorSubtract (startpoint, adjusted_startpoint, net1);	// How much did we change?  Well it will be pe->origin
	VectorSubtract (endpoint,   adjusted_endpoint  , net2);	// How much did we change?  Well it will be pe->origin

}

msurface_t	*texturepointer_surface;
vec3_t		 texturepointer_spot;		// Impact point
char		*texturepointer_name;

//texture_t	*oldtexture;

int TexturePointer_SurfacePoint (void)
{
	vec3_t		viewer, forward, up, right, destination;
	int			i;

	float		best_dist=0;
	vec3_t		best_spot;
	msurface_t	*best_surface;

	// Setup
	VectorCopy (r_refdef.vieworg, viewer);				// Player eye position
	AngleVectors (r_refdef.viewangles, forward, right, up);	// Take into account the angles
	VectorMultiplyAdd (viewer, 4096, forward, destination);		// Walk us forward


	if (TexturePointer_SurfacePoint_NodeCheck_Recursive (cl.worldmodel->nodes, viewer, destination))
	{
		extern vec_t VectorsLength2 (const vec3_t v1, const vec3_t v2);

		best_dist = VectorsLength2(viewer, test_collision_spot);
		VectorCopy (test_collision_spot, best_spot);
		best_surface = test_collision_surface;
	}

	for (i=1 ; i< cl_numvisedicts ; i++)	//0 is world...
	{	
		entity_t	*pe = cl_visedicts[i]; //&cl_entities[i];
		float		test_dist;
		vec3_t		adjusted_viewer, adjusted_destination, adjusted_net_player, adjusted_net_destination;

		if (!pe->model || !(pe->model->surfaces == cl.worldmodel->surfaces))
			continue;   // no model for ent or model isnt part of world

		// Baker: We need to adjust the point locations for entity position
		VectorAdjustPairForEntity (pe, viewer, destination, adjusted_viewer, adjusted_destination, adjusted_net_player, adjusted_net_destination);
		
		if (TexturePointer_SurfacePoint_NodeCheck_Recursive (pe->model->nodes+pe->model->hulls[0].firstclipnode, adjusted_viewer, adjusted_destination)
			&& (test_dist = VectorsLength2(adjusted_viewer, test_collision_spot))<best_dist)
		{
				// Update best surface
				best_dist = test_dist;
				VectorAdd (test_collision_spot, pe->origin, best_spot); // Adjusted, I hope ...
				best_surface = test_collision_surface;
		}
	}

	if (!best_dist)
		return false; // Failure - should we deselect stuff?

	if (best_surface  /*&& best_surface != texturepointer_surface*/)
	{
		// If there is a collision wall AND either there is no previous or the previous doesn't match, we have a change.
	
		// Undo old ..
		if (texturepointer_surface)
			texturepointer_surface->flags &= ~ SURF_SELECTED;	// Remove flag

		// Do new
		texturepointer_surface = best_surface;
		texturepointer_surface->flags |= SURF_SELECTED;

		VectorCopy (best_spot, texturepointer_spot);

		// Save name
		texturepointer_name = texturepointer_surface->texinfo->texture->name;
	}
	
	return true;
}

void TexturePointer_NewMap (void)
{
	VectorClear (texturepointer_spot);		// Impact point

	texturepointer_surface = NULL;
}


////////////////////////////////////////////////////////////////////////////////
#endif