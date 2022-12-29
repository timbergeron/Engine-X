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
===============================================================================

				ALIAS MODELS

===============================================================================
*/




//static qbool full_light;
//static float	shadelight, ambientlight;




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

static int DoWeaponInterpolation (entity_t *this_entity);


void GL_DrawAliasFrameVerts (/*const entity_t *my_glent, */ const aliashdr_t *paliashdr, const lerpdata_t lerpdata, const qbool mtexable, const qbool isViewWeapon, const float entalpha, const int maxDistance, const qbool shadowpass, const vec3_t myShadeVector, const float shadow_height, const float shadow_lheight);

#define INTERP_WEAP_MAXNUM		24
#define INTERP_WEAP_MINDIST		5000
#define INTERP_WEAP_MAXDIST		95000

/*
=================
R_SetupAliasFrame -- johnfitz -- rewritten to support lerping
=================
*/
//extern qbool start_of_demo;
static void R_SetupAliasFrame (entity_t *e, const aliashdr_t *paliashdr, const int in_frame, lerpdata_t *lerpdata /*int distance, qbool mtex*/)
{
	const qbool		FrameIsValid	= !(in_frame >= paliashdr->numframes) || (in_frame < 0);
	const int		myFrame			= FrameIsValid ?  in_frame : 0;

	if (FrameIsValid == false)		Con_DevPrintf (DEV_MODEL, "R_AliasSetupFrame: no such frame %d\n", in_frame);

	do
	{
		int		posenum		= paliashdr->frames[myFrame].firstpose;
		int		numposes	= paliashdr->frames[myFrame].numposes;
		
		// Advance the pose x number of poses according to the time, if a multipose frame
		e->lerptime								= (numposes > 1) ? paliashdr->frames[myFrame].interval : 0.1;
		
		if (numposes > 1)	posenum				+= (int)(cl.time / e->lerptime) % numposes;

		// Modify the entity information
		
		if (e->lerpflags & LERP_RESETANIM)												// Situation 1: kill any lerp in progress
		{
			e->lerpstart						= 0;
			e->currentpose = e->previouspose	= posenum;
			e->lerpflags						-= LERP_RESETANIM;
		}
		else if (e->currentpose != posenum)												// Situation 2:  pose changed, start new lerp
		{
			if (e->lerpflags & LERP_RESETANIM2) //defer lerping one more time			// Situation 2a
			{
				e->lerpstart					= 0;
				e->previouspose					= posenum;
				e->lerpflags					-= LERP_RESETANIM2;
			}
			else																		// Situation 2b
			{
				e->lerpstart					= cl.time;
				e->previouspose					= e->currentpose;
			}
			e->currentpose						= posenum;								// Common
		}																				// Situation 3: only 1 pose, but does not seem to need addressed
		
		// Modify and setup values for the lerpdata structure
		if (scene_lerpmodels.integer && !(e->model->modelflags & MOD_NOLERP && scene_lerpmodels.integer != 2))
		{
			// Calculate blend percent:  If lerp is finished with one pose, percent of end minus start, otherwise percent of intended frame interval (or 1/10th of a second if single pose)
			lerpdata->blend						= (cl.time - e->lerpstart) / ((e->lerpflags & LERP_FINISH && numposes == 1)  ? (e->lerpfinish - e->lerpstart) :  e->lerptime);
			lerpdata->blend						= CLAMP (0, lerpdata->blend, 1);
			lerpdata->iblend					= 1-lerpdata->blend;

			lerpdata->pose1						= e->previouspose;
			lerpdata->pose2						= e->currentpose;
			lerpdata->isLerpingAnimation		= (lerpdata->pose1 != lerpdata->pose2);	// It is lerping if the poses are not the same
		}
		else //don't lerp
		{
			lerpdata->blend						= 1;
			lerpdata->iblend					= 0;  // 1-blend
			lerpdata->pose2 = lerpdata->pose1	= posenum;
			lerpdata->isLerpingAnimation		= false;
		}

	} while (0);

}

#define VectorFlow(a, b, c)					((c)[0] = (b)[0], (c)[1] = (b)[1], (c)[2] = (b)[2], (b)[0] = (a)[0], (b)[1] = (a)[1], (b)[2] = (a)[2])
#define VectorCopy2(a, b, c)				((c)[0] = (b)[0] = (a)[0], (c)[1] = (b)[1] = (a)[1], (c)[2] = (b)[2] = (a)[2])

