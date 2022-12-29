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
// model_alias.c -- alias model loading and caching

// models are the only shared resource between a client and server running
// on the same machine.

#include "quakedef.h"

/*
==============================================================================

				ALIAS MODELS

==============================================================================
*/

static aliashdr_t	*pheader;

// model_alias_mesh.c needs access to these
stvert_t			stverts[MAXALIASVERTS];
mtriangle_t			triangles[MAXALIASTRIS];
trivertx_t			*poseverts[MAXALIASFRAMES];

static int			posenum;		// a pose is a single set of vertexes. a frame may be an animating sequence of poses
static byte			aliasbboxmins[3], aliasbboxmaxs[3];

static char			modelname_nopath_with_extension[64];

/*
=================
Mod_LoadAliasFrame
=================
*/
static void *Mod_LoadFrame_Alias (void *pin, maliasframedesc_t *frame)
{
	daliasframe_t	*pdaliasframe = (daliasframe_t *)pin;
	int				i;

	strcpy (frame->name, pdaliasframe->name);
	frame->firstpose = posenum;
	frame->numposes = 1;

	for (i=0 ; i<3 ; i++)
	{
	// these are byte values, so we don't have to worry about endianness
		frame->bboxmin.v[i] = pdaliasframe->bboxmin.v[i];
		frame->bboxmax.v[i] = pdaliasframe->bboxmax.v[i];

		aliasbboxmins[i] = min(aliasbboxmins[i], frame->bboxmin.v[i]);
		aliasbboxmaxs[i] = max(aliasbboxmaxs[i], frame->bboxmax.v[i]);
	}

	{
		trivertx_t	*pinframe = (trivertx_t *)(pdaliasframe + 1);
		poseverts[posenum] = pinframe;
		posenum++;

		pinframe += pheader->numvertsperframe;
		return (void *)pinframe;
	}


}

/*
=================
Mod_LoadAliasGroup
=================
*/
static void *Mod_LoadFrameGroup_Alias (void *pin,  maliasframedesc_t *frame)
{
	void				*ptemp;

	daliasgroup_t		*pingroup		= (daliasgroup_t *)pin;
	int					numframes		= LittleLong (pingroup->numframes);
	int					i;

	frame->firstpose = posenum;
	frame->numposes = numframes;

	for (i=0 ; i<3 ; i++)
	{
	// these are byte values, so we don't have to worry about endianness
		frame->bboxmin.v[i] = pingroup->bboxmin.v[i];
		frame->bboxmax.v[i] = pingroup->bboxmax.v[i];

		aliasbboxmins[i] = min(aliasbboxmins[i], frame->bboxmin.v[i]);
		aliasbboxmaxs[i] = max(aliasbboxmaxs[i], frame->bboxmax.v[i]);
	}

	{
		daliasinterval_t	*pin_intervals;

		pin_intervals = (daliasinterval_t *)(pingroup + 1);
		frame->interval = LittleFloat (pin_intervals->interval);
		pin_intervals += numframes;
		ptemp = (void *)pin_intervals;
	}

	for (i=0 ; i<numframes ; i++, posenum++)
	{
		poseverts[posenum] = (trivertx_t *)((daliasframe_t *)ptemp + 1);
		ptemp = (trivertx_t *)((daliasframe_t *)ptemp + 1) + pheader->numvertsperframe;
	}

	return ptemp;
}


