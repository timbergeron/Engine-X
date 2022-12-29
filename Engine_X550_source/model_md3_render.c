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

// model_md3_render.c -- brush model loading

#include "quakedef.h"


/*
===============================================================================

								Q3 MODELS

===============================================================================
*/

void R_RotateForTagEntity (tagentity_t *tagent, md3tag_t *tag, float *m)
{
	int		i;
	float	lerpfrac, timepassed;

	// positional interpolation
	timepassed = cl.time - tagent->tag_translate_start_time;

	if (tagent->tag_translate_start_time == 0 || timepassed > 1)
	{
		tagent->tag_translate_start_time = cl.time;
		VectorCopy (tag->pos, tagent->tag_pos1);
		VectorCopy (tag->pos, tagent->tag_pos2);
	}

	if (!VectorCompare(tag->pos, tagent->tag_pos2))
	{
		tagent->tag_translate_start_time = cl.time;
		VectorCopy (tagent->tag_pos2, tagent->tag_pos1);
		VectorCopy (tag->pos, tagent->tag_pos2);
		lerpfrac = 0;
	}
	else
	{
		lerpfrac = timepassed / 0.1;
		if (cl.paused || lerpfrac > 1)
			lerpfrac = 1;
	}

//	VectorInterpolate (tagent->tag_pos1, lerpfrac, tagent->tag_pos2, m + 12);
	LerpVector (tagent->tag_pos1, tagent->tag_pos2, lerpfrac, m + 12);

	m[15] = 1;

	for (i = 0 ; i < 3 ; i++)
	{
		// orientation interpolation (Euler angles, yuck!)
		timepassed = cl.time - tagent->tag_rotate_start_time[i];

		if (tagent->tag_rotate_start_time[i] == 0 || timepassed > 1)
		{
			tagent->tag_rotate_start_time[i] = cl.time;
			VectorCopy (tag->rot[i], tagent->tag_rot1[i]);
			VectorCopy (tag->rot[i], tagent->tag_rot2[i]);
		}

		if (!VectorCompare(tag->rot[i], tagent->tag_rot2[i]))
		{
			tagent->tag_rotate_start_time[i] = cl.time;
			VectorCopy (tagent->tag_rot2[i], tagent->tag_rot1[i]);
			VectorCopy (tag->rot[i], tagent->tag_rot2[i]);
			lerpfrac = 0;
		}
		else
		{
			lerpfrac = timepassed / 0.1;
			if (cl.paused || lerpfrac > 1)
				lerpfrac = 1;
		}

//		VectorInterpolate (tagent->tag_rot1[i], lerpfrac, tagent->tag_rot2[i], m + i*4);
		LerpVector (tagent->tag_rot1[i], tagent->tag_rot2[i], lerpfrac, m + i*4);
		m[i*4+3] = 0;
	}
}

int			bodyframe = 0, legsframe = 0;
animtype_t	bodyanim, legsanim;

void R_ReplaceQ3Frame (int frame)
{
	animdata_t		*currbodyanim, *currlegsanim;
	static animtype_t oldbodyanim, oldlegsanim;
	static float	bodyanimtime, legsanimtime;
	static qbool	deathanim = false;

	if (deathanim)
	{
		bodyanim = oldbodyanim;
		legsanim = oldlegsanim;
	}

	if (frame < 41 || frame > 102)
		deathanim = false;

	if (frame >= 0 && frame <= 5)		// axrun
	{
		bodyanim = torso_stand2;
		legsanim = legs_run;
	}
	else if (frame >= 6 && frame <= 11)	// rockrun
	{
		bodyanim = torso_stand;
		legsanim = legs_run;
	}
	else if ((frame >= 12 && frame <= 16) || (frame >= 35 && frame <= 40))	// stand, pain
	{
		bodyanim = torso_stand;
		legsanim = legs_idle;
	}
	else if ((frame >= 17 && frame <= 28) || (frame >= 29 && frame <= 34))	// axstand, axpain
	{
		bodyanim = torso_stand2;
		legsanim = legs_idle;
	}
	else if (frame >= 41 && frame <= 102 && !deathanim)	// axdeath, deatha, b, c, d, e
	{
		bodyanim = legsanim = rand() % 3;
		deathanim = true;
	}
	else if (frame >= 103 && frame <= 118)	// gun attacks
	{
		bodyanim = torso_attack;
	}
	else if (frame >= 119)			// axe attacks
	{
		bodyanim = torso_attack2;
	}

	currbodyanim = &anims[bodyanim];
	currlegsanim = &anims[legsanim];

	if (bodyanim == oldbodyanim)
	{
		if (cl.time >= bodyanimtime + currbodyanim->interval)
		{
			if (currbodyanim->loop_frames && bodyframe + 1 == currbodyanim->offset + currbodyanim->loop_frames)
				bodyframe = currbodyanim->offset;
			else if (bodyframe + 1 < currbodyanim->offset + currbodyanim->num_frames)
				bodyframe++;
			bodyanimtime = cl.time;
		}
	}
	else
	{
		bodyframe = currbodyanim->offset;
		bodyanimtime = cl.time;
	}

	if (legsanim == oldlegsanim)
	{
		if (cl.time >= legsanimtime + currlegsanim->interval)
		{
			if (currlegsanim->loop_frames && legsframe + 1 == currlegsanim->offset + currlegsanim->loop_frames)
				legsframe = currlegsanim->offset;
			else if (legsframe + 1 < currlegsanim->offset + currlegsanim->num_frames)
				legsframe++;
			legsanimtime = cl.time;
		}
	}
	else
	{
		legsframe = currlegsanim->offset;
		legsanimtime = cl.time;
	}

	oldbodyanim = bodyanim;
	oldlegsanim = legsanim;
}