/*
=================
R_SetupEntityTransform -- johnfitz -- set up transform part of lerpdata
=================
*/
static void R_SetupEntityTransform (entity_t *e, lerpdata_t *lerpdata)
{

	// if LERP_RESETMOVE, kill any lerps in progress
	if (e->lerpflags & LERP_RESETMOVE)
	{
		e->movelerpstart = 0;

		VectorCopy2	(e->origin, e->currentorigin, e->previousorigin);	// 3 = 2 = 1
		VectorCopy2	(e->angles, e->currentangles, e->previousangles);	// 3 = 2 = 1

		e->lerpflags -= LERP_RESETMOVE;
	}
	else if (!VectorCompare (e->origin, e->currentorigin) || !VectorCompare (e->angles, e->currentangles)) // origin/angles changed, start new lerp
	{
		e->movelerpstart = cl.time;

		VectorFlow	(e->origin, e->currentorigin, e->previousorigin);	// 3 = 2 ... 2 = 1
		VectorFlow	(e->angles, e->currentangles, e->previousangles);	// 3 = 2 ... 2 = 1
	}
	// Third situation is that the angles and origins didn't change and in that event we do nothing


	//set up values - lerp move only applies to movetype styep and never to the viewmodel.
	if (scene_lerpmove.floater && e != &cl.viewmodel_ent && e->lerpflags & LERP_MOVESTEP)
	{

		// If a LERP_FINISH has been indicated, use that otherwise adjust to 10 frames per second?
		float		divisor	= (e->lerpflags & LERP_FINISH) ? (e->lerpfinish - e->movelerpstart) : 0.1;
		float		blend	= (cl.time - e->movelerpstart) / divisor;
		blend				= CLAMP (0, blend, 1);															// Limit range to 0 to 1
		do
		{
			vec3_t		delta_temp_vector;

			//translation
			VectorSubtract			(e->currentorigin, e->previousorigin, delta_temp_vector);
			VectorMultiplyAdd		(e->previousorigin, blend, delta_temp_vector, lerpdata->origin);

			//rotation
			VectorSubtract			(e->currentangles, e->previousangles, delta_temp_vector);
			VectorShortenAngles		(delta_temp_vector);	// if -180 <= angles < 180, do nothing otherwise ... shortest path
			VectorMultiplyAdd		(e->previousangles, blend, delta_temp_vector, lerpdata->angles);
		} while (0);
	}
	else //don't lerp
	{
		VectorCopy (e->origin, lerpdata->origin);
		VectorCopy (e->angles, lerpdata->angles);
	}

}

/*
=================
R_DrawModel_Alias
=================
*/

void AliasLight_Blend_DynamicLightColor_Into_Color (const float luminance, const vec3_t dynamic_rgb_color, vec3_t set_inlightcolor)
{
	int		i, j;
	for (i=0 ; i<3 ; i++)
	{
		set_inlightcolor[i] = set_inlightcolor[i] + (dynamic_rgb_color[i] * luminance) * 2;
		if (lightpoint_lightcolor[i] > 256)	// If overbrighted, reduce other 2 colors out a bit
			for (j = 0; j<3; j++)
			{
				if (j == i)		continue; // Don't reduce own color
				set_inlightcolor[j] = set_inlightcolor[j] - (1 * set_inlightcolor[j]/3);
			}

	}
}

void R_MoreRotateForEntity_Well_Actually_Translate_And_Scale(const aliashdr_t *pAliasHeader, const modhint_t my_modhint, const qbool isviewModel, const float fov_x, const float viewmodelsize)
{
	if (my_modhint == MOD_EYES && cl_ent_doubleeyes.integer)
	{
		// double size of eyes, since they are really hard to see in gl or so they say ...
		eglTranslatef (pAliasHeader->scale_origin[0], pAliasHeader->scale_origin[1], pAliasHeader->scale_origin[2] - (22 + 8));
		eglScalef (pAliasHeader->scale[0]*2, pAliasHeader->scale[1]*2, pAliasHeader->scale[2]*2);
	}
	else if (isviewModel && r_viewmodel_fovscale.integer)
	{
		float	scale = (0.5f + CLAMP(0, viewmodelsize, 1) / 2.0f) / tanf(DEG2RAD(fov_x/2)) * r_viewmodel_fovscale.floater/90;

		eglTranslatef(pAliasHeader->scale_origin[0]*scale, pAliasHeader->scale_origin[1], pAliasHeader->scale_origin[2]);
		eglScalef(pAliasHeader->scale[0] * scale, pAliasHeader->scale[1], pAliasHeader->scale[2]);
	}
	else
	{
		eglTranslatef (pAliasHeader->scale_origin[0], pAliasHeader->scale_origin[1], pAliasHeader->scale_origin[2]);
		eglScalef (pAliasHeader->scale[0], pAliasHeader->scale[1], pAliasHeader->scale[2]);
	}
}


