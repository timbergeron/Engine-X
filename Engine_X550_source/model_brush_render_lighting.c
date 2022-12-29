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
// r_light.c

#include "quakedef.h"

static int r_dlightframecount;

static const int	blocklight_bytes = 3;  // Assigned at level load so this is ok



/*
=============================================================================

DYNAMIC LIGHTS BLEND RENDERING

=============================================================================
*/

static float	bubble_sintable[17], bubble_costable[17];

static void sLight_Init_FlashBlend_Bubble (void)
{
	int	i;
	float	a, *bub_sin, *bub_cos;

	bub_sin = bubble_sintable;
	bub_cos = bubble_costable;

	for (i=16 ; i>=0 ; i--)
	{
		a = i/16.0 * M_PI*2;
		*bub_sin++ = sinf(a);
		*bub_cos++ = cosf(a);
	}
}

float bubblecolor[NUM_DLIGHTTYPES][4] =
{
	{0.2, 0.1, 0.05},		// dimlight or brightlight (lt_default)
	{0.2, 0.1, 0.05},		// muzzleflash
	{0.2, 0.1, 0.05},		// explosion
	{0.2, 0.1, 0.05},		// rocket
	{0.5, 0.05, 0.05},		// red
	{0.05, 0.05, 0.3},		// blue
	{0.5, 0.05, 0.4}		// red + blue
};




/*
==================
R_AnimateLight
==================
*/
static int light_valuetable[256];

static void sLight_NewMap_ResetLightStyles (void)
{
	int	i;
//	for (i=0 ; i<256 ; i++)
//		d_lightstylevalue[i] = 264;		// normal light value
	// correct lightscaling so that 'm' is 256, not 264
	for (i = 0; i < 256; i++)
	{
		// make it explicit that yeah, it's a *signed* char we want to use here
		// so that values > 127 will behave as expected
		float fv = (float) ((signed char) i - 'a') * 22;

		fv *= 256.0f;
		fv /= 264.0f;

		// in general a mod should never provide values for these that are outside of the 'a'..'z' range,
		// but the argument could be made that ID Quake/QC does/did nothing to prevent it, so it's legal
		if (fv < 0)
			light_valuetable[i] = (int) (fv - 0.5f);
		else light_valuetable[i] = (int) (fv + 0.5f);
	}

	// normal light value - making this consistent with a value of 'm' in R_AnimateLight
	// will prevent the upload of lightmaps when a surface is first seen
	for (i = 0; i < 256; i++) d_lightstylevalue[i] = light_valuetable['m'];
}



static void sLight_FrameSetup_AnimateLight_Styles (void)
{
	int	j;//, k;
	if (light_lerplightstyle.integer)
	{
		float		l;

		// light animations
		// 'm' is normal light, 'a' is no light, 'z' is double bright
		int		flight = (int)floor(cl.ctime * 10);
		int		clight = (int)ceil(cl.ctime * 10);
		float	lerpfrac = (cl.ctime * 10) - flight;
		float	backlerp = 1.0 - lerpfrac;

		for (j = 0 ; j < MAX_LIGHTSTYLES ; j++)
		{
			if (!cl_lightstyle[j].length)
			{
				d_lightstylevalue[j] = light_valuetable['m'];
				continue;
			}
			else if (cl_lightstyle[j].length == 1)
			{
				// single length style so don't bother interpolating
				d_lightstylevalue[j] = light_valuetable[cl_lightstyle[j].map[0]];
				continue;
			}

			// interpolate animating light
			l = (float) light_valuetable[cl_lightstyle[j].map[flight % cl_lightstyle[j].length]] * backlerp;
			l += (float) light_valuetable[cl_lightstyle[j].map[clight % cl_lightstyle[j].length]] * lerpfrac;

			d_lightstylevalue[j] = (int)l;
		}
	}
	else
	{
		// old light animation
		int i = (int) (cl.time * 10.0f);

		// light animations
		// 'm' is normal light, 'a' is no light, 'z' is double bright

		for (j=0 ; j<MAX_LIGHTSTYLES ; j++)
		{
			if (!cl_lightstyle[j].length)
			{
				d_lightstylevalue[j] = light_valuetable['m'];
				continue;
			}
			else if (cl_lightstyle[j].length == 1)
			{
				// single length style so don't bother interpolating
				d_lightstylevalue[j] = light_valuetable[cl_lightstyle[j].map[0]];
				continue;
			}

			d_lightstylevalue[j] = light_valuetable[cl_lightstyle[j].map[i % cl_lightstyle[j].length]];
		}
	}
}

/*
=============================================================================

DYNAMIC LIGHTS BLEND RENDERING  (gl_flashblend 1)

=============================================================================
*/



static void sLight_Render_FlashBlend_Dlights_Bubble (dlight_t *light)
{
	int		i, j;
	vec3_t	v;
	vec3_t	v_right, v_up;
	float	length;
	float	rad;


	rad = light->radius * 0.35;

	VectorSubtract (light->origin, r_origin, v);
	if ((length = VectorNormalize (v)) < rad)
	{
// view is inside the dlight
// joe: this looks ugly, so I decided NOT TO use it...
//		V_AddLightBlend (1, 0.5, 0, light->radius * 0.0003);
		return;
	}

	eglBegin (GL_TRIANGLE_FAN);
	if (light->color_type == lt_explosion2 || light->color_type == lt_explosion3)
		MeglColor3fv (ExploColor); // Dynamic color
	else
		MeglColor3fv (bubblecolor[light->color_type]);  // Dynamic color

	VectorVectors(v, v_right, v_up);

	if (length - rad > 8)
		VectorScale (v, rad, v);
	else
	// make sure the light bubble will not be clipped by near z clip plane
		VectorScale (v, length - 8, v);

	VectorSubtract (light->origin, v, v);

	eglVertex3fv (v);
	MeglColor3ubv (color_black);

	{	float	*bub_sin = bubble_sintable;
		float	*bub_cos = bubble_costable;

		for (i=16; i>=0; i--)
		{

			for (j=0 ; j<3 ; j++)
				v[j] = light->origin[j] + (v_right[j]*(*bub_cos) + v_up[j]*(*bub_sin)) * rad;
			bub_sin++;
			bub_cos++;
			eglVertex3fv (v);
		}

		eglEnd ();
	}
}

static void sLight_Render_FlashBlend_Dlights_GLQuake (dlight_t *light)
{
	int		i, j;
	float	a;
	vec3_t	v;
	float	rad;

	rad = light->radius * 0.35;

	VectorSubtract (light->origin, r_origin, v);
	if (VectorLength (v) < rad)
//	{	// view is inside the dlight
//		AddLightBlend (1, 0.5, 0, light->radius * 0.0003);
		return;
//	}

	eglBegin (GL_TRIANGLE_FAN);
	MeglColor3f (0.2,0.1,0.0);
	for (i=0 ; i<3 ; i++)
		v[i] = light->origin[i] - vpn[i]*rad;
	eglVertex3fv (v);
	MeglColor3f (0,0,0);
	for (i=16 ; i>=0 ; i--)
	{
		a = i/16.0 * M_PI*2;
		for (j=0 ; j<3 ; j++)
			v[j] = light->origin[j] + vright[j]*cos(a)*rad
				+ vup[j]*sin(a)*rad;
		eglVertex3fv (v);
	}
	eglEnd ();
}