int			multimodel_level;
qbool	surface_transparent;

/*
=================
R_DrawQ3Frame
=================
*/

static qbool full_light;
static float	shadelight, ambientlight;

//#ifdef _DEBUG
// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
float	r_avertexnormal_dots[SHADEDOT_QUANT][256] =
#include "model_alias_anorm_dots.h"
;

float	*shadedots = r_avertexnormal_dots[0];
		vec3_t	shadevector;

//float	apitch, ayaw;
//#endif

void R_DrawQ3Frame (int frame, md3header_t *pmd3hdr, md3surface_t *pmd3surf, entity_t *ent, int distance)
{
	int			i, j, numtris, pose, pose1, pose2;
	float		l, lerpfrac;
	vec3_t		lightvec, interpolated_verts;
	unsigned int *tris;
	md3tc_t		*tc;
	md3vert_mem_t *verts, *v1, *v2;
	model_t		*clmodel = ent->model;
	float iblend = 1;

	if ((frame >= pmd3hdr->numframes) || (frame < 0))
	{
		Con_DevPrintf (DEV_MODEL, "R_DrawQ3Frame: no such frame %d\n", frame);
		frame = 0;
	}

	if (ent->currentpose >= pmd3hdr->numframes)
		ent->currentpose = 0;

	pose = frame;

	if (COM_StringMatch (clmodel->name, "progs/player/lower.md3"))
		ent->lerptime = anims[legsanim].interval;
	else if (COM_StringMatch (clmodel->name, "progs/player/upper.md3"))
		ent->lerptime = anims[bodyanim].interval;
	else
		ent->lerptime = 0.1;

	if (ent->previouspose != pose)
	{
		ent->lerpstart = cl.time;
		ent->currentpose = ent->previouspose;
		ent->previouspose = pose;
//		ent->framelerp = 0;
		iblend = 0;
	}
	else
	{
//		ent->framelerp = (cl.time - ent->lerpstart) / ent->lerptime;
		iblend = (cl.time - ent->lerpstart) / ent->lerptime;
	}

	// weird things start happening if blend passes 1
//	if (cl.paused || ent->framelerp > 1)
//		ent->framelerp = 1;

	if (cl.paused || iblend > 1)
		iblend = 1;


	verts = (md3vert_mem_t *)((byte *)pmd3hdr + pmd3surf->ofsverts);
	tc = (md3tc_t *)((byte *)pmd3surf + pmd3surf->ofstc);
	tris = (unsigned int *)((byte *)pmd3surf + pmd3surf->ofstris);
	numtris = pmd3surf->numtris * 3;
	pose1 = ent->currentpose * pmd3surf->numverts;
	pose2 = ent->previouspose * pmd3surf->numverts;

/*
#if 0
	if (surface_transparent)
	{
		glEnable (GL_BLEND);
//		if (clmodel->modhint == MOD_Q3GUNSHOT || clmodel->modhint == MOD_Q3TELEPORT)
//			glBlendFunc (GL_SRC_ALPHA, GL_ONE);
//		else
			glBlendFunc (GL_ONE, GL_ONE);
		glDepthMask (GL_FALSE);
		glDisable (GL_CULL_FACE);
	}
	else if (ISTRANSPARENT(ent))
	{
		glEnable (GL_BLEND);
	}
#endif
*/
	eglBegin (GL_TRIANGLES);
	for (i = 0 ; i < numtris ; i++)
	{
		float	s, t;

		v1 = verts + *tris + pose1;
		v2 = verts + *tris + pose2;

//		if (clmodel->modhint == MOD_Q3TELEPORT)
//			s = tc[*tris].s, t = tc[*tris].t * 4;
//		else
			s = tc[*tris].s, t = tc[*tris].t;

		if (gl_mtexable)
		{
			qglMultiTexCoord2f (GL_TEXTURE0_ARB, s, t);
			qglMultiTexCoord2f (GL_TEXTURE1_ARB, s, t);
		}
		else
		{
			eglTexCoord2f (s, t);
		}

//		lerpfrac = VectorL2Compare(v1->vec, v2->vec, distance) ? ent->framelerp : 1;
			lerpfrac = VectorL2Compare(v1->vec, v2->vec, distance) ? iblend : 1;


//#ifndef _DEBUG
		if (r_vertex_lights.integer && !full_light)
		{
			float VertexLights_LerpVertexLightMD3 (byte ppitch1, byte pyaw1, byte ppitch2, byte pyaw2, float ilerp, float apitch, float ayaw);
			l = VertexLights_LerpVertexLightMD3 (v1->anorm_pitch, v1->anorm_yaw, v2->anorm_pitch, v2->anorm_yaw, lerpfrac, -ent->angles[0] /*apitch*/, ent->angles[1] /* ayaw */);
// Baker: This is supposed to be using pitch and yaw to get or generate the lightnormalindex somehow
// Then we are interpolating
			l = VertexLights_LerpVertexLight ((int)v1->oldnormal, (int)v2->oldnormal, lerpfrac, v2->anorm_pitch, v2->anorm_yaw);
			l = min(l, 1);

			for (j = 0 ; j < 3 ; j++)
				lightvec[j] = lightpoint_lightcolor[j] / 256 + l;
			MeglColor4f (lightvec[0], lightvec[1], lightvec[2], ENTALPHA_DECODE(ent->alpha));
		}
		else
//#endif

		{
			l = FloatInterpolate (shadedots[v1->oldnormal>>8], lerpfrac, shadedots[v2->oldnormal>>8]);
			l = (l * shadelight + ambientlight) / 256;
			l = min(l, 1);

//			MeglColor4f (l, l, l, ent->transparency);
			MeglColor4f (l, l, l, ENTALPHA_DECODE(ent->alpha));
//			MeglColor4f (255, 255, 255, 1);
		}

//		VectorInterpolate (v1->vec, lerpfrac, v2->vec, interpolated_verts);
		LerpVector (v1->vec, v2->vec, lerpfrac, interpolated_verts);
		eglVertex3fv (interpolated_verts);

		*tris++;
	}
	eglEnd ();

/*
#ifndef _DEBUG
	if (gl_shownormals.value)
	{
		vec3_t	temp;

		tris = (unsigned int *)((byte *)pmd3surf + pmd3surf->ofstris);
		eglDisable (GL_TEXTURE_2D);
		eglColor3ubv (color_white);
		eglBegin (GL_LINES);
		for (i=0 ; i<numtris ; i++)
		{
			glVertex3fv (verts[*tris+pose1].vec);
			VectorMultiplyAdd (verts[*tris+pose1].vec, 2, verts[*tris+pose1].normal, temp);
			glVertex3fv (temp);
			*tris++;
		}
		glEnd ();
		glEnable (GL_TEXTURE_2D);
	}
#endif
*/

/*
#if 0
	if (surface_transparent)
	{
		glDisable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask (GL_TRUE);
		glEnable (GL_CULL_FACE);
	}
	else if (ISTRANSPARENT(ent))
	{
		glDisable (GL_BLEND);
	}
#endif
*/
}