qbool Alias_SetSpecialLighting(const modhint_t my_modhint, qbool *set_full_light, float *set_shadelight, float *set_ambientlight);
extern float vlight_pitch, vlight_yaw;
void R_DrawModel_Alias (entity_t *ent, const qbool shadowpass)
{
	aliashdr_t	*paliashdr = (aliashdr_t *)Mod_Extradata (ent->model);
	int			client_no		= ent - cl_entities;
	qbool		isPlayer		= (client_no >= 1 && client_no<=cl.maxclients);
	qbool		isViewModel		= (ent == &cl.viewmodel_ent);
	model_t		*clmodel		= ent->model;
	float		entalpha		= ENTALPHA_DECODE(ent->alpha);
//	int			i;
	lerpdata_t	lerpdata;
	int			maxDistance;

	int			anim;
	int			skinnum;//, distance;

//	qbool		noshadow = false;

	int			texture, fb_texture = 0;

	if (!r_drawmodels_alias.integer) return;
//	if (entalpha < 1 && !r_drawmodels_alphaents.integer) return;

	if (r_drawmodels_alias.integer == -1 && clmodel->modelflags & MOD_RENDERADDITIVE) return; // Not additive
	if (r_drawmodels_alias.integer == -2 && clmodel->modelflags & MOD_RENDERFENCE) return; // Not fence
	if (r_drawmodels_alias.integer == -3 && clmodel->modelflags & MOD_RENDERFILTER) return; // Not filter



	if (shadowpass && (isViewModel || (ent->model->modelflags & MOD_NOSHADOW)))
		return; // We do not do shadows on these
	//
	// setup pose/lerp data -- do it first so we don't miss updates due to culling
	//
	R_SetupAliasFrame (ent, paliashdr, ent->frame, &lerpdata);
	R_SetupEntityTransform (ent, &lerpdata);

	lerpdata.full_light		= 0;
	lerpdata.shade_light	= 0;
	lerpdata.ambient_light	= 0;


	//
	// cull it
	//

	if (!shadowpass && R_CullForEntity (ent))
		return;

	// Adjust the entalpha we use for rendering if we have a viewmodel
	if (isViewModel)
		if (cl.items & IT_INVISIBILITY)
			entalpha = CLAMP(0, r_viewmodel_ringalpha.floater, 1);
		else
			entalpha = CLAMP(0, r_drawviewmodel.floater, 1);

	// Calculate the vert max interpolation distance
	if ((maxDistance = DoWeaponInterpolation(ent)) != -1)
		maxDistance = maxDistance;
	else if (isViewModel)
		maxDistance = (int)r_vertex_blenddist.floater;
	else
		maxDistance = INTERP_WEAP_MAXDIST;


	// normal lighting
 	lerpdata.ambient_light = lerpdata.shade_light = Light_LightPoint (ent->origin);		// Baker: done before shadows to get the lighting spot
	lerpdata.full_light = false;

	// Baker: Draw shadows before model.  FitzQuake draws shadows before ALL models.  Maybe we should too.
	// Shadow phase

	if (shadowpass)
	{

		// Baker:	Things wrong with shadows
		//			They can "clip" into monster's feet
		//			They don't clip against polygons
		//			Lightpoint's trace is wrong especially for shadow_height and lheight and can pick the wrong surface
		//			They don't affect multiple surfaces
		//			Nitpick: Culled entities don't have shadows even if the shadow point wouldn't be culled
		//			The current version of mine can't do less than full shadows (other blendfuncs make it so you can see thru submodel surfaces)
		//			And since it is "full", it cannot honor entity alpha
		//			And finally, can't implement some reduction of shadow formula like R00k uses
		//          R00k takes into account lightmap lighting, entity alpha
		//          R00k even has shadows on angles of surfaces


		vec3_t	shadevector;
		float lheight;
		float height;
		float dist = ent->origin[2]+ent->model->mins[2]-lightpoint_lightspot[2];
		float factor = r_shadows.integer == 2 ? 1 : ((lerpdata.shade_light+lerpdata.ambient_light)/2 - dist) *r_shadows.floater *0.0066 *entalpha;//R00k user-controlled shadow values

		factor = CLAMP(0, factor, 1);
//		Con_Printf("Factor is %2.2f and dist is %4.2f\n", factor, dist);
		// Calculate shadevector and heights
		{
			float		an;
			static	float	shadescale = 0;

			if (!shadescale)
				shadescale = 1 / sqrtf(2);
			an = -ent->angles[1] / 180 * M_PI;

			VectorSet (shadevector, cosf(an) * shadescale, sinf(an) * shadescale, shadescale);
			lheight = ent->origin[2] - lightpoint_lightspot[2];
			height = 1 - lheight;
		}

		mglPushStates (); eglPushMatrix ();

//		eglBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);	// All black but ok

//		eglBlendFunc (GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

//		MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		MeglDisable (GL_TEXTURE_2D);
		MeglEnable (GL_BLEND);
		MeglColor3f (factor, factor, factor);	// Should be modified by entalpha
		MeglBlendFunc (GL_ZERO, GL_ONE_MINUS_SRC_COLOR);	// All black but ok
		mglFinishedStates ();

		eglTranslatef (lerpdata.origin[0], lerpdata.origin[1], lerpdata.origin[2]);
		eglRotatef (lerpdata.angles[1], 0, 0, 1);


		GL_DrawAliasFrameVerts (paliashdr, lerpdata, 0 , isViewModel, entalpha, maxDistance, true, shadevector, height, lheight);

		MeglEnable (GL_TEXTURE_2D);
		MeglDisable (GL_BLEND);

		MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//		MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);


		MeglColor3ubv (color_white);

		eglPopMatrix ();  mglPopStates ();
		return;
	}
	// End shadow phase

	if (clmodel->modelflags & MOD_FBRIGHTHACK)
	{
		lerpdata.ambient_light = 255;
		lerpdata.shade_light = 0;
		lerpdata.full_light = true;
	}
	else // Calculate lighting
	{
		float	radiusmax		= 0.0; // dlight thingy used for vertexlights
		int		lnum;
		vec3_t	vertexlight;

		// Add dynamic lights ... add, radiusmax, dlight_color, vertexlight
		for (lnum = 0 ; lnum < MAX_DLIGHTS ; lnum++)
		{
			float		add;
			vec3_t		dist;

			if (cl_dlights[lnum].die < cl.time || !cl_dlights[lnum].radius)	continue;	// Light dead

			VectorSubtract (ent->origin, cl_dlights[lnum].origin, dist);
			add = cl_dlights[lnum].radius - VectorLength(dist);

			if (add > 0 && r_vertex_lights.integer && (!radiusmax || cl_dlights[lnum].radius > radiusmax))
			{	// Copy the dlight info for vertex lights
				radiusmax = cl_dlights[lnum].radius;
				VectorCopy (cl_dlights[lnum].origin, vertexlight);
			}

			if (add > 0 && scene_dynamiclight.integer)	// joe: only allow colorlight affection if dynamic lights are on
				AliasLight_Blend_DynamicLightColor_Into_Color (add, bubblecolor[cl_dlights[lnum].color_type], lightpoint_lightcolor);
			else if (add > 0)
				lerpdata.ambient_light += add;

		} // end for

		// calculate pitch and yaw for vertex lighting
		if (r_vertex_lights.integer)				VertexLights_CalcPitchYaw (ent->origin, ent->angles, vertexlight, radiusmax, &vlight_pitch, &vlight_yaw);

//		if (game_kurok.integer)
//		{
//			// clamp lighting so it doesn't overbright as much
//			if (ambientlight > 96)						ambientlight = 96;
//			if (ambientlight + shadelight > 128)		shadelight = 128 - ambientlight;
//		}
//		else
//		{
			// clamp lighting so it doesn't overbright as much
		if (lerpdata.ambient_light > 128)								lerpdata.ambient_light	= 128;
		if (lerpdata.ambient_light + lerpdata.shade_light > 192)		lerpdata.shade_light	= 192 - lerpdata.ambient_light;
//		}

		// always give the gun some light
		if (isViewModel && lerpdata.ambient_light < 24)					lerpdata.ambient_light	= lerpdata.shade_light = 24;

		// ZOID: never allow players to go totally black
		if (isPlayer && lerpdata.ambient_light < 8)						lerpdata.ambient_light	= lerpdata.shade_light = 8;
		if (isPlayer && r_lightmap.integer == 2)						lerpdata.full_light		= (qbool)(lerpdata.ambient_light = lerpdata.shade_light = 128);

	} // end !special lighting

//	shadedots = r_avertexnormal_dots[((int)(ent->angles[1] * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];

	{
		// locate the proper data
		qbool IsLumaTex(const int texturenum);
		qbool isluma = false;
		qbool do_mtex_fullbright = false;

		c_alias_polys += paliashdr->numtris;

		// draw all the triangles
		eglPushMatrix ();

		R_RotateForEntity (lerpdata.origin, lerpdata.angles); //ent->origin, ent->angles);

		R_MoreRotateForEntity_Well_Actually_Translate_And_Scale(paliashdr, clmodel->modhint, isViewModel, scene_fov_x.floater, r_viewmodel_size.floater);


		anim = (int)(cl.time*10) & 3;
		skinnum = ent->skinnum;
		if ((skinnum >= paliashdr->numskins) || (skinnum < 0))
		{
			Con_DevPrintf (DEV_MODEL, "R_DrawModel_Alias: no such skin # %d\n", skinnum);
			skinnum = 0;
		}

		texture = paliashdr->gl_texturenum[skinnum][anim];
		fb_texture = paliashdr->fb_texturenum[skinnum][anim];

		if (fb_texture)
			isluma = paliashdr->isluma[skinnum][anim];

		// we can't dynamically colormap textures, so they are cached
		// separately for the players. Heads are just uncolored.
		if (isPlayer && ent->colormap != vid.colormap && !cl_ent_nocolors.integer)
		{
			texture = playertextures - 1 + client_no;
				fb_texture = fb_skins[client_no-1];
		}

		if (lerpdata.full_light || (!r_pass_lumas.integer || !r_pass_lumas_alias.integer))
			fb_texture = 0;

		// Begin drawing!

		mglPushStates ();

		GL_DisableMultitexture ();

		if (gl_smoothmodels.integer)	MeglShadeModel (GL_SMOOTH);
		if (gl_affinemodels.integer)	eglHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

		if (entalpha < 1)
			MeglEnable (GL_BLEND);

//		if (strstr(clmodel->name, "shot"))
//			game_kurok.integer = game_kurok.integer;

		if (clmodel->modelflags & MOD_RENDERADDITIVE)
		{
			MeglEnable			(GL_BLEND);
			MeglBlendFunc		(GL_ONE, GL_ONE);
		}
		else if (clmodel->modelflags & (MOD_RENDERFILTER))
		{
//			MeglEnable			(GL_ALPHA_TEST);
			MeglEnable			(GL_BLEND);
			MeglBlendFunc		(GL_ZERO, GL_SRC_COLOR);
		}
		else if (clmodel->modelflags & (MOD_RENDERFENCE | MOD_FORCEDRENDERFENCE))
		{
			MeglEnable			(GL_ALPHA_TEST);
		}


		GL_Bind (texture);
		MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);  // Must be modulate for vertex lights to show


		isluma = fb_texture && isluma;
		do_mtex_fullbright = gl_mtexable && (!isluma || gl_add_ext);
		if (fb_texture && do_mtex_fullbright)
		{
			GL_EnableMultitexture ();
			GL_Bind (fb_texture);

			if (isluma)
				entalpha = entalpha;
			MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, isluma ? GL_ADD : GL_DECAL);
		}

		GL_DrawAliasFrameVerts (paliashdr, lerpdata, fb_texture && gl_mtexable , isViewModel, entalpha, maxDistance, false, NULL, 0, 0);

		if (fb_texture && !do_mtex_fullbright)
		{
			if (isluma)
			{
				MeglBlendFunc (GL_ONE, GL_ONE);
				MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			}
			else
			{
				MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				MeglEnable (GL_ALPHA_TEST);
			}
			GL_Bind (fb_texture);

			GL_DrawAliasFrameVerts (paliashdr, lerpdata, fb_texture && gl_mtexable , isViewModel, entalpha, maxDistance, false, NULL, 0, 0);

			if (isluma)
			{
				MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			}
			else
			{
				MeglDisable (GL_ALPHA_TEST);

			}
		}

		if (fb_texture && do_mtex_fullbright)	GL_DisableMultitexture ();

		MeglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);


