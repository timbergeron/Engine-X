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
=================
R_CullBox

Returns true if the box is completely outside the frustum
=================
*/
/*
=================
R_CullBox -- replaced with new function from lordhavoc

Returns true if the box is completely outside the frustum
=================
*/
qbool R_CullBox (const vec3_t emins, const vec3_t emaxs)
{
	int i;
	mplane_t *p;
	for (i = 0;i < 4;i++)
	{
		p = frustum + i;
		switch(p->signbits)
		{
		default:
		case 0:
			if (p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2] < p->dist)
				return true;
			break;
		case 1:
			if (p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2] < p->dist)
				return true;
			break;
		case 2:
			if (p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2] < p->dist)
				return true;
			break;
		case 3:
			if (p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2] < p->dist)
				return true;
			break;
		case 4:
			if (p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2] < p->dist)
				return true;
			break;
		case 5:
			if (p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2] < p->dist)
				return true;
			break;
		case 6:
			if (p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2] < p->dist)
				return true;
			break;
		case 7:
			if (p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2] < p->dist)
				return true;
			break;
		}
	}
	return false;
}

/*
=================
R_CullSphere

Returns true if the sphere is completely outside the frustum
=================
*/
qbool R_CullSphere (const vec3_t centre, const float radius)
{
	int		i;
	mplane_t	*p;

	for (i=0, p=frustum ; i<4 ; i++, p++)
	{
		if (PlaneDiff(centre, p) <= -radius)
			return true;
	}

	return false;
}

qbool R_CullForEntity (const entity_t *ent/*, vec3_t returned_center*/)
{
	vec3_t		mins, maxs;

	if (ent->angles[0] || ent->angles[1] || ent->angles[2])
		return R_CullSphere(ent->origin, ent->model->radius);		// Angles turned; do sphere cull test

	// Angles all 0; do box cull test

	VectorAdd (ent->origin, ent->model->mins, mins);			// Add entity origin and model mins to calc mins
	VectorAdd (ent->origin, ent->model->maxs, maxs);			// Add entity origin and model maxs to calc maxs

//	if (returned_center)
//		LerpVector (mins, maxs, 0.5, returned_center);
	return R_CullBox(mins, maxs);

}

int SignbitsForPlane (mplane_t *out)
{
	int	bits, j;

	// for fast box on planeside test

	bits = 0;
	for (j=0 ; j<3 ; j++)
	{
		if (out->normal[j] < 0)
			bits |= 1<<j;
	}
	return bits;
}

/*
===============
TurnVector -- johnfitz

turn forward towards side on the plane defined by forward and side
if angle = 90, the result will be equal to side
assumes side and forward are perpendicular, and normalized
to turn away from side, use a negative angle
===============
*/
void TurnVector (vec3_t out, const vec3_t forward, const vec3_t side, float angle)
{
	float scale_forward, scale_side;

	scale_forward = cos( DEG2RAD( angle ) );
	scale_side = sin( DEG2RAD( angle ) );

	out[0] = scale_forward*forward[0] + scale_side*side[0];
	out[1] = scale_forward*forward[1] + scale_side*side[1];
	out[2] = scale_forward*forward[2] + scale_side*side[2];
}


/*
=============
GL_SetFrustum -- johnfitz -- written to replace MYgluPerspective
=============
*/
#define NEARCLIP 1
float frustum_skew = 0.0; //used by r_stereo
void GL_SetFrustum(float fovx, float fovy)
{
	float xmax, ymax;
	xmax = NEARCLIP * tan( fovx * M_PI / 360.0 );
	ymax = NEARCLIP * tan( fovy * M_PI / 360.0 );
	eglFrustum(-xmax, xmax, -ymax, ymax, NEARCLIP, scene_farclip.floater);
}


/*
===============
R_SetFrustum
===============
*/
void Frustum_ViewSetup_SetFrustum (const float fovx, const float fovy)
{
	int	i;

	if (scene_lockpvs.integer) return; // No frustum updates

	TurnVector(frustum[0].normal, vpn, vright, fovx/2 - 90); //left plane
	TurnVector(frustum[1].normal, vpn, vright, 90 - fovx/2); //right plane
	TurnVector(frustum[2].normal, vpn, vup, 90 - fovy/2); //bottom plane
	TurnVector(frustum[3].normal, vpn, vup, fovy/2 - 90); //top plane

	for (i=0 ; i<4 ; i++)
	{
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct (r_origin, frustum[i].normal); //FIXME: shouldn't this always be zero?
		frustum[i].signbits = SignbitsForPlane (&frustum[i]);
	}
}


/*
=============
R_Clear
=============
*/
qbool gl_clear_for_frame;
void R_Clear (void)
{
	MeglClearColor (0, 0, 0, 0);	//Black by default

	gl_clear_for_frame = !!gl_clear.integer;	// If gl_clear is 0, gl_clear_for frame is 0; otherwise gl_clear_for_frame is 1.

	if (gl_clear.integer == -1) // However ... -1 is special case
	{
		// Automatic adaptive gl_clear
		// We will clear IF ... r_lockpvs is true or noclip is on ... Because those aren't "serious" options and tend to see HOM
		if (scene_lockpvs.integer || (sv.active && sv_player->v.movetype == MOVETYPE_NOCLIP))
		{
			gl_clear_for_frame = true;
			if (scene_lockpvs.integer)
				MeglClearColor (.25, .12, 0, 0);	//Orange
			else
				MeglClearColor (0, 0.05, 0.05,0); // Cyan
		}
		else
			gl_clear_for_frame = false;
	}




	// Works extremely poorly in Direct3D if the window is resized on-the-fly
	if (gl_clear_for_frame)
		eglClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	else
		eglClear (GL_DEPTH_BUFFER_BIT);
}



