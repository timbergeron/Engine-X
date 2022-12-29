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

//johnfitz -- struct for passing lerp information to drawing functions
typedef struct {
	short	pose1;
	short	pose2;

	vec3_t	origin;
	vec3_t	angles;

	qbool	full_light;
	float	shade_light;
	float	ambient_light;

	qbool	isLerpingAnimation;
	float	blend;
	float   iblend;

} lerpdata_t;
//johnfitz

// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
static float	r_avertexnormal_dots[SHADEDOT_QUANT][256] =
#include "model_alias_anorm_dots.h"
;

static float	*shadedots = r_avertexnormal_dots[0];

/*
=============
GL_DrawAliasFrame
=============
*/

void GL_DrawAliasFrameVerts (/*const entity_t *my_glent, */ const aliashdr_t *paliashdr, const lerpdata_t lerpdata, const qbool mtexable, const qbool isViewWeapon, const float entalpha, const int maxDistance, const qbool shadowpass, const vec3_t myShadeVector, const float shadow_height, const float shadow_lheight)
{
	int			*commands;
	int			count;

	// Non-Interpolated
//	trivertx_t	*verts;

	// Fitz
	float	u,v;
	trivertx_t	*verts_current = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
	trivertx_t	*verts_previous;

	// poses the same means either 1. the entity has paused its animation, or 2. r_lerpmodels is disabled
//B	if (lerpdata.pose1 != lerpdata.pose2)	// <---- Baker: I'd like to get this out of here
//B	{
//B		lerping = true;				// Baker: I'd like to get this out of here
//B		blend = lerpdata.blend;		// Baker: I'd like to get this out of here
//B		iblend = 1.0f - blend;		// Baker: I'd like to get this out of here

	if (lerpdata.isLerpingAnimation)
	{
		// Interpolation uses combo of 2 frames
		verts_previous   = verts_current;
		verts_previous += lerpdata.pose1 * paliashdr->poseverts;
	}

	verts_current  += lerpdata.pose2 * paliashdr->poseverts;

	commands = (int *)((byte *)paliashdr + paliashdr->commands);

	while (1)
	{
		// get the vertex count and primitive type
		count = *commands++;
		if (!count)
			break;		// done

		if (count < 0)
		{
			count = -count;
			eglBegin (GL_TRIANGLE_FAN);
		}
		else
			eglBegin (GL_TRIANGLE_STRIP);

		do
		{
			// texture coordinates come from the draw list
			u = ((float *)commands)[0];
			v = ((float *)commands)[1];
			commands += 2;

			if (shadowpass)
			{
				vec3_t point_current;
				// normals and vertexes come from the frame list
//				point[0] = verts->v[0] * paliashdr->scale[0] + paliashdr->scale_origin[0];
//				point[1] = verts->v[1] * paliashdr->scale[1] + paliashdr->scale_origin[1];
//				point[2] = verts->v[2] * paliashdr->scale[2] + paliashdr->scale_origin[2];

				VectorMultiplyAddV (paliashdr->scale_origin, paliashdr->scale, verts_current->v, point_current);

				point_current[0] -= myShadeVector[0] * (point_current[2] + shadow_lheight);
				point_current[1] -= myShadeVector[1] * (point_current[2] + shadow_lheight);
				point_current[2] =  shadow_height; // height;

				if (lerpdata.isLerpingAnimation)
				{
					vec3_t	point_previous;
					vec3_t	point_delta;
					vec3_t	point_final;

					VectorMultiplyAddV (paliashdr->scale_origin, paliashdr->scale, verts_previous->v, point_previous);
					point_previous[0] -= myShadeVector[0] * (point_previous[2]+shadow_lheight);
					point_previous[1] -= myShadeVector[1] * (point_previous[2]+shadow_lheight);
					point_previous[2] =  shadow_height; // height;

					VectorSubtract (point_current, point_previous, point_delta); // delta

					VectorMultiplyAdd (point_previous, lerpdata.blend, point_delta, point_final);
//					point_final[2] = shadow_height;

					eglVertex3fv (point_final);

				}
				else
					eglVertex3fv (point_current);

				goto shadowskips;
			}

			if (mtexable)
			{
				qglMultiTexCoord2f (GL_TEXTURE0_ARB, u, v);
				qglMultiTexCoord2f (GL_TEXTURE1_ARB, u, v);
			}
			else
				eglTexCoord2f (u, v);

			// normals and vertexes come from the frame list
			if (!lerpdata.full_light)
			{
				float		luminance;
				float 		vertcolor[4];
				int			i;

				if (lerpdata.isLerpingAnimation)
				{
					// blend the light intensity from the two frames together
					if (r_vertex_lights.integer)
						luminance = VertexLights_LerpVertexLight (verts_previous->lightnormalindex, verts_current->lightnormalindex, lerpdata.blend, lerpdata.angles[0] /*my_glent->angles[0]*/, lerpdata.angles[1] /*my_glent->angles[1]*/);
					else
					{
						// interpolates the shadedots from 2 frames
						float luminance_current = (shadedots[verts_current->lightnormalindex]  * lerpdata.shade_light + lerpdata.ambient_light) 
												- (shadedots[verts_previous->lightnormalindex] * lerpdata.shade_light + lerpdata.ambient_light);
						luminance = ((shadedots[verts_previous->lightnormalindex] * lerpdata.shade_light + lerpdata.ambient_light) + (lerpdata.blend * luminance_current)) / 256.0;
					}
				}
				else	// Not lerping
				{

					if (r_vertex_lights.integer)
						luminance = VertexLights_GetVertexLightValue (verts_current->lightnormalindex, lerpdata.angles[0], lerpdata.angles[1]);
					else
						luminance = (shadedots[verts_current->lightnormalindex] * lerpdata.shade_light + lerpdata.ambient_light) / 256.0;
				}

				luminance = min(luminance, 1);

				for (i=0 ; i<3 ; i++)
					vertcolor[i] = lightpoint_lightcolor[i] / 256 + luminance;

				vertcolor[3] = entalpha;
				MeglColor4fv (vertcolor);

			}

			{	// Pathway for non-interpolated verts within interpolated frames
				qbool 	interpolate_vert=true;
				vec3_t	vert_delta;

				// Jozsef's favored method to disqualify the lerping of certain verts
				if (lerpdata.isLerpingAnimation)
				{

					float	myDistance;

					VectorSubtract (verts_current->v, verts_previous->v, vert_delta);

					if (isViewWeapon && (myDistance = DotProduct (vert_delta, vert_delta))>maxDistance)
						interpolate_vert = false;
				}

				if (lerpdata.isLerpingAnimation && interpolate_vert)
				{
	//				// Baker: Note!  Fitz blends it more precisely
	//				vec3_t finalvert;
	//				VectorMultiplyAdd (verts_previous->v, blend, vert_delta, finalvert);
	//				eglVertex3fv (finalvert);
					eglVertex3f (verts_previous->v[0] + (lerpdata.blend * vert_delta[0]), verts_previous->v[1] + (lerpdata.blend * vert_delta[1]), verts_previous->v[2] + (lerpdata.blend * vert_delta[2]));
				}
				else
				{
					eglVertex3f (verts_current->v[0], verts_current->v[1], verts_current->v[2]);
//					eglVertex3fv (verts_current->v);
				}
			}

shadowskips:

			if (lerpdata.isLerpingAnimation) verts_previous++;
			verts_current++;
		} while (--count); // Do loop

		eglEnd ();
	} // End main while loop

}