//		if (clmodel->modelflags & (MOD_RENDERFENCE | MOD_FORCEDRENDERFENCE))
//			MeglDisable (GL_ALPHA_TEST);

		if (clmodel->modelflags & MOD_RENDERADDITIVE)
		{
			MeglDisable			(GL_BLEND);
			MeglBlendFunc		(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if (clmodel->modelflags & (MOD_RENDERFILTER))
		{
//			MeglDisable			(GL_ALPHA_TEST);
			MeglDisable			(GL_BLEND);
			MeglBlendFunc		(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if (clmodel->modelflags & (MOD_RENDERFENCE | MOD_FORCEDRENDERFENCE))
		{
			MeglDisable			(GL_ALPHA_TEST);
		}


		if (entalpha < 1)
			MeglDisable (GL_BLEND);

		if (gl_smoothmodels.integer)
			MeglShadeModel (GL_FLAT);
		if (gl_affinemodels.integer)
			eglHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		MeglColor3ubv (color_white);
		eglPopMatrix (); mglPopStates ();
	} // Model drawing phase end

	lerpdata.full_light =0;
	lerpdata.shade_light =0;
	lerpdata.ambient_light = 0;

}

// View model stuff
typedef struct interpolated_weapon
{
	char	name[MAX_QPATH];
	int	maxDistance;
} interp_weapon_t;

static	interp_weapon_t	interpolated_weapons[INTERP_WEAP_MAXNUM];
static	int		interp_weap_num = 0;




/*
=============
R_DrawViewModel
=============
*/
void R_DrawViewModel (void)
{
	// fenix@io.com: model transform interpolation
//	float		old_interpolate_transform;
	entity_t	*my_view_model = &cl.viewmodel_ent; // the_currententity

	if (!r_drawviewmodel.floater || chase_active.integer || !r_drawentities.integer
		|| (cl.stats[STAT_HEALTH] <= 0) || !my_view_model->model)
		return;

	// LordHavoc: if the player is transparent, so is his gun

	my_view_model->alpha = cl_entities[cl.player_point_of_view_entity].alpha;
	// hack the depth range to prevent view model from poking into walls
	MeglDepthRange (0, 0.3);

	R_DrawModel_Alias (my_view_model, false);

	MeglDepthRange (0, 1);
}


// joe: from FuhQuake, but this is less configurable
void Set_Interpolated_Weapon_f (void)
{
	int	i;
	char	str[MAX_QPATH];

	if (cmd_source != src_command)
		return;

	if (Cmd_Argc() == 2)
	{
		for (i=0 ; i<interp_weap_num ; i++)
			if (COM_StringMatchCaseless (Cmd_Argv(1), interpolated_weapons[i].name))
			{
				Con_Printf ("%s`s distance is %d\n", Cmd_Argv(1), interpolated_weapons[i].maxDistance);
				return;
			}
		Con_Printf ("%s`s distance is default (%d)\n", Cmd_Argv(1), r_vertex_blenddist.integer);
		return;
	}

	if (Cmd_Argc() != 3)
	{
		Con_Printf ("Usage: %s <model> [distance]\n", Cmd_Argv (0));
		return;
	}

	strcpy (str, Cmd_Argv(1));
	for (i=0 ; i<interp_weap_num ; i++)
		if (COM_StringMatchCaseless (str, interpolated_weapons[i].name))
			break;
	if (i == interp_weap_num)
	{
		if (interp_weap_num == 24)
		{
			Con_Printf ("interp_weap_num == INTERP_WEAP_MAXNUM\n");
			return;
		}
		else
		{
			interp_weap_num++;
		}
	}

	strcpy (interpolated_weapons[i].name, str);
	interpolated_weapons[i].maxDistance = (int)atof(Cmd_Argv(2));
}

static int DoWeaponInterpolation (entity_t *this_entity)
{
	int	i;

	if (this_entity != &cl.viewmodel_ent)
		return -1;

	for (i=0 ; i<interp_weap_num ; i++)
	{
		if (!interpolated_weapons[i].name[0])
			return -1;

		if (COM_StringMatchCaseless (this_entity->model->name, va("%s.mdl", interpolated_weapons[i].name)) ||
		    COM_StringMatchCaseless (this_entity->model->name, va("progs/%s.mdl", interpolated_weapons[i].name)))
			return interpolated_weapons[i].maxDistance;
	}

	return -1;
}
