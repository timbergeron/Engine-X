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
// model_brush_render_liquids.c -- warping textures ... water, lava, slime
// Baker: Validated 6-27-2011.  Just r_waterripple


#include "quakedef.h"



msurface_t	*warpface;



void BoundPoly (const int numverts, const float *verts, vec3_t mins, vec3_t maxs)
{
	int	i, j;
	const float	*v;

	mins[0] = mins[1] = mins[2] = 9999;
	maxs[0] = maxs[1] = maxs[2] = -9999;
	v = verts;
	for (i=0 ; i<numverts ; i++)
	{
		for (j=0 ; j<3 ; j++, v++)
		{
			if (*v < mins[j])
				mins[j] = *v;
			if (*v > maxs[j])
				maxs[j] = *v;
		}
	}
}


void SubdividePolygon (const int numverts, float *verts)
{
	int			i, j, k;
	vec3_t		mins, maxs;
	float		m;
	float		*v;
	vec3_t		front[64], back[64];
	int			f, b;
	float		dist[64];
	float		frac;
	glpoly_t	*poly;
	float		s, t;
	float		subdivide_size;

	if (numverts > 60)
		Sys_Error ("numverts = %i", numverts);

	subdivide_size = max(1, r_subdivide_size.floater);
	BoundPoly (numverts, verts, mins, maxs);

	for (i=0 ; i<3 ; i++)
	{
		m = (mins[i] + maxs[i]) * 0.5;
		m = subdivide_size * floorf (m/subdivide_size + 0.5);
		if (maxs[i] - m < 8)
			continue;
		if (m - mins[i] < 8)
			continue;

		// cut it
		v = verts + i;
		for (j=0 ; j<numverts ; j++, v+=3)
			dist[j] = *v - m;

		// wrap cases
		dist[j] = dist[0];
		v-=i;
		VectorCopy (verts, v);

		f = b = 0;
		v = verts;
		for (j=0 ; j<numverts ; j++, v+=3)
		{
			if (dist[j] >= 0)
			{
				VectorCopy (v, front[f]);
				f++;
			}
			if (dist[j] <= 0)
			{
				VectorCopy (v, back[b]);
				b++;
			}
			if (dist[j] == 0 || dist[j+1] == 0)
				continue;
			if ((dist[j] > 0) != (dist[j+1] > 0))
			{
				// clip point
				frac = dist[j] / (dist[j] - dist[j+1]);
				for (k=0 ; k<3 ; k++)
					front[f][k] = back[b][k] = v[k] + frac*(v[3+k] - v[k]);
				f++;
				b++;
			}
		}

		SubdividePolygon (f, front[0]);
		SubdividePolygon (b, back[0]);
		return;
	}

	poly = Hunk_AllocName (1, sizeof(glpoly_t) + (numverts - 4) * VERTEXSIZE * sizeof(float), "subdiv_p");
	poly->next = warpface->polys;
	warpface->polys = poly;
	poly->numverts = numverts;
	for (i = 0 ; i < numverts ; i++, verts += 3)
	{
		VectorCopy (verts, poly->verts[i]);
		s = DotProduct (verts, warpface->texinfo->vecs[0]);
		t = DotProduct (verts, warpface->texinfo->vecs[1]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;
	}
}

//=========================================================

#define	TURBSINSIZE	128
#define	TURBSCALE	((float)TURBSINSIZE / (2 * M_PI))

static byte turbsin[TURBSINSIZE] =
{
	127, 133, 139, 146, 152, 158, 164, 170, 176, 182, 187, 193, 198, 203, 208, 213,
	217, 221, 226, 229, 233, 236, 239, 242, 245, 247, 249, 251, 252, 253, 254, 254,
	255, 254, 254, 253, 252, 251, 249, 247, 245, 242, 239, 236, 233, 229, 226, 221,
	217, 213, 208, 203, 198, 193, 187, 182, 176, 170, 164, 158, 152, 146, 139, 133,
	127, 121, 115, 108, 102, 96, 90, 84, 78, 72, 67, 61, 56, 51, 46, 41,
	37, 33, 28, 25, 21, 18, 15, 12, 9, 7, 5, 3, 2, 1, 0, 0,
	0, 0, 0, 1, 2, 3, 5, 7, 9, 12, 15, 18, 21, 25, 28, 33,
	37, 41, 46, 51, 56, 61, 67, 72, 78, 84, 90, 96, 102, 108, 115, 121,
};

__inline static float SINTABLE_APPROX (float time)
{
	float	sinlerpf, lerptime, lerp;
	int	sinlerp1, sinlerp2;

	sinlerpf = time * TURBSCALE;
	sinlerp1 = floorf(sinlerpf);
	sinlerp2 = sinlerp1 + 1;
	lerptime = sinlerpf - sinlerp1;

	lerp =	turbsin[sinlerp1 & (TURBSINSIZE - 1)] * (1 - lerptime) + turbsin[sinlerp2 & (TURBSINSIZE - 1)] * lerptime;
	return -8 + 16 * lerp / 255.0;
}


void CalcCausticTexCoords (const float *verts, float *s, float *t)
{
	float	os, ot;

	os = verts[3];
	ot = verts[4];

	*s = os + SINTABLE_APPROX(0.465 * (cl.ctime + ot));
	*s *= -3 * (0.5 / 64);

	*t = ot + SINTABLE_APPROX(0.465 * (cl.ctime + os));
	*t *= -3 * (0.5 / 64);
}




/*
=============
EmitWaterPolys

Does a water warp on the pre-fragmented glpoly_t chain
=============
*/

void EmitWaterPolys (const msurface_t *surf)
{
	glpoly_t	*polys;
	float		*verts;
	int			i;
	float		s, t, os, ot;
	vec3_t		newverts;
#ifdef _DEBUGG
	if (in_scene_setup == false)
		Con_Printf ("Scene violation: EmitWaterPolys -> Drawing submodel surfaces prescene");
#endif

	mglPushStates ();
	GL_DisableMultitexture ();

	GL_Bind (surf->texinfo->texture->gl_texturenum);
//	GL_Bind (glasstexture);
//	glColor3f (1,1,1);
//	MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	mglFinishedStates ();

	for (polys = surf->polys ; polys ; polys = polys->next)
	{
		eglBegin (GL_POLYGON);
		for (i = 0, verts = polys->verts[0] ; i < polys->numverts ; i++, verts += VERTEXSIZE)
		{
			os = verts[3];
			ot = verts[4];

			s = os + SINTABLE_APPROX(ot * 0.125 + cl.ctime);
			s *= (1.0 / 64);

			t = ot + SINTABLE_APPROX(os * 0.125 + cl.ctime);
			t *= (1.0 / 64);

			eglTexCoord2f (s, t);
// Q2K4-0027 start
			if(r_water_ripple.floater && !(ISTELETEX(surf->texinfo->texture->name)) && !(ISTELETEX2(surf->texinfo->texture->name)))
			{
				newverts[0] = verts[0];
				newverts[1] = verts[1];
				//qbism//jf 2000-03-01 adjusted ripple speed/amplitude

				newverts[2] = verts[2] + r_water_ripple.floater *sinf(verts[0]*0.05+cl.ctime*3)*sinf(verts[2]*0.05+cl.ctime*3);
				eglVertex3fv (newverts);
			}
			else
			{
				eglVertex3fv (verts);
			}
// Q2K4-0027 end
		}
		eglEnd ();
	}

	mglPopStates ();
}


/*
=============
EmitAlphaPolys
=============
*/

// Baker: WE are doing this instead of the normal
// texture coords

void EmitFencePolys (const msurface_t *surf)
{
	glpoly_t	*polys;
	float		*verts;
//	float		s, t;
	int			i;
#ifdef _DEBUGG
	if (in_scene_setup == false)
		Con_Printf ("Scene violation: EmitFencePolys -> Drawing submodel surfaces prescene");
#endif

	GL_DisableMultitexture ();
	GL_Bind (surf->texinfo->texture->gl_texturenum);

	MeglEnable (GL_BLEND);
	MeglEnable (GL_ALPHA_TEST);

	for (polys = surf->polys ; polys ; polys=polys->next)
	{
		eglBegin (GL_POLYGON);
		for (i=0, verts = polys->verts[0] ; i< polys->numverts ; i++, verts += VERTEXSIZE)
		{
			eglTexCoord2f (verts[3], verts[4]);
			eglVertex3fv (verts);
		}
		eglEnd ();
	}


	MeglDisable (GL_ALPHA_TEST);
	MeglDisable (GL_BLEND);
}




/*
=============
EmitReflectivePolys
=============
*/

// Baker: WE are doing this instead of the normal
// texture coords

void EmitGlassyPolys (const msurface_t *surf)
{
	glpoly_t	*polys;
	float		*verts;
	float		s, t;
	int			i;
#ifdef _DEBUGG
	if (in_scene_setup == false)
		Con_Printf ("Scene violation: EmitGlassyPolys -> Drawing submodel surfaces prescene");
#endif

	GL_DisableMultitexture ();
//	GL_Bind (glasstexture);
	GL_Bind (surf->texinfo->texture->gl_texturenum);

	MeglEnable (GL_BLEND);
	MeglBlendFunc(GL_DST_COLOR, GL_SRC_COLOR); //2x modulate

	for (polys = surf->polys ; polys ; polys=polys->next)
	{
		eglBegin (GL_POLYGON);
		for (i=0, verts = polys->verts[0] ; i< polys->numverts ; i++, verts += VERTEXSIZE)
		{

			CalcReflectiveTexCoords (verts, &s, &t);

			eglTexCoord2f (s, t);
			eglVertex3fv (verts);
		}
		eglEnd ();
	}


	MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	MeglDisable (GL_BLEND);
}