/*
=============
R_RenderDlights
=============
*/
void Light_Render_FlashBlend_Dlights (void)
{
	int		i;
	dlight_t	*l;

	if (!light_flashblend.integer)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
						// advanced yet for this frame
	MeglDepthMask (GL_FALSE); // GL_FALSE = 0
	MeglDisable (GL_TEXTURE_2D);
	MeglShadeModel (GL_SMOOTH);
	MeglEnable (GL_BLEND);
	MeglBlendFunc (GL_ONE, GL_ONE);

	l = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, l++)
	{
		if (l->die < cl.time || !l->radius) // light dead
			continue;

		if (light_flashblend.integer == 2)
			sLight_Render_FlashBlend_Dlights_GLQuake (l);
		else
		sLight_Render_FlashBlend_Dlights_Bubble (l);
	}

	MeglColor3ubv (color_white);
	MeglDisable (GL_BLEND);
	MeglEnable (GL_TEXTURE_2D);
	MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	MeglDepthMask (GL_TRUE); // gl_true=1
}


// joe: this is said to be faster, so I use it instead
static void sLight_MarkLights (dlight_t *light, int bit, mnode_t *node)
{
	mplane_t	*splitplane;
	float		dist;

	while (1)
	{
		if (node->contents < 0) return;

		splitplane = node->plane;
		dist = PlaneDiff(light->origin, splitplane);

		if (dist > light->radius)
		{
			node = node->children[0];
			continue;
		}

		if (dist < -light->radius)
		{
			node = node->children[1];
			continue;
		}

		break;
	}

	{
		// mark the polygons
		msurface_t	*surf = cl.worldmodel->surfaces + node->firstsurface;
		float	maxdist = light->radius * light->radius;

		vec3_t		impact;
		float		l;
		int			i, j, s, t, sidebit;

		for (i=0 ; i<node->numsurfaces ; i++, surf++)
		{
			// no lights on these
			if (surf->flags & SURF_DRAWTILED) continue;
#if 1 // Light bleed through fix?
			dist = DotProduct (light->origin, surf->plane->normal) - surf->plane->dist;		// JT030305 - fix light bleed through

			if (dist >= 0)
				sidebit = 0;
			else
				sidebit = SURF_PLANEBACK;

			if ( (surf->flags & SURF_PLANEBACK) != sidebit ) //Discoloda
				continue;
#endif

			for (j=0 ; j<3 ; j++)
				impact[j] = light->origin[j] - surf->plane->normal[j]*dist;

			// clamp center of light to corner and check brightness
			l = DotProduct(impact, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3] - surf->texturemins[0];
			s = l + 0.5;
			s = CLAMP (0, s, surf->extents[0]);
			s = l - s;
			l = DotProduct(impact, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3] - surf->texturemins[1];
			t = l + 0.5;
			t = CLAMP (0, t, surf->extents[1]);
			t = l - t;

			// compare to minimum light
			if ((s*s + t*t + dist*dist) < maxdist)
			{
				if (surf->dlightframe != r_dlightframecount)	// not dynamic until now
				{
					surf->dlightbits = bit;
					surf->dlightframe = r_dlightframecount;
				}
				else	// already dynamic
				{
					surf->dlightbits |= bit;
				}



			}
		}
		if (node->children[0]->contents >= 0) sLight_MarkLights (light, bit, node->children[0]);
		if (node->children[1]->contents >= 0) sLight_MarkLights (light, bit, node->children[1]);
	}
}

/*
=============
R_PushDlights
=============
*/

void Light_PushDlights_DrawModel_Brush_Ent (const model_t *clmodel)
{
	int k;
#ifdef _DEBUGG
	if (in_scene_setup == false)
		Con_Printf ("Scene violation: Light_PushDlights_DrawModel_Brush_Ent -> marking visible surfaces during rendering");
#endif

	if (light_flashblend.integer) return;

	for (k=0 ; k<MAX_DLIGHTS ; k++)
	{
		if (cl_dlights[k].die < cl.time || !cl_dlights[k].radius)
			continue;

		sLight_MarkLights (&cl_dlights[k], 1 << k, clmodel->nodes + clmodel->hulls[0].firstclipnode);
	}
}


static void sLight_FrameSetup_PushDlights (void)
{
	int			i;
	dlight_t	*l = cl_dlights;

	if (light_flashblend.integer) return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
											// advanced yet for this frame

	for (i=0 ; i<MAX_DLIGHTS ; i++, l++)
	{
		if (l->die < cl.time || !l->radius)
			continue;

		sLight_MarkLights (l, 1<<i, cl.worldmodel->nodes);
	}
}


/*
=============================================================================

LIGHT SAMPLING - used exclusively for getting alias model lighting info
                 also used for determining the shadow location

=============================================================================
*/


// Variables for most recent success
mplane_t	*lightpoint_lightplane;
vec3_t		 lightpoint_lightspot;
vec3_t		 lightpoint_lightcolor;
float		 lightpoint_distance;