/*
=================
R_DrawQ3Shadow
=================
*/
/*
void R_DrawQ3Shadow (entity_t *ent, float lheight, float s1, float c1, trace_t downtrace)
{
	int			i, j, numtris, pose1, pose2;
	vec3_t		point1, point2, interpolated;
	md3header_t	*pmd3hdr;
	md3surface_t *pmd3surf;
	unsigned int *tris;
	md3vert_mem_t *verts;
	model_t		*clmodel = ent->model;
#if 0
	float		m[16];
	md3tag_t	*tag;
	tagentity_t	*tagent;
#endif
	float		iblend = 1;

	pmd3hdr = (md3header_t *)Mod_Extradata (clmodel);

	pmd3surf = (md3surface_t *)((byte *)pmd3hdr + pmd3hdr->ofssurfs);
	for (i = 0 ; i < pmd3hdr->numsurfs ; i++)
	{
		verts = (md3vert_mem_t *)((byte *)pmd3hdr + pmd3surf->ofsverts);
		tris = (unsigned int *)((byte *)pmd3surf + pmd3surf->ofstris);
		numtris = pmd3surf->numtris * 3;
		pose1 = ent->currentpose * pmd3surf->numverts;
		pose2 = ent->previouspose * pmd3surf->numverts;

		eglBegin (GL_TRIANGLES);
		for (j = 0 ; j < numtris ; j++)
		{
			// normals and vertexes come from the frame list
			VectorCopy (verts[*tris+pose1].vec, point1);

			point1[0] -= shadevector[0] * (point1[2] + lheight);
			point1[1] -= shadevector[1] * (point1[2] + lheight);

			VectorCopy (verts[*tris+pose2].vec, point2);

			point2[0] -= shadevector[0] * (point2[2] + lheight);
			point2[1] -= shadevector[1] * (point2[2] + lheight);

//			VectorInterpolate (point1, ent->framelerp, point2, interpolated);
			LerpVector (point1, point2, iblend, interpolated);

			interpolated[2] = -(ent->origin[2] - downtrace.endpos[2]);

			interpolated[2] += ((interpolated[1] * (s1 * downtrace.plane.normal[0])) -
								(interpolated[0] * (c1 * downtrace.plane.normal[0])) -
								(interpolated[0] * (s1 * downtrace.plane.normal[1])) -
								(interpolated[1] * (c1 * downtrace.plane.normal[1]))) +
								((1 - downtrace.plane.normal[2]) * 20) + 0.2;

			glVertex3fv (interpolated);

			*tris++;
		}
		glEnd ();

		pmd3surf = (md3surface_t *)((byte *)pmd3surf + pmd3surf->ofsend);
	}

	if (!pmd3hdr->numtags)	// single model, done
		return;

// no multimodel shadow support yet
#if 0
	tag = (md3tag_t *)((byte *)pmd3hdr + pmd3hdr->ofstags);
	tag += ent->previouspose * pmd3hdr->numtags;
	for (i = 0 ; i < pmd3hdr->numtags ; i++, tag++)
	{
		if (multimodel_level == 0 && COM_StringMatch (tag->name, "tag_torso"))
		{
			tagent = &q3player_body;
			ent = &q3player_body.ent;
			multimodel_level++;
		}
		else if (multimodel_level == 1 && COM_StringMatch (tag->name, "tag_head"))
		{
			tagent = &q3player_head;
			ent = &q3player_head.ent;
			multimodel_level++;
		}
		else
		{
			continue;
		}

		glPushMatrix ();
		R_RotateForTagEntity (tagent, tag, m);
		glMultMatrixf (m);
		R_DrawQ3Shadow (ent, lheight, s1, c1, downtrace);
		glPopMatrix ();
	}
#endif
}
*/
/*
#define	ADD_EXTRA_TEXTURE(_texture, _param)					\
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, _param);\
	GL_Bind (_texture);										\
															\
	glDepthMask (GL_FALSE);									\
	glEnable (GL_BLEND);									\
	glBlendFunc (GL_ONE, GL_ONE);							\
															\
	R_DrawQ3Frame (frame, pmd3hdr, pmd3surf, ent, INTERP_MAXDIST);\
															\
	glDisable (GL_BLEND);									\
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		\
	glDepthMask (GL_TRUE);
*/