static void sMod_LoadAllSkins_ProcessSkin (const char *identifier, byte *native_data, const int skinwidth, const int skinheight, int *gl_texnum, int *fb_texnum, qbool *isluma)
{
	int picmip_flag = tex_picmip_allmodels.integer ? TEX_MIPMAP : 0;

	char loadpath[64];
	snprintf (loadpath, sizeof(loadpath), "progs/%s", identifier);	// Baker: no more models folder; use progs folder (but that isn't quite right, should use the model's folder.  Period.)

	*isluma = *gl_texnum = *fb_texnum = 0;

	ImageWork_Start ("Alias skin", identifier);

	// External textures phase
	do
	{
		if (game_kurok.integer) 	// Kurok Game: No external textures
			break;					// Baker: Don't want the hassle of alpha channel external regular textures yet
									//        Let alone rewriting that stuff to support fullbright

		if (!(*gl_texnum = GL_LoadExternalTextureImage (loadpath, identifier, picmip_flag, loadmodel->loadinfo.searchpath /*PATH LIMITED*/)))
			break;	// No external texture do next phase

		if ((*fb_texnum = GL_LoadExternalTextureImage (va("%s_glow", loadpath), va("@fb_%s", identifier), picmip_flag | TEX_LUMA, loadmodel->loadinfo.searchpath /*PATH LIMITED*/)))
			*isluma = true;

		// External texture(s) found ... done!
		goto skin_done;
	} while (0);

	// Alpha texture phase; Kurok only for now

	do
	{
		// Only check these model types for alpha mask.
		// Otherwise it is pointless for rendering.

		qbool qualifying_flag = loadmodel->modelflags & (MOD_RENDERADDITIVE | MOD_RENDERFILTER | MOD_RENDERFENCE);

		// Baker: Remember if we allow non-Kurok we'll have to allow fullbrights somehow?
		//        And that's gonna mean mtex for anything with alpha blending

		if (!game_kurok.integer && !qualifying_flag)
			break;					// Only check if Kurok or a special model flag

		if (!Texture_QPAL_HasMaskColor255 (native_data, skinwidth * skinheight))
			break;					// No mask so we can't have alpha mask

		//
		// At this point we have a texture with alpha
		//

		*gl_texnum = GL_LoadTexture (va("@mask_%s", identifier), skinwidth, skinheight, native_data, picmip_flag | TEX_ALPHA_TEST, QPAL_BYTES_PER_PIXEL_IS_1);


		if (game_kurok.integer && !qualifying_flag)	// Kurok, force alpha if alpha is found, even if not a special model flag
			loadmodel->modelflags |= MOD_FORCEDRENDERFENCE;

		if (game_kurok.integer)
			goto skin_done;		// Kurok gets 1 texture, we got it and we're done

		goto skin_done;			// I guess we aren't supporting fullbrights for alpha textures right now ...
	} while (0);


	// Normal Quake texture phase
	do
	{
		*gl_texnum = GL_LoadTexture (identifier, skinwidth, skinheight, native_data, picmip_flag, QPAL_BYTES_PER_PIXEL_IS_1);

	} while (0);

	// Fullbright texture phase

 	do
	{

		if (game_kurok.integer)
			break; // We won't be doing fullbrights

		if (!Texture_QPAL_HasFullbrights(native_data, skinwidth * skinheight))
			break;	// No fullbright pixels so phase is over

		*fb_texnum = GL_LoadTexture (va("@fb_%s", identifier), skinwidth, skinheight, native_data, picmip_flag | TEX_FULLBRIGHT, QPAL_BYTES_PER_PIXEL_IS_1);
	} while (0);


skin_done:

	ImageWork_Finish ();
}


