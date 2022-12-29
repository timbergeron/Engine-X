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
// model_sprite.c -- sprite model loading and caching


// models are the only shared resource between a client and server running
// on the same machine.

#include "quakedef.h"

//=============================================================================

//static	int	spr_version;

/*
=================
Mod_LoadSpriteModelTexture
=================
*/
static void Mod_LoadFrame_Sprite_LoadModelTexture (char *sprite_name, dspriteframe_t *pinframe, mspriteframe_t *pspriteframe, const int framenum, const int width, int height, int texture_flag)
{
	char	texmgr_name_for_sprite_frame[64], /*sprite[64],*/ sprite_external_texture[64];

//	COM_Copy_StripExtension (sprite_name, sprite);

	snprintf (texmgr_name_for_sprite_frame, sizeof(texmgr_name_for_sprite_frame), "%s_%i",		sprite_name, framenum); // frame number!
	snprintf (sprite_external_texture,		sizeof(sprite_external_texture)		, "progs/%s_%i", sprite_name, framenum);  // s_bubble.spr_0.tga for example

	ImageWork_Start ("spritetex", sprite_name);

	pspriteframe->gl_texturenum = GL_LoadExternalTextureImage (sprite_external_texture, texmgr_name_for_sprite_frame, texture_flag, loadmodel->loadinfo.searchpath /*PATH LIMIT ME*/);

	// No external texture, use sprite model texture
	if (pspriteframe->gl_texturenum == 0)
	{
		if (loadmodel->modelformat == mod_spr32)
			pspriteframe->gl_texturenum = GL_LoadTexture (texmgr_name_for_sprite_frame, width, height, (byte *)(pinframe + 1), texture_flag, RGBA_BYTES_PER_PIXEL_IS_4);
		else
			pspriteframe->gl_texturenum = GL_LoadTexture (texmgr_name_for_sprite_frame, width, height, (byte *)(pinframe + 1), texture_flag, QPAL_BYTES_PER_PIXEL_IS_1);
	}

	ImageWork_Finish ();
}

/*
=================
Mod_LoadSpriteFrame
=================
*/
void *Mod_LoadFrame_Sprite (void *pin, mspriteframe_t **ppframe, const int framenum)
{
	int					texture_flag	= (tex_picmip_allmodels.integer ? TEX_MIPMAP : 0) | TEX_ALPHA_TEST;
	dspriteframe_t		*pinframe		= (dspriteframe_t *)pin;
	const int			width			=  LittleLong (pinframe->width);
	const int			height			=  LittleLong (pinframe->height);
	const int			size			= (loadmodel->modelformat == mod_spr32) ? width * height * 4 : width * height;
	mspriteframe_t		*pspriteframe	=  Hunk_AllocName (1, sizeof(mspriteframe_t), loadname);
	int					origin[2];

//	memset (pspriteframe, 0, sizeof(mspriteframe_t));	// This is redundant, but whatever ...  Hunk_AllocName does a memset to 0

	*ppframe							= pspriteframe;

	pspriteframe->width					= width;
	pspriteframe->height				= height;
	origin[0]							= LittleLong (pinframe->origin[0]);
	origin[1]							= LittleLong (pinframe->origin[1]);

	pspriteframe->up					= origin[1];
	pspriteframe->down					= origin[1] - height;
	pspriteframe->left					= origin[0];
	pspriteframe->right					= width + origin[0];

	if (!isDedicated)
		Mod_LoadFrame_Sprite_LoadModelTexture (loadmodel->name, pinframe, pspriteframe, framenum, width, height, texture_flag);

	return (void *)((byte *)pinframe + sizeof(dspriteframe_t) + size);
}