extern float r_fovx, r_fovy; //johnfitz -- rendering fov may be different becuase of r_waterwarp and r_stereo

/*
=============
R_SetupGL
=============
*/
void R_SetupGL (void)
{
	float	screenaspect;
	int	x, x2, y2, y, w, h;//, farclip;

#define screen_width vid.width //(developer.integer ? glwidth : vid.width)
#define screen_height vid.height // (developer.integer ? glheight : vid.height)


	// set up viewpoint
	eglMatrixMode (GL_PROJECTION);
	eglLoadIdentity ();

#if 1	// Sadly, this isn't doing jack for many reasons, but the code will be useful and it isn't hurting us
	x = r_refdef.vrect.x * glwidth/screen_width;
	x2 = (r_refdef.vrect.x + r_refdef.vrect.width) * glwidth/screen_width;
	y = (screen_height-r_refdef.vrect.y) * glheight/screen_height;
	y2 = (screen_height - (r_refdef.vrect.y + r_refdef.vrect.height)) * glheight/screen_height;

	w = x2 - x;
	h = y - y2;

	// Baker: 3D rendering phase can only draw in own window.  Cannot draw on status bar or anywhere else.
	eglViewport (glx + x, gly + y2, w, h);

	// Baker: Wow.  We calculate this to not use it.  And we ignore what we just did above.  Groovy
	screenaspect = (float)r_refdef.vrect.width/r_refdef.vrect.height;
#endif
//	yfov = 2*atanf((float)r_refdef.vrect.height/r_refdef.vrect.width)*180/M_PI;
//	farclip = max((int)scene_farclip.floater, 4096);
	GL_SetFrustum (r_fovx, r_fovy);  // old GLQUAKE is farclip 4096

	MeglCullFace (GL_FRONT);

	eglMatrixMode (GL_MODELVIEW);
	eglLoadIdentity ();

	eglRotatef (-90, 1, 0, 0);	    // put Z going up
	eglRotatef (90, 0, 0, 1);	    // put Z going up


#if SUPPORTS_XFLIP
    if (scene_invert_x.integer)  //Atomizer - GL_XFLIP
	{
		eglScalef (1, -1, 1);
		MeglCullFace(GL_BACK);
	}
#endif

	eglRotatef (-r_refdef.viewangles[2], 1, 0, 0);
	eglRotatef (-r_refdef.viewangles[0], 0, 1, 0);
	eglRotatef (-r_refdef.viewangles[1], 0, 0, 1);
	eglTranslatef (-r_refdef.vieworg[0], -r_refdef.vieworg[1], -r_refdef.vieworg[2]);

	eglGetFloatv (GL_MODELVIEW_MATRIX, r_world_matrix); // We get this.  But so far we are not using it.

	// set drawing parms
	if (gl_cull.integer)
		MeglEnable (GL_CULL_FACE);
	else
		MeglDisable (GL_CULL_FACE);

	MeglDisable (GL_BLEND);
	MeglDisable (GL_ALPHA_TEST);
	MeglEnable (GL_DEPTH_TEST);

// Baker: It takes tremendously large numbers like 100,000 to make a dent in fps
//        So creating a tracking system isn't going to get us fps
//        But it will definitely save headaches.
//	{
//		int i;
//		for (i = 0; i < r_pass_caustics.integer; i++)
//		{
//			MeglEnable (GL_BLEND);
//			MeglDisable (GL_BLEND);
//		}
//	}
}

/*
===============
R_MarkLeaves
===============
*/
void Frustum_ViewSetup_MarkVisibleLeaves (void)
{
	int		i;
	byte	*vis, solid[MAX_MAP_LEAFS/8];
	mnode_t	*node;

	if (scene_lockpvs.integer)
		return;		// Do not update!


	if ((!scene_novis.integer || pq_cheatfree) && r_oldviewleaf == r_viewleaf && r_oldviewleaf2 == r_viewleaf2)	// watervis hack
		return;

	r_visframecount++;
	r_oldviewleaf = r_viewleaf;

	if (scene_novis.integer && !pq_cheatfree) // JPG 3.20 - cheat protection
	{
		vis = solid;
		memset (solid, 0xff, (cl.worldmodel->numleafs+7)>>3);
	}
	else
	{
		vis = Mod_LeafPVS (r_viewleaf, cl.worldmodel);

		if (r_viewleaf2)
		{
			int		i, count;
			unsigned	*src, *dest;

			// merge visibility data for two leafs
			count = (cl.worldmodel->numleafs+7)>>3;
			memcpy (solid, vis, count);
			src = (unsigned *) Mod_LeafPVS (r_viewleaf2, cl.worldmodel);
			dest = (unsigned *) solid;
			count = (count + 3)>>2;
			for (i=0 ; i<count ; i++)
				*dest++ |= *src++;
			vis = solid;
		}
	}

	for (i=0 ; i<cl.worldmodel->numleafs ; i++)
	{
		if (vis[i>>3] & (1 << (i & 7)))
		{
			node = (mnode_t *)&cl.worldmodel->leafs[i+1];
			do {
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}
}

void Frustum_NewMap (void)
{
	r_framecount = 1;		// no dlightcache; also used for vis
	r_viewleaf = NULL;		// no viewpoint
}