/*
===============
Mod_LoadAllSkins
===============
*/
static void *Mod_LoadAllSkins (int numskins, daliasskintype_t *pskintype)
{
	int						skinsize = pheader->skinwidth * pheader->skinheight;
	byte					*trashable_skin = (byte *)(pskintype + 1);
	int						i_skin;

	if (numskins < 1 || numskins > MAX_SKINS)		Host_Error ("Mod_LoadAllSkins: Invalid # of skins: %d\n", numskins);

	for (i_skin =0 ; i_skin<numskins ; i_skin++)
	{
		if (pskintype->type == ALIAS_SKIN_SINGLE)
		{
			do	// save 8 bit texels for the player model to remap
			{
				byte *native_texels;

				if (isDedicated)	break;

				Mod_FloodFillSkin (trashable_skin, pheader->skinwidth, pheader->skinheight);

				{	// Save pixels for R_Translateskin stuffs
					native_texels = Hunk_AllocName (1, skinsize, loadname);
					pheader->texels[i_skin] = native_texels - (byte *)pheader;
					memcpy (native_texels,  (byte *)(pskintype + 1), skinsize);	// skin = (byte *)(pskintype + 1)
				}

				{ // OpenGL Upload
					char 	identifier[64];
					int  	gl_texnum, fb_texnum;
					qbool	isluma;

					snprintf (identifier, sizeof(identifier), "%s_%i", modelname_nopath_with_extension, i_skin);

					sMod_LoadAllSkins_ProcessSkin
					(
						identifier,
						 (byte *)(pskintype + 1),
						pheader->skinwidth,
						pheader->skinheight,
						&gl_texnum,
						&fb_texnum,
						&isluma
					);

					// Tell the header the data
					pheader->gl_texturenum[i_skin][0] 	= pheader->gl_texturenum[i_skin][1] = pheader->gl_texturenum[i_skin][2] = pheader->gl_texturenum[i_skin][3] = gl_texnum;
					pheader->fb_texturenum[i_skin][0] 	= pheader->fb_texturenum[i_skin][1] = pheader->fb_texturenum[i_skin][2] = pheader->fb_texturenum[i_skin][3] = fb_texnum;
					pheader->isluma[i_skin][0] 			= pheader->isluma[i_skin][1] = pheader->isluma[i_skin][2] = pheader->isluma[i_skin][3] = isluma;

				}
			} while (0);

			// Advance the pointer to the verts segment
			pskintype = (daliasskintype_t *)((byte *)(pskintype+1) + skinsize);	// skin = (byte *)(pskintype + 1)
		}
		else
		{
			// animating skin group. yuck.
			int						j_skingroup, k;

			daliasskingroup_t		*pinskingroup 		= (daliasskingroup_t *)++pskintype;	// Killed line: pskintype++;
 			daliasskininterval_t	*pinskinintervals 	= (daliasskininterval_t *)(pinskingroup + 1);
			int						groupskins 			= LittleLong (pinskingroup->numskins);

			// Place our pointer
			pskintype = (void *)(pinskinintervals + groupskins);

			for (j_skingroup=0 ; j_skingroup<groupskins ; j_skingroup++)
			{
				do
				{
					byte *native_texels;

					if (isDedicated)	break;

					Mod_FloodFillSkin (trashable_skin, pheader->skinwidth, pheader->skinheight);

					if (j_skingroup ==0) // Only colormap first group skin :(  Still, those are silly.
					{
						native_texels = Hunk_AllocName (1, skinsize, loadname);
						pheader->texels[i_skin] = native_texels - (byte *)pheader;
						memcpy (native_texels, (byte *)(pskintype), skinsize);	// skin = (byte *)(pskintype + 1)
					}

					{
						char 	identifier[64];
						int  	gl_texnum, fb_texnum;
						qbool	isluma;

						snprintf (identifier, sizeof(identifier), "%s_%i_%i", modelname_nopath_with_extension, i_skin, j_skingroup);

						sMod_LoadAllSkins_ProcessSkin
						(
							identifier,
							(byte *)(pskintype),					// Baker: No native texels except skin 1 :(
							pheader->skinwidth,
							pheader->skinheight,
							&gl_texnum,
							&fb_texnum,
							&isluma
						);

						pheader->gl_texturenum[i_skin][j_skingroup & 3] = gl_texnum;
						pheader->fb_texturenum[i_skin][j_skingroup & 3] = fb_texnum;
						pheader->isluma[i_skin][j_skingroup & 3] = isluma;
					}
				} while (0);

				pskintype = (daliasskintype_t *)((byte *)(pskintype) + skinsize);
			}
			for (k = j_skingroup; j_skingroup < 4 ; j_skingroup++)
				pheader->gl_texturenum[i_skin][j_skingroup & 3] = pheader->gl_texturenum[i_skin][j_skingroup - k];
		}
	}

	return (void *)pskintype;
}