/*
=================
Mod_LoadSpriteGroup
=================
*/
static void *Mod_LoadGroup_Sprite (void *pin, mspriteframe_t **ppframe, int framenum)
{
	dspritegroup_t		*pingroup		= (dspritegroup_t *)pin;
	int					numframes		= LittleLong (pingroup->numframes);
	mspritegroup_t		*pspritegroup	= Hunk_AllocName (1, sizeof(mspritegroup_t) + (numframes - 1) * sizeof(pspritegroup->frames[0]), loadname);
	dspriteinterval_t	*pin_intervals	= (dspriteinterval_t *)(pingroup + 1);
	float				*poutintervals	= Hunk_AllocName (numframes, sizeof(float), loadname);

//	pingroup = (dspritegroup_t *)pin;
//	numframes = LittleLong (pingroup->numframes);
//	pspritegroup = Hunk_AllocName (sizeof(mspritegroup_t) + (numframes - 1) * sizeof(pspritegroup->frames[0]), loadname);

	pspritegroup->numframes = numframes;
	*ppframe = (mspriteframe_t *)pspritegroup;

//	pin_intervals = (dspriteinterval_t *)(pingroup + 1);
//	poutintervals = Hunk_AllocName (numframes. sizeof(float), loadname);

	pspritegroup->intervals = poutintervals;

	{
		int	i;
		for (i=0 ; i<numframes ; i++)
		{
			*poutintervals = LittleFloat (pin_intervals->interval);
			if (*poutintervals <= 0.0)
				Host_Error ("Mod_LoadSpriteGroup: interval %f <= 0 in %s", *poutintervals, loadmodel->name);

			poutintervals++;
			pin_intervals++;
		}
	

		{
			void	*ptemp = (void *)pin_intervals;

		//	ptemp = (void *)pin_intervals;

			for (i=0 ; i<numframes ; i++)
				ptemp = Mod_LoadFrame_Sprite (ptemp, &pspritegroup->frames[i], framenum * 100 + i);

			return ptemp;
		}

	}
	// Function end
}


/*
=================
Mod_LoadSpriteModel
=================
*/
void Mod_LoadModel_Sprite (model_t *mod, void *buffer)
{
	dsprite_t			*pin		= (dsprite_t *)buffer;
	int					spr_version = LittleLong (pin->version);

	if (spr_version != SPRITE_VERSION && spr_version != SPRITE32_VERSION)
		Host_Error ("Mod_LoadSpriteModel: %s has wrong version number (%i should be %i or %i)", mod->name, spr_version, SPRITE_VERSION, SPRITE32_VERSION);

	mod->modelformat = (spr_version == SPRITE32_VERSION) ? mod_spr32 : mod_sprite;
	
	do
	{
		
		int					numframes	= LittleLong (pin->numframes);
		int					size		= sizeof(msprite_t) + (numframes - 1) * sizeof(mspriteframedesc_t); // sizeof(psprite->frames);
		msprite_t			*psprite	= Hunk_AllocName (1, size, loadname);	// Baker: can't and shouldn't try to determine "frame cost"
		
		mod->cache.data = psprite;

		psprite->type					= LittleLong  (pin->type);
		psprite->maxwidth				= LittleLong  (pin->width);
		psprite->maxheight				= LittleLong  (pin->height);
		psprite->beamlength				= LittleFloat (pin->beamlength);

		if (game_kurok.integer && psprite->beamlength == 10)	loadmodel->modelflags |= MOD_RENDERADDITIVE;	//	additive = true;
		if (game_kurok.integer && psprite->beamlength == 20)	loadmodel->modelflags |= MOD_RENDERFILTER;		//	filter = true;
		//	Con_Printf ("Sprite model flags are %s = %i\n", loadmodel->name, loadmodel->modelflags);

		mod->synctype					= LittleLong (pin->synctype);
		psprite->numframes				= numframes;

		mod->mins[0] = mod->mins[1]		= -psprite->maxwidth /2;
		mod->maxs[0] = mod->maxs[1]		=  psprite->maxwidth /2;
		mod->mins[2]					= -psprite->maxheight/2;
		mod->maxs[2]					=  psprite->maxheight/2;

	// load the frames
		if (numframes < 1)	Host_Error ("Mod_LoadSpriteModel: Invalid # of frames %d in %s", numframes, mod->name);

		mod->numframes = numframes;

		{
			int					i; 
			dspriteframetype_t	*pframetype = (dspriteframetype_t *)(pin + 1);

			for (i=0 ; i<numframes ; i++)
			{
				spriteframetype_t	frametype;

				frametype				= LittleLong (pframetype->type);
				psprite->frames[i].type = frametype;

				if (frametype == SPR_SINGLE)
					pframetype			= (dspriteframetype_t *)Mod_LoadFrame_Sprite (pframetype + 1, &psprite->frames[i].frameptr, i);  // Frame
				else
					pframetype			= (dspriteframetype_t *)Mod_LoadGroup_Sprite (pframetype + 1, &psprite->frames[i].frameptr, i);	//  Group

			}
		}

	} while (0);

}