/*
=================
R_SetupQ3Frame
=================
*/
void R_SetupQ3Frame (entity_t *ent)
{
	int			i, j, frame, shadernum, texture, fb_texture;
	float		m[16];
	md3header_t	*pmd3hdr;
	md3surface_t *pmd3surf;
	md3tag_t	*tag;
	model_t		*clmodel = ent->model;
	tagentity_t	*tagent;

	MeglDisable (GL_ALPHA_TEST);

	if (COM_StringMatch (clmodel->name, "progs/player/lower.md3"))
		frame = legsframe;
	else if (COM_StringMatch (clmodel->name, "progs/player/upper.md3"))
		frame = bodyframe;
	else
		frame = ent->frame;

	// locate the proper data
	pmd3hdr = (md3header_t *)Mod_Extradata (clmodel);

	// draw all the triangles

	// draw non-transparent surfaces first, then the transparent ones
	for (i = 0 ; i < 2 ; i++)
	{
		pmd3surf = (md3surface_t *)((byte *)pmd3hdr + pmd3hdr->ofssurfs);
		for (j = 0 ; j < pmd3hdr->numsurfs ; j++)
		{
			md3shader_mem_t	*shader;
/*
			surface_transparent = ( strstr(pmd3surf->name, "energy") ||
									strstr(pmd3surf->name, "f_") ||
									strstr(pmd3surf->name, "flare") ||
									strstr(pmd3surf->name, "flash") ||
									strstr(pmd3surf->name, "Sphere") ||
									strstr(pmd3surf->name, "telep"));
*/
			surface_transparent = false;
			if ((!i && surface_transparent) || (i && !surface_transparent))
			{
				pmd3surf = (md3surface_t *)((byte *)pmd3surf + pmd3surf->ofsend);
				continue;
			}

			c_md3_polys += pmd3surf->numtris;

			shadernum = ent->skinnum;
			if ((shadernum >= pmd3surf->numshaders) || (shadernum < 0))
			{
				Con_DevPrintf (DEV_MODEL, "R_SetupQ3Frame: no such skin # %d\n", shadernum);
				shadernum = 0;
			}

			shader = (md3shader_mem_t *)((byte *)pmd3hdr + pmd3surf->ofsshaders);

			texture = shader[shadernum].gl_texnum;
			fb_texture = shader[shadernum].fb_texnum;

			if (fb_texture && gl_mtexable)
			{
				GL_DisableMultitexture ();
				MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				GL_Bind (texture);

				GL_EnableMultitexture ();
				if (gl_add_ext)
				{
					MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
					GL_Bind (fb_texture);
				}
#define INTERP_MAXDIST 300
				R_DrawQ3Frame (frame, pmd3hdr, pmd3surf, ent, INTERP_MAXDIST);
/*
				if (!gl_add_ext)
				{
					ADD_EXTRA_TEXTURE(fb_texture, GL_DECAL);
				}
*/
				GL_DisableMultitexture ();
			}
			else
			{
				GL_DisableMultitexture ();
				MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				GL_Bind (texture);

				R_DrawQ3Frame (frame, pmd3hdr, pmd3surf, ent, INTERP_MAXDIST);
/*
				if (fb_texture)
				{
					ADD_EXTRA_TEXTURE(fb_texture, GL_REPLACE);
				}
*/
			}

			pmd3surf = (md3surface_t *)((byte *)pmd3surf + pmd3surf->ofsend);
		}
	}

	if (!pmd3hdr->numtags)	// single model, done
		return;

	tag = (md3tag_t *)((byte *)pmd3hdr + pmd3hdr->ofstags);
	tag += frame * pmd3hdr->numtags;
	for (i = 0 ; i < pmd3hdr->numtags ; i++, tag++)
	{
		extern	tagentity_t	q3player_body, q3player_head;
		if (multimodel_level == 0 && COM_StringMatch (tag->name, "tag_torso"))
		{

			tagent = &q3player_body;
			ent = &q3player_body.ent;
			multimodel_level++;
		}
		else if (multimodel_level == 1 && COM_StringMatch (tag->name, "tag_head"))
		{
			tagent = &q3player_head;
			ent = &q3player_head.ent;
			multimodel_level++;
		}
		else
		{
			continue;
		}

		eglPushMatrix ();
		R_RotateForTagEntity (tagent, tag, m);
		eglMultMatrixf (m);
		R_SetupQ3Frame (ent);
		eglPopMatrix ();
	}

}