/*
=================
Mod_LoadModel_Alias
=================
*/
void Mod_LoadModel_Alias (model_t *mod, void *buffer)
{
	modhint_t GameHacks_IsSpecialQuakeAliasModel (const char *model_name);	// gamehack
	void Mod_SetExtraFlags (model_t *mod);									// Another gamehack

	mdl_t	*pinmodel			= (mdl_t *)buffer;
	int 	version				= LittleLong (pinmodel->version);
	int		i;
	int		hunk_start;

	if (version != ALIAS_VERSION)	Host_Error ("Mod_LoadModel_Alias: %s has wrong version number (%i should be %i)", mod->name, version, ALIAS_VERSION);

// Baker: Get to work on defining some stuff
	mod->modelformat			= mod_alias;
	mod->modhint				= GameHacks_IsSpecialQuakeAliasModel (mod->name);
	mod->modelflags				= LittleLong (pinmodel->flags);

	Mod_SetExtraFlags 					(mod); //johnfitz
	StringLCopy (modelname_nopath_with_extension, StringTemp_SkipPath(loadmodel->name));

// allocate space for a working header, plus all the data except the frames, skin and group info
	{
		int size				= sizeof(aliashdr_t) + (LittleLong(pinmodel->numframes) - 1) * sizeof(pheader->frames[0]);
		hunk_start				= Hunk_LowMark ();
		pheader					= Hunk_AllocName (1, size, loadname);
	}

// endian-adjust and copy the data, starting with the alias model header
	pheader->boundingradius		= LittleFloat (pinmodel->boundingradius);
	pheader->numskins			= LittleLong (pinmodel->numskins);
	pheader->skinwidth			= LittleLong (pinmodel->skinwidth);
	pheader->skinheight			= LittleLong (pinmodel->skinheight);
	pheader->numvertsperframe	= LittleLong (pinmodel->numverts);
	pheader->numtris			= LittleLong (pinmodel->numtris);
	pheader->numframes			= LittleLong (pinmodel->numframes);

	// validate the setup
	if (pheader->numframes < 1) 						Host_Error ("Mod_LoadModel_Alias: Model %s has invalid # of frames: %d", mod->name, pheader->numframes);
	if (pheader->skinheight > MAX_LBM_HEIGHT)			Host_Error ("Mod_LoadModel_Alias: Model %s has a skin taller than %d", mod->name, MAX_LBM_HEIGHT);
	if (pheader->numvertsperframe <= 0)					Host_Error ("Mod_LoadModel_Alias: Model %s has no vertices", mod->name);
	if (pheader->numvertsperframe > MAXALIASVERTS)		Host_Error ("Mod_LoadModel_Alias: Model %s has too many vertices (%d, max = %d)", mod->name, pheader->numvertsperframe, MAXALIASVERTS);
	if (pheader->numtris <= 0)							Host_Error ("Mod_LoadModel_Alias: Model %s has no triangles", mod->name);
	if (pheader->numtris > MAXALIASTRIS)				Host_Error ("Mod_LoadModel_Alias: Model %s has too many triangles (%d, max = %d)", mod->name, pheader->numtris, MAXALIASTRIS);

	pheader->size				= LittleFloat (pinmodel->size) * ALIAS_BASE_SIZE_RATIO;
	mod->synctype				= LittleLong (pinmodel->synctype);
	mod->numframes				= pheader->numframes;

// origins and scale
	for (i=0 ; i<3 ; i++)
	{
		pheader->scale[i]		= LittleFloat (pinmodel->scale[i]);
		pheader->scale_origin[i]= LittleFloat (pinmodel->scale_origin[i]);
		pheader->eyeposition[i] = LittleFloat (pinmodel->eyeposition[i]);
	}

	// Drill down and load
	{	// load the skins
		daliasskintype_t *pskintype = (daliasskintype_t *)&pinmodel[1];
		pskintype = Mod_LoadAllSkins (pheader->numskins, pskintype);

		{	// load base s and t vertices
			stvert_t		*pinstverts = (stvert_t *)pskintype;
			for (i=0 ; i<pheader->numvertsperframe ; i++)
			{
				stverts[i].onseam = LittleLong (pinstverts[i].onseam);
				stverts[i].s = LittleLong (pinstverts[i].s);
				stverts[i].t = LittleLong (pinstverts[i].t);
			}

			{	// load triangle lists
				dtriangle_t			*pintriangles = (dtriangle_t *)&pinstverts[pheader->numvertsperframe];
				for (i=0 ; i<pheader->numtris ; i++)
				{
					int					j;
					triangles[i].facesfront = LittleLong (pintriangles[i].facesfront);


					for (j=0 ; j<3 ; j++)
						triangles[i].vertindex[j] = LittleLong (pintriangles[i].vertindex[j]);
				}

				{ // load the frames
					daliasframetype_t	*pframetype = (daliasframetype_t *)&pintriangles[pheader->numtris];
					posenum = 0;

					aliasbboxmins[0] = aliasbboxmins[1] = aliasbboxmins[2] = 255;
					aliasbboxmaxs[0] = aliasbboxmaxs[1] = aliasbboxmaxs[2] = 0;

					for (i=0 ; i<pheader->numframes ; i++)
					{
						aliasframetype_t	frametype = LittleLong (pframetype->type);

						if (frametype == ALIAS_SINGLE)
							pframetype = (daliasframetype_t *)Mod_LoadFrame_Alias (pframetype + 1, &pheader->frames[i]);
						else
							pframetype = (daliasframetype_t *)Mod_LoadFrameGroup_Alias (pframetype + 1, &pheader->frames[i]);

					}
				} // End of frames
			} // End of triangles
		} // End of st verts
	} //End of skins
	pheader->numposes = posenum;

	// bboxes fun

	for (i=0 ; i<3 ; i++)
	{
		mod->mins[i] = aliasbboxmins[i] * pheader->scale[i] + pheader->scale_origin[i];
		mod->maxs[i] = aliasbboxmaxs[i] * pheader->scale[i] + pheader->scale_origin[i];
	}

	mod->radius = RadiusFromBounds (mod->mins, mod->maxs);

// build the draw lists
	GL_MakeAliasModelDisplayLists (pheader);

// move the complete, relocatable alias model to the cache
	{
		int end 	= Hunk_LowMark ();
		int total 	= end - hunk_start;

		Cache_Alloc (&mod->cache, total, loadname);

		if (!mod->cache.data)
			return;

		memcpy (mod->cache.data, pheader, total);
	}

	Hunk_FreeToLowMark (hunk_start);
}