static qbool sLight_LightPoint_Recursive (vec3_t color, const mnode_t *node, const vec3_t start, const vec3_t end)
{
	float	front, back, frac;
	vec3_t	mid;

loc0:
	// didn't hit anything (CONTENTS_EMPTY or CONTENTS_WATER, etc.)
	if (node->contents < 0)
		return false;

// calculate mid point
	if (node->plane->type < 3)
	{
		front = start[node->plane->type] - node->plane->dist;
		back = end[node->plane->type] - node->plane->dist;
	}
	else
	{
		front = DotProduct(start, node->plane->normal) - node->plane->dist;
		back = DotProduct(end, node->plane->normal) - node->plane->dist;
	}

	// LordHavoc: optimized recursion
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
	if (sLight_LightPoint_Recursive(color, node->children[front < 0], start, mid))
	{
		return true;	// hit something
	}
	else
	{
		int		i, ds, dt;
		msurface_t	*surf;

	// check for impact on this node
		VectorCopy (mid, lightpoint_lightspot);
		lightpoint_lightplane = node->plane;

		surf = cl.worldmodel->surfaces + node->firstsurface;
		for (i = 0 ; i < node->numsurfaces ; i++, surf++)
		{
			// no lightmaps
			if (surf->flags & SURF_DRAWTILED) continue;

			ds = (int)((float)DotProduct (mid, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3]);
			dt = (int)((float)DotProduct (mid, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3]);

			// out of range
			if (ds < surf->texturemins[0] || dt < surf->texturemins[1]) continue;

			ds -= surf->texturemins[0];
			dt -= surf->texturemins[1];

			// out of range
			if (ds > surf->extents[0] || dt > surf->extents[1]) continue;

			if (surf->samples)
			{
				// LordHavoc: enhanced to interpolate lighting
				byte	*lightmap;
				int		maps,
						dsfrac = ds & 15,
						dtfrac = dt & 15,
						r00 = 0, g00 = 0, b00 = 0,
						r01 = 0, g01 = 0, b01 = 0,
						r10 = 0, g10 = 0, b10 = 0,
						r11 = 0, g11 = 0, b11 = 0;
				float	scale;
				int		line3 = /*((surf->extents[0] >> 4) + 1)*/ surf->smax * 3;

				// LordHavoc: *3 for color
				lightmap = surf->samples + ( (dt>>4) *  surf->smax + (ds>>4) )*3;

				for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ; maps++)
				{
					scale = (float)d_lightstylevalue[surf->styles[maps]] * 1.0 / 256.0;
					r00 += (float)lightmap[0] * scale;
					g00 += (float)lightmap[1] * scale;
					b00 += (float)lightmap[2] * scale;

					r01 += (float)lightmap[3] * scale;
					g01 += (float)lightmap[4] * scale;
					b01 += (float)lightmap[5] * scale;

					r10 += (float)lightmap[line3+0] * scale;
					g10 += (float)lightmap[line3+1] * scale;
					b10 += (float)lightmap[line3+2] * scale;

					r11 += (float)lightmap[line3+3] * scale;
					g11 += (float)lightmap[line3+4] * scale;
					b11 += (float)lightmap[line3+5] * scale;

					// LordHavoc: *3 for colored lighting
//					lightmap += ((surf->extents[0] >> 4) + 1) * ((surf->extents[1] >> 4) +1 ) * 3;
					lightmap += surf->smax * surf->tmax * 3;
				}
				color[0] += (float)((int)((((((((r11 - r10) * dsfrac) >> 4) + r10) - ((((r01 - r00) * dsfrac) >> 4) + r00)) * dtfrac) >> 4) + ((((r01 - r00) * dsfrac) >> 4) + r00)));
				color[1] += (float)((int)((((((((g11 - g10) * dsfrac) >> 4) + g10) - ((((g01 - g00) * dsfrac) >> 4) + g00)) * dtfrac) >> 4) + ((((g01 - g00) * dsfrac) >> 4) + g00)));
				color[2] += (float)((int)((((((((b11 - b10) * dsfrac) >> 4) + b10) - ((((b01 - b00) * dsfrac) >> 4) + b00)) * dtfrac) >> 4) + ((((b01 - b00) * dsfrac) >> 4) + b00)));
			}

			// success
			return true;
		}

	// go down back side
		return sLight_LightPoint_Recursive (color, node->children[front >= 0], mid, end);
	}
}
int Light_LightPoint (const vec3_t startpoint)
{
	// Reset stuff
	lightpoint_lightplane   = NULL;
	lightpoint_distance		= 0;
	lightpoint_lightcolor[0] = lightpoint_lightcolor[1] = lightpoint_lightcolor[2] = 255;
	lightpoint_lightspot[0]  = lightpoint_lightspot[1]  = lightpoint_lightspot[2]  = 0;  // This is kind of a silly "erased" lightspot; might be center of the map

	if (IS_FULL_LIGHT)
		return 255;

	lightpoint_lightcolor[0] = lightpoint_lightcolor[1] = lightpoint_lightcolor[2] = 0;

	do	// Search function
	{
		// These variables store what we have for "best results" so far.
		mplane_t	*best_lightplane;
		vec3_t		 best_lightspot;
		vec3_t		 best_lightcolor;
		int			 best_distance_ent=-1;
		float		 best_distance;
		qbool		 best_found = false;

		vec3_t		endpoint;
		int			i;

		// Baker: This is the maximum depth below the player for the shadow
		// MH: set end point (back to 2048 for less BSP tree tracing)
		endpoint[0] = startpoint[0];
		endpoint[1] = startpoint[1];
		endpoint[2] = startpoint[2] - 2048;	//johnfitz increased to 8192, MH reversed due to speed.

		if (sLight_LightPoint_Recursive (lightpoint_lightcolor, cl.worldmodel->nodes, startpoint, endpoint))
		{
			best_found = true;
			// Update  bests   500               300      = 200
			best_distance   = lightpoint_distance = startpoint[2] - lightpoint_lightspot[2];
			best_lightplane	= lightpoint_lightplane;
			VectorCopy (lightpoint_lightspot,  best_lightspot);
			VectorCopy (lightpoint_lightcolor, best_lightcolor);
			best_distance_ent = 0;

		}
		else
			best_found = best_found;

		// Now check for hit with world submodels
//		Con_Printf ("Startpoint Lightspot is %4.2f vs, %4.2f\n", startpoint[2], lightpoint_lightspot[2]);
//		break;
		for (i=0 ; i< cl_numvisedicts ; i++)	//0 is world...
		{	// Baker: no point of shadows on non-visible edicts?
			entity_t	*pe = cl_visedicts[i]; //&cl_entities[i];
			vec3_t		adjusted_startpoint;
			vec3_t		adjusted_endpoint;
			vec3_t		adjusted_net;

			VectorClear (adjusted_startpoint);
			VectorClear (adjusted_endpoint);
			VectorClear (adjusted_net);

			if (!pe->model)
				continue;   // no model for ent

			if (!(pe->model->surfaces == cl.worldmodel->surfaces))
				continue;	// model isnt part of world

//			if (!(pe->model->name[0] == '*'))	// isn't a submodel ... what?
//				continue;

//			if (pe->model->type != mod_brush)
//				continue;

			// Baker: We need to adjust the point locations for entity position

			VectorSubtract (startpoint, pe->origin, adjusted_startpoint);
			VectorSubtract (endpoint,   pe->origin, adjusted_endpoint);
			VectorSubtract (startpoint, adjusted_startpoint, adjusted_net);	// How much did we change?  Well it will be pe->origin


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

			// Reset stuff
			lightpoint_lightplane   = NULL;
			lightpoint_distance		= 0;
			lightpoint_lightcolor[0] = lightpoint_lightcolor[1] = lightpoint_lightcolor[2] = 0;
			lightpoint_lightspot[0]  = lightpoint_lightspot[1]  = lightpoint_lightspot[2]  = 0;  // This is kind of a silly "erased" lightspot; might be center of the map


//			cl.worldmodel->nodes+cl.worldmodel->hulls[0].firstclipnode
//			pe->model->nodes+pe->model->hulls[0].firstclipnode
//			//
//			break;
			if (sLight_LightPoint_Recursive (lightpoint_lightcolor, pe->model->nodes+pe->model->hulls[0].firstclipnode /*pe->model->nodes*/, adjusted_startpoint, adjusted_endpoint))
			{
				// Baker: We have to add the pe->origin back into the results here!


//				VectorSubtract (adjusted_endpoint, endpoint, net_endpoint);
				VectorAdd (lightpoint_lightspot, adjusted_net, lightpoint_lightspot);


				// Calc Z distance
				lightpoint_distance   = startpoint[2] - lightpoint_lightspot[2];

				// Distance should be the dotproduct of the start - lightspot (we could sqrt it, but no reason to do so ..
				// unnecessary calculation

				if (!best_found || lightpoint_distance < best_distance)
				{
					// New best
					best_found = true;  //(Need to set an ent flag here?)
					best_distance   = lightpoint_distance;
					best_lightplane	= lightpoint_lightplane;
					best_distance_ent = i;
//					pe->alpha = 128;
					VectorCopy (lightpoint_lightspot,  best_lightspot);
					VectorCopy (lightpoint_lightcolor, best_lightcolor);
			//		Con_Printf ("Shadow surface is %s visent num is %i\n", cl_visedicts[i]->model->name, i);
				}
			}

			// On to next entity
		}

		if (!best_found)	// Lightpoint results are 0 or null
			return 255;

		// Copy over the best results as the lightpoint results

		lightpoint_distance = best_distance;
		lightpoint_lightplane = best_lightplane;
		VectorCopy (best_lightspot,  lightpoint_lightspot);
//		lightpoint_lightspot[2] += 24;
//		lightpoint_lightspot[2] += 1 ;
		VectorCopy (best_lightcolor, lightpoint_lightcolor);


	} while (0);


	return (lightpoint_lightcolor[0] + lightpoint_lightcolor[1] + lightpoint_lightcolor[2]) / 3.0;

}