/*
=============
R_SetupLighting
=============
*/
vec3_t lightcolor;
void R_SetupLighting (entity_t *ent)
{
	int		i, lnum;
	float	add;
	vec3_t	dist, dlight_color;
	model_t	*clmodel = ent->model;
#if 1
	// make thunderbolt and torches full light
	if (clmodel->modhint == MOD_THUNDERBOLT)
	{
		ambientlight = 210;
		shadelight = 0;
		full_light = /*ent->noshadow =  */ true;
		return;
	}
	else if (clmodel->modhint == MOD_FLAME)
	{
		ambientlight = 255;
		shadelight = 0;
		full_light = /* ent->noshadow = */ true;
		return;
	}
//	else if (clmodel->modhint == MOD_Q3GUNSHOT || clmodel->modhint == MOD_Q3TELEPORT)
//	{
//		ambientlight = 128;
//		shadelight = 0;
//		full_light = ent->noshadow = true;
//		return;
//	}
#endif
	// normal lighting
	ambientlight = shadelight = Light_LightPoint (ent->origin);
 	full_light = false;
//	ent->noshadow = false;
#if 1
	for (lnum = 0 ; lnum < MAX_DLIGHTS ; lnum++)
	{
		if (cl_dlights[lnum].die < cl.time || !cl_dlights[lnum].radius)
			continue;

		VectorSubtract (ent->origin, cl_dlights[lnum].origin, dist);
		add = cl_dlights[lnum].radius - VectorLength (dist);

		if (add > 0)
		{
		// joe: only allow colorlight affection if dynamic lights are on
			if (scene_dynamiclight.integer)
			{
				VectorCopy (bubblecolor[cl_dlights[lnum].color_type], dlight_color);
				for (i = 0 ; i < 3 ; i++)
				{
					lightcolor[i] = lightcolor[i] + (dlight_color[i] * add) * 2;
					if (lightcolor[i] > 256)
					{
						switch (i)
						{
						case 0:
							lightcolor[1] = lightcolor[1] - (lightcolor[1] / 3);
							lightcolor[2] = lightcolor[2] - (lightcolor[2] / 3);
							break;

						case 1:
							lightcolor[0] = lightcolor[0] - (lightcolor[0] / 3);
							lightcolor[2] = lightcolor[2] - (lightcolor[2] / 3);
							break;

						case 2:
							lightcolor[1] = lightcolor[1] - (lightcolor[1] / 3);
							lightcolor[0] = lightcolor[0] - (lightcolor[0] / 3);
							break;
						}
					}
				}
			}
			else
			{
				ambientlight += add;
			}
		}
	}
#endif
	// calculate pitch and yaw for vertex lighting
#if 0
	if (gl_vertexlights.value)
	{
		apitch = ent->angles[0];
		ayaw = ent->angles[1];
	}
#endif
	// clamp lighting so it doesn't overbright as much
	ambientlight = min(128, ambientlight);
	if (ambientlight + shadelight > 192)
		shadelight = 192 - ambientlight;
#if 0
	if (ent == &cl.viewent)
	{
		ent->noshadow = true;
		// always give the gun some light
		if (ambientlight < 24)
			ambientlight = shadelight = 24;
	}
#endif
	// never allow players to go totally black
	if (clmodel->modhint == MOD_PLAYER)
	{
		if (ambientlight < 8)
			ambientlight = shadelight = 8;
#if 0
		if (r_fullbrightskins.value)
		{
			ambientlight = shadelight = 128;
			full_light = true;
		}
#endif
	}

#if 0
	if (r_fullbrightskins.value == 2 &&
		(ent->modelindex == cl_modelindex[mi_fish] ||
		ent->modelindex == cl_modelindex[mi_dog] ||
		ent->modelindex == cl_modelindex[mi_soldier] ||
		ent->modelindex == cl_modelindex[mi_enforcer] ||
		ent->modelindex == cl_modelindex[mi_knight] ||
		ent->modelindex == cl_modelindex[mi_hknight] ||
		ent->modelindex == cl_modelindex[mi_scrag] ||
		ent->modelindex == cl_modelindex[mi_ogre] ||
		ent->modelindex == cl_modelindex[mi_fiend] ||
		ent->modelindex == cl_modelindex[mi_vore] ||
		ent->modelindex == cl_modelindex[mi_shambler]))
	{
		ambientlight = shadelight = 128;
		full_light = true;
	}
#endif
}