#define MAX_LIGHTMAP_SIZE	4096
static unsigned	blocklights[MAX_LIGHTMAP_SIZE*3];
lightmapinfo_t lightmap[MAX_LIGHTMAPS];    //lightmapinfo_t **lightmap = &lightmapdata[0];



// the lightmap texture data needs to be kept in
// main memory so texsubimage can update properly


//=============================================================
// Dynamic lights

typedef struct dlightinfo_s
{
	int	local[2];
	int	rad;
	int	minlight;	// rad - minlight
	int	color_type;
} dlightinfo_t;

static dlightinfo_t dlightlist[MAX_DLIGHTS];
static int	numdlights;

/*
===============
R_BuildDlightList
===============
*/
static void sLight_GenerateDynamicLightmaps_For_Surface_BuildDlightList (msurface_t *surf)
{
	int			lnum, i;
	int			irad, iminlight, local[2], tdmin, sdmin, distmin;
	float		dist;
	vec3_t		impact;
	mtexinfo_t	*tex;
	dlightinfo_t	*light;

	numdlights = 0;	// For surface

//	smax = (surf->extents[0] >> 4) + 1;
//	tmax = (surf->extents[1] >> 4) + 1;
	tex = surf->texinfo;

	for (lnum = 0 ; lnum < MAX_DLIGHTS ; lnum++)
	{
//		if (cl_dlights[lnum].die < cl.time)
//			continue;		// dead light

		if (!(surf->dlightbits & (1 << lnum)))
			continue;		// not lit by this light

		dist = PlaneDiff(cl_dlights[lnum].origin, surf->plane);
		irad = (cl_dlights[lnum].radius - fabsf(dist)) * 256;
		iminlight = cl_dlights[lnum].minlight * 256;
		if (irad < iminlight)
			continue;

		iminlight = irad - iminlight;

		for (i=0 ; i<3 ; i++)
			impact[i] = cl_dlights[lnum].origin[i] - surf->plane->normal[i]*dist;

		local[0] = DotProduct(impact, tex->vecs[0]) + tex->vecs[0][3] - surf->texturemins[0];
		local[1] = DotProduct(impact, tex->vecs[1]) + tex->vecs[1][3] - surf->texturemins[1];

		// check if this dlight will touch the surface
		if (local[1] > 0)
		{
			tdmin = local[1] - (surf->tmax << 4);
			if (tdmin < 0)
				tdmin = 0;
		}
		else
		{
			tdmin = -local[1];
		}

		if (local[0] > 0)
		{
			sdmin = local[0] - (surf->smax<<4);
			if (sdmin < 0)
				sdmin = 0;
		}
		else
		{
			sdmin = -local[0];
		}

		distmin = (sdmin > tdmin) ? (sdmin << 8) + (tdmin << 7) : (tdmin << 8) + (sdmin << 7);

		if (distmin < iminlight)
		{
			// save dlight info
			light = &dlightlist[numdlights];
			light->minlight = iminlight;
			light->rad = irad;
			light->local[0] = local[0];
			light->local[1] = local[1];
			light->color_type = cl_dlights[lnum].color_type;
			numdlights++;
		}
	}
}

int dlightcolor[NUM_DLIGHTTYPES][3] = {
	{100, 90, 80},		// dimlight or brightlight
	{100, 50, 10},		// muzzleflash
	{100, 50, 10},		// explosion
	{90, 60, 7},		// rocket
	{128, 0, 0},		// red
	{0, 0, 128},		// blue
	{128, 0, 128},		// red + blue
//	{0,	128, 0}			// green
};

/*
=============================================================

	LIGHTMAPS

=============================================================
*/

static void sLight_BuildLightMap (msurface_t *surf /*, byte *dest */ /*, int stride*/);

/*
================
R_GenerateDynamicLightmaps
called during rendering
================
*/
const int lightmap_bytes = 3;
void Light_GenerateDynamicLightmaps_For_Surface (msurface_t *fa)
{

	int			maps;
	qbool		lightstyle_modified = false;

	c_brush_polys++;

	if (fa->flags & SURF_DRAWTILED) //johnfitz -- not a lightmapped surface
		return;

	// Baker: We don't generate a lightmap chain here
	// We don't always need such a chain, like if mtex

#ifdef SUPPORTS_OVERBRIGHT_SWITCH // MH Overbrights
	// mh - overbrights - need to rebuild the lightmap if this changes
	if (fa->overbright_mode != light_overbright.integer)
	{
		fa->overbright_mode = light_overbright.integer;
		goto dynamic;	// Skip checking for modification and just rebuild them all
	}

#endif

// Baker: Moved below above code to allow lightmap generation
// if gl_overbright is changed but retain speed
// in other situations

	if (!scene_dynamiclight.integer)
		return;


	// check for lightmap modification
	for (maps = 0 ; maps < MAXLIGHTMAPS && fa->styles[maps] != 255 ; maps++)
	{
		if (d_lightstylevalue[fa->styles[maps]] != fa->cached_light[maps])
		{
			lightstyle_modified = true;
			break;
		}
	}

	if (fa->dlightframe == r_framecount)
		sLight_GenerateDynamicLightmaps_For_Surface_BuildDlightList (fa);
	else
		numdlights = 0;

	if (numdlights == 0 && !fa->cached_dlight && !lightstyle_modified)
		return;

#if	SUPPORTS_OVERBRIGHT_SWITCH
dynamic:
#endif
	lightmap[fa->lightmaptexturenum].modified = true;
	//theRect = &lightmap[fa->lightmaptexturenum].rectchange;

	if (fa->light_t < lightmap[fa->lightmaptexturenum].rectchange.t)
	{
		if (lightmap[fa->lightmaptexturenum].rectchange.h)
			lightmap[fa->lightmaptexturenum].rectchange.h += lightmap[fa->lightmaptexturenum].rectchange.t - fa->light_t;
		lightmap[fa->lightmaptexturenum].rectchange.t = fa->light_t;
	}
	if (fa->light_s < lightmap[fa->lightmaptexturenum].rectchange.l)
	{
		if (lightmap[fa->lightmaptexturenum].rectchange.w)
			lightmap[fa->lightmaptexturenum].rectchange.w += lightmap[fa->lightmaptexturenum].rectchange.l - fa->light_s;
		lightmap[fa->lightmaptexturenum].rectchange.l = fa->light_s;
	}
//	smax = (fa->extents[0] >> 4) + 1;
//	tmax = (fa->extents[1] >> 4) + 1;
	if (lightmap[fa->lightmaptexturenum].rectchange.w + lightmap[fa->lightmaptexturenum].rectchange.l < fa->light_s + fa->smax)
		lightmap[fa->lightmaptexturenum].rectchange.w = fa->light_s - lightmap[fa->lightmaptexturenum].rectchange.l + fa->smax;

	if (lightmap[fa->lightmaptexturenum].rectchange.h + lightmap[fa->lightmaptexturenum].rectchange.t < fa->light_t + fa->tmax)
		lightmap[fa->lightmaptexturenum].rectchange.h = fa->light_t - lightmap[fa->lightmaptexturenum].rectchange.t + fa->tmax;

	sLight_BuildLightMap (fa);

}

/*
========================
AllocBlock -- returns a texture number and the position inside it
========================
*/

static int sLight_NewMap_BuildAllLightmaps_CreateSurfaceLightmap_AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;
	int		texnum;

	for (texnum=0 ; texnum<MAX_LIGHTMAPS ; texnum++)
	{
		best = LIGHTMAP_BLOCK_HEIGHT;

		for (i=0 ; i<LIGHTMAP_BLOCK_WIDTH-w ; i++)
		{
			best2 = 0;

			for (j=0 ; j<w ; j++)
			{
				if (lightmap[texnum].allocated[i+j] >= best)
					break;
				if (lightmap[texnum].allocated[i+j] > best2)
					best2 = lightmap[texnum].allocated[i+j];
			}
			if (j == w)
			{	// this is a valid spot
				*x = i;
				*y = best = best2;
			}
		}

		if (best + h > LIGHTMAP_BLOCK_HEIGHT)
			continue;

		for (i=0 ; i<w ; i++)
			lightmap[texnum].allocated[*x + i] = best + h;

		return texnum;
	}

	Sys_Error ("AllocBlock: full");
	return 0;
}

static void sLight_NewMap_ResetAllocBlock (void)
{
	int i;
	for (i = 0; i < MAX_LIGHTMAPS; i++)
		memset (lightmap[i].allocated, 0, sizeof(lightmap[i].allocated));	// Wipe lightmap allocation block
}

//mvertex_t	*r_pcurrentvertbase;
//model_t		*currentmodel;
/*
========================
GL_CreateSurfaceLightmap
========================
*/
static void sLight_NewMap_BuildAllLightmaps_CreateSurfaceLightmap (msurface_t *surf)
{


	if (surf->flags & SURF_DRAWTILED)
		return;

	// store these out so that we don't have to recalculate them every time
	surf->smax = (surf->extents[0] >> 4) + 1;
	surf->tmax = (surf->extents[1] >> 4) + 1;

//	if (surf->smax > LIGHTMAP_BLOCK_WIDTH || surf->tmax > LIGHTMAP_BLOCK_HEIGHT || surf->smax < 0 || surf->tmax < 0)
//	{	//whoa, buggy.
//		// Baker: can this happen?
//		surf->lightmaptexturenum = -1;
//		return;
//	}

	if (surf->smax > LIGHTMAP_BLOCK_WIDTH)
		Host_Error ("Light_CreateSurfaceLightmap: smax = %d > LIGHTMAP_BLOCK_WIDTH", surf->smax);
	if (surf->tmax > LIGHTMAP_BLOCK_HEIGHT)
		Host_Error ("Light_CreateSurfaceLightmap: tmax = %d > LIGHTMAP_BLOCK_HEIGHT", surf->tmax);
	if (surf->smax * surf->tmax > MAX_LIGHTMAP_SIZE)
		Host_Error ("Light_CreateSurfaceLightmap: smax * tmax = %d > MAX_LIGHTMAP_SIZE", surf->smax * surf->tmax);

	surf->lightmaptexturenum = sLight_NewMap_BuildAllLightmaps_CreateSurfaceLightmap_AllocBlock (surf->smax, surf->tmax, &surf->light_s, &surf->light_t);

	numdlights = 0;
	sLight_BuildLightMap (surf);
}