/*
=================
R_DrawQ3Model
=================
*/
vec3_t r_entorigin;
vec3_t modelorg;
void R_DrawQ3Model (entity_t *ent)
{
	vec3_t		mins, maxs, md3_scale_origin = {0, 0, 0};
	model_t		*clmodel = ent->model;

//	if (clmodel->modhint == MOD_Q3TELEPORT)
//		ent->origin[2] -= 30;

	VectorAdd (ent->origin, clmodel->mins, mins);
	VectorAdd (ent->origin, clmodel->maxs, maxs);

	if (ent->angles[0] || ent->angles[1] || ent->angles[2])
	{
		if (R_CullSphere(ent->origin, clmodel->radius))
			return;
	}
	else
	{
		if (R_CullBox(mins, maxs))
			return;
	}

	VectorCopy (ent->origin, r_entorigin);
	VectorSubtract (r_origin, r_entorigin, modelorg);

	// get lighting information


	R_SetupLighting (ent);


//	ambientlight = 210;
//	shadelight = 0;
//	full_light = true ; //ent->noshadow = true;

	shadedots = r_avertexnormal_dots[((int)(ent->angles[1] * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];

	eglPushMatrix ();

/*
	if (ent == &cl.viewmodel_ent)
		R_RotateForViewEntity (ent);
	else
*/
//		R_RotateForEntity (ent, false);
	R_RotateForEntity (ent->origin, ent->angles);

	if (ent == &cl.viewmodel_ent)
	{
		float	scale = 0.5 + CLAMP(0, r_viewmodel_size.floater, 1) / 2;

		eglTranslatef (md3_scale_origin[0], md3_scale_origin[1], md3_scale_origin[2]);
		eglScalef (scale, 1, 1);
	}
	else
	{
		eglTranslatef (md3_scale_origin[0], md3_scale_origin[1], md3_scale_origin[2]);
	}

	if (gl_smoothmodels.floater)
		MeglShadeModel (GL_SMOOTH);

	if (gl_affinemodels.floater)
		eglHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

	if (COM_StringMatch (clmodel->name, "progs/player/lower.md3"))
	{
		R_ReplaceQ3Frame (ent->frame);
//		ent->noshadow = true;
	}

	multimodel_level = 0;
	R_SetupQ3Frame (ent);

	MeglShadeModel (GL_FLAT);
	if (gl_affinemodels.floater)
		eglHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	eglPopMatrix ();
/*
	if (r_shadows.integer  && !ent->noshadow)
	{
		int			farclip;
		float		theta, lheight, s1, c1;
		vec3_t		downmove;
		trace_t		downtrace;
		static float shadescale = 0;

		farclip = max((int)r_farclip.floater, 4096);

		if (!shadescale)
			shadescale = 1 / sqrt(2);
		theta = -ent->angles[1] / 180 * M_PI;

		VectorSet (shadevector, cos(theta) * shadescale, sin(theta) * shadescale, shadescale);

		glPushMatrix ();

//		R_RotateForEntity (ent, true);
		R_RotateForEntity (ent->currentorigin, ent->currentangles);

		VectorCopy (ent->origin, downmove);
		downmove[2] -= farclip;
		memset (&downtrace, 0, sizeof(downtrace));
		SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, ent->origin, downmove, &downtrace);

		lheight = 0; // ent->origin[2] - lightspot[2];

		s1 = sin(ent->angles[1] / 180 * M_PI);
		c1 = cos(ent->angles[1] / 180 * M_PI);

		glDepthMask (GL_FALSE);
		glDisable (GL_TEXTURE_2D);
		glEnable (GL_BLEND);
		glColor4f (0, 0, 0, (ambientlight - (mins[2] - downtrace.endpos[2])) / 150);
#if 0	// Baker: Stencil!
		if (gl_have_stencil && r_shadows.value == 2)
		{
			glEnable (GL_STENCIL_TEST);
			glStencilFunc (GL_EQUAL, 1, 2);
			glStencilOp (GL_KEEP, GL_KEEP, GL_INCR);
		}
#endif
		multimodel_level = 0;
		R_DrawQ3Shadow (ent, lheight, s1, c1, downtrace);

		glDepthMask (GL_TRUE);
		glEnable (GL_TEXTURE_2D);
		glDisable (GL_BLEND);
#if 0
		if (gl_have_stencil && r_shadows.value == 2)
			glDisable (GL_STENCIL_TEST);
#endif
		glPopMatrix ();
	}
*/
	MeglColor3ubv (color_white);
}