/*
================
BuildSurfaceDisplayList -- called at level load time
================
*/
static void sLight_NewMap_BuildAllLightmaps_BuildSurfaceDisplayList (model_t *curmodel, msurface_t *fa)
{
	medge_t		*pedges		= curmodel->edges;

	// reconstruct the polygon
	int			lnumverts	= fa->numedges;
	int			vertpage	= 0;

	glpoly_t	*poly		= Hunk_AllocName (1, sizeof(glpoly_t) + (lnumverts - 4) * VERTEXSIZE * sizeof(float), "lm_polys");

	medge_t		*r_pedge;

	int			i, lindex;
	float		*vec, s, t;

	// draw texture
//	poly = Hunk_Alloc (sizeof(glpoly_t) + (lnumverts - 4) * VERTEXSIZE * sizeof(float), "lm_polys");
	poly->next				= fa->polys;
	fa->polys				= poly;
	poly->numverts			= lnumverts;

	for (i=0 ; i<lnumverts ; i++)
	{
		lindex = curmodel->surfedges[fa->firstedge + i];

		if (lindex > 0)
		{
			r_pedge			= &pedges[lindex];
			vec				= curmodel->vertexes[r_pedge->v[0]].position;
		}
		else
		{
			r_pedge			= &pedges[-lindex];
			vec				= curmodel->vertexes[r_pedge->v[1]].position;
		}

		s = DotProduct(vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s /= fa->texinfo->texture->width;

		t = DotProduct(vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t /= fa->texinfo->texture->height;

		VectorCopy (vec, poly->verts[i]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;

		// lightmap texture coordinates
		s = DotProduct(vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s -= fa->texturemins[0];
		s += fa->light_s * 16;
		s += 8;
		s /= LIGHTMAP_BLOCK_WIDTH * 16;	//fa->texinfo->texture->width;

		t = DotProduct(vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t -= fa->texturemins[1];
		t += fa->light_t * 16;
		t += 8;
		t /= LIGHTMAP_BLOCK_HEIGHT * 16;	//fa->texinfo->texture->height;

		poly->verts[i][5] = s;
		poly->verts[i][6] = t;

		s = DotProduct(vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s /= 128;

		t = DotProduct(vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t /= 128;

		VectorCopy (vec, poly->verts[i]);
		poly->verts[i][7] = s;
		poly->verts[i][8] = t;
	}

	poly->numverts = lnumverts;
}



/*
==================
Light_BuildAllLightmaps_NewMap

Builds the lightmap texture
with all the surfaces from all brush models

Called at level load by CL_NewMap (aka start of new map begins here ...)

Light_BuildAllLightmaps_NewMap
---> Light_CreateSurfaceLightmap
---> R_BuildSurfaceDisplayList
==================
*/

static void sLight_NewMap_BuildAllLightmaps_UploadAll (void)
{
	int i;

 	if (gl_mtexable)
 		GL_EnableMultitexture ();

	// upload all lightmaps that were filled
	for (i=0 ; i<MAX_LIGHTMAPS ; i++)
	{
		if (!lightmap[i].allocated[0])
			break;		// no more used

		lightmap[i].modified		= false;
		lightmap[i].rectchange.l	= LIGHTMAP_BLOCK_WIDTH;
		lightmap[i].rectchange.t	= LIGHTMAP_BLOCK_HEIGHT;
		lightmap[i].rectchange.w	= 0;
		lightmap[i].rectchange.h	= 0;

		GL_Bind (lightmap_textures + i);
		MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		MeglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		eglTexImage2D (GL_TEXTURE_2D, 0, lightmap_bytes, LIGHTMAP_BLOCK_WIDTH, LIGHTMAP_BLOCK_HEIGHT, 0, gl_lightmap_format, GL_UNSIGNED_BYTE, lightmap[i].lightmaps);  // UPLOAD ROGUE
	}

	if (gl_mtexable)
 		GL_DisableMultitexture ();

}

static void sLight_NewMap_BuildAllLightmaps (void)
{
	int	i, j;

	for (j=1 ; j<MAX_MODELS ; j++)
	{
		if (!cl.model_precache[j])
			break;

		if (cl.model_precache[j]->name[0] == '*')	// Built in submodel ... I guess we don't lightmap that?
			continue;

		for (i=0 ; i<cl.model_precache[j]->numsurfaces ; i++)
		{
			sLight_NewMap_BuildAllLightmaps_CreateSurfaceLightmap (cl.model_precache[j]->surfaces + i);
			if (cl.model_precache[j]->surfaces[i].flags & SURF_DRAWTILED /*(SURF_DRAWLIQUID | SURF_DRAWSKY)*/)
				continue;
			sLight_NewMap_BuildAllLightmaps_BuildSurfaceDisplayList (cl.model_precache[j], cl.model_precache[j]->surfaces + i);
		}
	}

	sLight_NewMap_BuildAllLightmaps_UploadAll ();
}




/*
===============
R_AddDynamicLights

NOTE: R_BuildDlightList must be called first!
===============
*/
static void sLight_BuildLightMap_AddDynamicLights (msurface_t *surf)
{
	int				i, j;
	int				s, t, sd, td, _sd, _td;

	int				irad;		// radius
	int				idist;		// distance
	int				iminlight;	// minimum light level

	int				color[3];
	int				tmp;
	unsigned		*dest;
	dlightinfo_t	*light;

//	smax = (surf->extents[0] >> 4) + 1;
//	tmax = (surf->extents[1] >> 4) + 1;

	for (i = 0, light = dlightlist ; i < numdlights ; i++, light++)
	{
		// Baker: This populates the color component
		//        like from explosions I guess
		for (j=0 ; j<3 ; j++)
		{
			if (light->color_type == lt_explosion2 || light->color_type == lt_explosion3)
				color[j] = (int)(ExploColor[j] * 255);  // Dynamic color
			else
				color[j] = dlightcolor[light->color_type][j];  // Dynamic color
		}

		irad = light->rad;
		iminlight = light->minlight;

		_td = light->local[1];

		// Blocklights is what is being modified
		dest = blocklights;
		for (t=0 ; t<surf->tmax ; t++)
		{
			td = _td;
			if (td < 0)
				td = -td;
			_td -= 16;
			_sd = light->local[0];

			for (s=0 ; s<surf->smax ; s++, dest += blocklight_bytes)
			{
				sd = _sd < 0 ? -_sd : _sd;
				_sd -= 16;
				idist = (sd > td) ? (sd << 8) + (td << 7) : (td << 8) + (sd << 7);
				if (idist < iminlight)
				{
					tmp = (irad - idist) >> 7;
					dest[0] += tmp * color[0];  // Dynamic color
					dest[1] += tmp * color[1];  // Dynamic color
					dest[2] += tmp * color[2];  // Dynamic color
				}
			}
		}
	}
}

/*
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the 8.8 format in blocklights

FROM blocklight to dest lightmap
Stride looks like the start of the column
===============
*/
static void sLight_BuildLightMap (msurface_t *surf) // Baker: stride ... uh this is virtually a constant ... why pass it sheesh
{
	byte		*mylightmap		=	surf->samples;
	int			size			=	surf->smax * surf->tmax;


	surf->cached_dlight = !!numdlights;

	do
	{
		unsigned int base_luminance = IS_FULL_LIGHT ? 255 /*fullbright*/ : 0;	// No light

		// clear to no light (0) or full light if no light data (255)
		memset (&blocklights[0], base_luminance, size * blocklight_bytes * sizeof (unsigned int)); //johnfitz -- lit support via lordhavoc

		if (IS_FULL_LIGHT)
			break;		// We don't have any lightmap building to do

		// add all the lightmaps for the surface
		if (mylightmap)
		{
			int			block_size = size * blocklight_bytes;
			int			maps;
			unsigned	scale;
			int			i;

			for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ; maps++)
			{
				unsigned	*blocklight_fill = blocklights;

				surf->cached_light[maps] = scale = d_lightstylevalue[surf->styles[maps]];	// 8.8 fraction

				for (i=0 ; i<block_size  ; i++)
					*blocklight_fill++ += mylightmap[i] * scale;

				mylightmap += block_size;	// skip to next lightmap
			}
		}

		// add all the dynamic lights
		if (numdlights)
			sLight_BuildLightMap_AddDynamicLights (surf);

	} while (0);


// bound, invert, and shift
//store:
	{
		int			i, j, k, t;
		unsigned	*bl=blocklights;
		qbool		isstained		=   r_stains.integer ? surf->stained : false;

		int			in_stride		=	(LIGHTMAP_BLOCK_WIDTH - surf->smax) * lightmap_bytes;
		byte		*dest			=	lightmap[surf->lightmaptexturenum].lightmaps +
										   (surf->light_t * LIGHTMAP_BLOCK_WIDTH + surf->light_s) * lightmap_bytes;

		byte		*stain			=	lightmap[surf->lightmaptexturenum].stainmaps +
										   (surf->light_t * LIGHTMAP_BLOCK_WIDTH + surf->light_s) * 3;

			for (i = 0 ; i < surf->tmax ; i++, dest += in_stride, stain +=in_stride)
				for (j = surf->smax ; j ; j--, bl +=blocklight_bytes, dest +=lightmap_bytes, stain +=lightmap_bytes)
					for (k = 0; k<lightmap_bytes; k++)
					{
						t = bl[k];

					switch (surf->overbright_mode)					// Add light shift
					{
					case -1:		t = (t >> 6);	break;				// Kurok? ... no overbrighting
					case 0:		t = (t >> 7);	break;				// GLQuake ... no overbrighting
					default:
					case 1:		t = (t >> 8) + (t >> 9); break;		// JoeQuake default
					case 2:		t = (t >> 8);	break;						// MH likes this one
					case 3:		t = (t >> 9);	break;				// Kurok? ... no overbrighting
					case 4:		t = (t >> 10);	break;				// Kurok? ... no overbrighting
					

					}

					if (isstained && stain[k])						// merge in stain; don't waste time if stain[k] is 0
						t = (t * (256-stain[k])) >> 8;				// 256 - stain[k] is effectively the inverted stain alpha (0-255)

							dest[k] = CLAMP (0, 255 - t, 255);
					}

	}
}

/*
=============================================================

	LIGHTMAP UPLOAD

=============================================================
*/


/*
===============
R_UploadLightmap -- johnfitz -- uploads the modified lightmap to opengl if necessary

assumes lightmap texture is already bound
===============
*/
static void sLight_Upload_Lightmap_ChangedRegion (const msurface_t *surf, const int in_lightmapnum)
{
	int lightmapnum = !surf ? in_lightmapnum : surf->lightmaptexturenum;

	if (!lightmap[lightmapnum].modified)	return;		// Not modified

	lightmap[lightmapnum].modified = false;

	eglTexSubImage2D (GL_TEXTURE_2D, 0, 0, lightmap[lightmapnum].rectchange.t, LIGHTMAP_BLOCK_WIDTH, lightmap[lightmapnum].rectchange.h, gl_lightmap_format, GL_UNSIGNED_BYTE, lightmap[lightmapnum].lightmaps+(lightmap[lightmapnum].rectchange.t) *LIGHTMAP_BLOCK_WIDTH*lightmap_bytes); // UPLOAD ROGUE

//	if (surf) Con_Printf ("Uploaded lightmap texture subrect for surf %i\n", surf->stained);

	// Restore update region to the whole
	lightmap[lightmapnum].rectchange.l = LIGHTMAP_BLOCK_WIDTH;
	lightmap[lightmapnum].rectchange.t = LIGHTMAP_BLOCK_HEIGHT;
	lightmap[lightmapnum].rectchange.h = 0;
	lightmap[lightmapnum].rectchange.w = 0;
}


void Light_UploadLightmaps_Modified (void)
{
	int			i;
//	glRect_t	*theRect;

	for (i = 0; i < MAX_LIGHTMAPS; i++)
	{
		if (!lightmap[i].modified)	continue;				// Not modified

		GL_Bind (lightmap_textures + i);				// Because sLight_Upload_Lightmap_ChangedRegion assumes texture already bound
		sLight_Upload_Lightmap_ChangedRegion (NULL, i);
	}
}


/*
=============================================================

	STAINMAPS - ADAPTED FROM FTEQW

=============================================================
*/

#if SUPPORTS_FTESTAINS

static void sStain_NewMap_WipeStains(void)
{
	int i;
	for (i = 0; i < MAX_LIGHTMAPS; i++)
	{
//		if (!lightmap[i])
//			break;
		memset(lightmap[i].stainmaps, 0, sizeof(lightmap[i].stainmaps));
	}
}


// Fade out the stain map data lightmap[x].stainmap until decays to 255 (no stain)
// Needs to occur before
static void sStain_FrameSetup_LessenStains(void)
{
	static		float time;

	if (!r_stains.integer)
		return;

	time += host_frametime;

	if (time < r_stains_fadetime.floater)
		return;

	// Time for stain fading.  Doesn't occur every frame
	time-=r_stains_fadetime.floater;

	{
		int			decay_factor	= r_stains_fadeamount.floater;
		msurface_t	*surf = cl.worldmodel->surfaces;
		int			i;

		for (i=0 ; i<cl.worldmodel->numsurfaces ; i++, surf++)
		{
			if (surf->stained)
			{
				int			stride			=	(LIGHTMAP_BLOCK_WIDTH-surf->smax)*3;
				byte		*stain			=	lightmap[surf->lightmaptexturenum].stainmaps +
											     (surf->light_t * LIGHTMAP_BLOCK_WIDTH + surf->light_s) * 3;
				int			s, t;

				surf->cached_dlight=-1;		// nice hack here...
				surf->stained = false;		// Assume not stained until we know otherwise

				for (t = 0 ; t<surf->tmax ; t++, stain+=stride)
				{
					int smax_times_3 = surf->smax * 3;
					for (s=0 ; s<smax_times_3 ; s++, stain++)
					{
						if (*stain < decay_factor)
							{
							*stain = 0;		//reset to 0
							continue;
						}

						//eventually decay to 0
						*stain -= decay_factor;
						surf->stained=true;
					}
				}
				// End of surface stained check
			}

			// Next surface

		}
	}
}

// Fill in the stain map data; lightmap[x].stainmap
static void sStain_AddStain_StainNodeRecursive_StainSurf (msurface_t *surf, const vec3_t origin, const float tint, const float radius /* float *parms*/)
{
	if (surf->lightmaptexturenum < 0)		// Baker: Does this happen?
		return;


	{
		float		dist;
		float		rad = radius;
		float		minlight=0;

		int			lim= 240; // (int)(r_stains.floater*255.0f); // //255 - (r_stains.floater*255);
		mtexinfo_t	*tex = surf->texinfo;
		byte		*stainbase;

		int			s,t, i, sd, td;
		float		change, amm;
		vec3_t		impact, local;

		stainbase = lightmap[surf->lightmaptexturenum].stainmaps;	// Each lightmap has own special slot with a stainmap ... pointer?
		stainbase += (surf->light_t * LIGHTMAP_BLOCK_WIDTH + surf->light_s) * 3;

		// Calculate impact
		dist = DotProduct (origin, surf->plane->normal) - surf->plane->dist;
		rad -= fabs(dist);
		minlight = 0;

		if (rad < minlight)	//not hit
			return;

		minlight = rad - minlight;

		for (i=0 ; i<3 ; i++)
		{
			impact[i] = origin[i] - surf->plane->normal[i]*dist;
		}

		local[0] = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3];
		local[1] = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3];

		local[0] -= surf->texturemins[0];
		local[1] -= surf->texturemins[1];


		for (t = 0 ; t<surf->tmax ; t++, stainbase += 3*LIGHTMAP_BLOCK_WIDTH /*stride*/)
		{
			td = local[1] - t*16;  if (td < 0) td = -td;

			for (s=0 ; s<surf->smax ; s++)
			{
				sd = local[0] - s*16;  if (sd < 0) sd = -sd;

				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);
				if (dist < minlight)
				{
					//amm = stainbase[s*3+0]*(rad - dist)*tint;
					amm = (rad - dist);
					change = stainbase[s*3+0] + amm * tint  /* parms[4+x] */;
					stainbase[s*3+0] = CLAMP (0, change, lim);
//					change = stainbase[s*3+1] + amm * tint /* parms[4+x] */;
					stainbase[s*3+1] = CLAMP (0, change, lim);
//					change = stainbase[s*3+2] + amm * tint /* parms[4+x] */;
					stainbase[s*3+2] = CLAMP (0, change, lim);

					surf->stained = true;
				}
			}

		}
	}
	// Mark it as a dynamic light so the lightmap gets updated

	if (surf->stained)
	{
		static int mystainnum;
		surf->cached_dlight=-1;
//		surf->flags |= SURF_SELECTED;	// Add flag
		if (!surf->stainnum)
			surf->stainnum = mystainnum++;
	}
}


//combination of R_AddDynamicLights and R_MarkLights
static void sStain_AddStain_StainNodeRecursive (mnode_t *node, const vec3_t origin, const float tint, const float radius/*float *parms*/)
{
	mplane_t	*splitplane;
	float		dist;
	msurface_t	*surf;
	int			i;

	if (node->contents < 0)
		return;

	splitplane = node->plane;
	dist = DotProduct (origin, splitplane->normal) - splitplane->dist;

	if (dist > radius)
	{
		sStain_AddStain_StainNodeRecursive (node->children[0], origin, tint, radius);
		return;
	}

	if (dist < -radius)
	{
		sStain_AddStain_StainNodeRecursive (node->children[1], origin, tint, radius);
		return;
	}

// mark the polygons
	surf = cl.worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		if (surf->flags&~(SURF_DRAWTILED|SURF_PLANEBACK))  // Baker: Can stain sky?  We no ... because they don't have lightmaps
			continue;

		sStain_AddStain_StainNodeRecursive_StainSurf(surf, origin, tint, radius);
	}


	sStain_AddStain_StainNodeRecursive (node->children[0], origin, tint, radius);
	sStain_AddStain_StainNodeRecursive (node->children[1], origin, tint, radius);

}




// This builds the stain map
void Stain_AddStain(const vec3_t origin, const float tint, /*float red, float green, float blue,*/ const float in_radius)
{
	entity_t	*pe;
	int i;
	vec3_t		adjorigin;

	// Slight randomization.  Use 80 of the radius plus or minus 20%
	float		radius = in_radius * .90 + (sinf(cl.ctime*3)*.20);

	if (!cl.worldmodel || !r_stains.integer)
		return;

	// Stain the world surfaces
	sStain_AddStain_StainNodeRecursive(cl.worldmodel->nodes+cl.worldmodel->hulls[0].firstclipnode, origin, tint, radius);
//		if above then Con_Printf ("Hit world surface\n");

	//now stain bsp models other than world.

	for (i=1 ; i< cl.num_entities ; i++)	//0 is world...
	{
		pe = &cl_entities[i];
		if (pe->model && pe->model->surfaces == cl.worldmodel->surfaces)
		{
			VectorSubtract (origin, pe->origin, adjorigin);
			if (pe->angles[0] || pe->angles[1] || pe->angles[2])
			{
				vec3_t f, r, u, temp;
				AngleVectors(pe->angles, f, r, u);
				VectorCopy(adjorigin, temp);
				adjorigin[0] = DotProduct(temp, f);
				adjorigin[1] = -DotProduct(temp, r);
				adjorigin[2] = DotProduct(temp, u);
			}

			sStain_AddStain_StainNodeRecursive (pe->model->nodes+pe->model->hulls[0].firstclipnode, adjorigin, tint, radius);
//				if above Con_Printf ("Hit sub-model surface\n");
		}
	}
}



#endif

// Once per map after world model and other model precaches are loaded
void Light_NewMap (void)
{
#if SUPPORTS_FTESTAINS
	sStain_NewMap_WipeStains ();
#endif

	sLight_NewMap_ResetAllocBlock ();
	sLight_NewMap_ResetLightStyles ();
	sLight_NewMap_BuildAllLightmaps ();

// Clears lightmap stuff
}

// Once per frame setup before rendering
void Light_FrameSetup (void)
{

#if SUPPORTS_FTESTAINS
	sStain_FrameSetup_LessenStains();  //qbism ftestain
#endif

	sLight_FrameSetup_PushDlights ();			// Fitz Moved
	sLight_FrameSetup_AnimateLight_Styles ();	// R_AnimateLight ();
}

void Light_Init (void)
{
	sLight_Init_FlashBlend_Bubble ();


}


// File these


#ifdef SUPPORTS_COLORED_LIGHTS
dlighttype_t Light_SetDlightColor (const float f, const dlighttype_t def, const qbool random)
{
	dlighttype_t	colors[NUM_DLIGHTTYPES-4] = {lt_red, lt_blue, lt_redblue/*, lt_green*/};

	if ((int)f == 1)
		return lt_red;
	else if ((int)f == 2)
		return lt_blue;
	else if ((int)f == 3)
		return lt_redblue;
/*	else if ((int)f == 4)
		return lt_green;*/
	else if (((int)f == NUM_DLIGHTTYPES - 3) && random)
		return colors[rand()%(NUM_DLIGHTTYPES-4)];
	else
		return def;
}
#endif

/*
===============
CL_AllocDlight
===============
*/
dlight_t *Light_AllocDlight (const int key)
{
	int		i;
	dlight_t	*dl;

	// first look for an exact key match
	if (key)
	{
		dl = cl_dlights;
		for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
		{
			if (dl->key == key)
			{
				memset (dl, 0, sizeof(*dl));
				dl->key = key;
				return dl;
			}
		}
	}

	// then look for anything else
	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time)
		{
			memset (dl, 0, sizeof(*dl));
			dl->key = key;
			return dl;
		}
	}

	dl = &cl_dlights[0];
	memset (dl, 0, sizeof(*dl));
	dl->key = key;
	return dl;
}

void Light_NewDlight (const int key, const vec3_t origin, const float radius, const float time, const int type)
{
	dlight_t	*dl;

	dl = Light_AllocDlight (key);
	VectorCopy (origin, dl->origin);
	dl->radius = radius;
	dl->die = cl.time + time;
#ifdef SUPPORTS_COLORED_LIGHTS
	dl->color_type = type;
#endif
}


/*
===============
CL_DecayLights
===============
*/
void Light_DecayLights (void)
{
	int		i;
	dlight_t	*dl;
	float		time;

	time = cl.time - cl.oldtime;

	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time || !dl->radius)
			continue;

//		dl->radius -= host_frametime * dl->decay;
//		Baker: should fix dlight decay when paused
		dl->radius -= time*dl->decay;
		if (dl->radius < 0)
			dl->radius = 0;
	}
}