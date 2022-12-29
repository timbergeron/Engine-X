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

				SPRITE MODELS

===============================================================================
*/

/*
================
R_GetSpriteFrame
================
*/
static mspriteframe_t *R_DrawModel_Sprite_GetFrame (const model_t *clmodel, const int myframe, const float mysyncbase)
{
	int				i, numframes, frame;
	float			*pintervals, fullinterval, targettime, time;
	msprite_t		*psprite;
	mspritegroup_t	*pspritegroup;
	mspriteframe_t	*pspriteframe;

	psprite = clmodel->cache.data;
	frame = myframe;

	if (frame >= psprite->numframes || frame < 0)
	{
		Con_Printf ("R_DrawSprite: no such frame %d\n", frame);
		frame = 0;
	}

	if (psprite->frames[frame].type == SPR_SINGLE)
	{
		pspriteframe = psprite->frames[frame].frameptr;
	}
	else
	{
		pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		pintervals = pspritegroup->intervals;
		numframes = pspritegroup->numframes;
		fullinterval = pintervals[numframes-1];

		time = cl.time + mysyncbase; // currententity->syncbase;

	// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
	// are positive, so we don't have to worry about division by 0
		targettime = time - ((int)(time / fullinterval)) * fullinterval;

		for (i=0 ; i<(numframes-1) ; i++)
		{
			if (pintervals[i] > targettime)
				break;
		}

		pspriteframe = pspritegroup->frames[i];
	}

	return pspriteframe;
}

/*
=================
R_DrawSpriteModel
=================
*/
void R_DrawModel_Sprite (const model_t *mymodel, const vec3_t location, const vec3_t myangles, const int myframe, const float mysyncbase)
{
	vec3_t			point;
	mspriteframe_t	*frame;
	vec3_t			right, up;
	msprite_t		*psprite;

	if (!r_drawmodels_sprite.integer)	return;

	// don't even bother culling, because it's just a single
	// polygon without a surface cache
	frame = R_DrawModel_Sprite_GetFrame (mymodel, myframe, mysyncbase);
	psprite = mymodel->cache.data;

	if (psprite->type == SPR_ORIENTED)
	{
		// bullet marks on walls
		AngleVectors (myangles, NULL, right, up);
	}
	else if (psprite->type == SPR_FACING_UPRIGHT)
	{
		VectorSet (up, 0, 0, 1);
		right[0] = location[1] - r_origin[1];
		right[1] = -(location[0] - r_origin[0]);
		right[2] = 0;
		VectorNormalize (right);
	}
	else if (psprite->type == SPR_VP_PARALLEL_UPRIGHT)
	{
		VectorSet (up, 0, 0, 1);
		VectorCopy (vright, right);
	}
	else
	{
		// normal sprite
		VectorCopy (vup, up);
		VectorCopy (vright, right);
	}


	mglPushStates ();
	
	GL_DisableMultitexture ();
	GL_Bind (frame->gl_texturenum);


	MeglDepthMask		(GL_FALSE);		// Baker: Cautiously enabling depth mask for all sprites.  Normal sprites don't normally get this.
										//        The reason I did this was additive sprites with Kurok and crossing the waterline looking terrible
										//        This shouldn't be necessary for regular sprites since they don't blend
										//        But it is more consistent this way and should have no effect I can think of.
										//        Unless it involves something drawn later.  We'll see...

	if (game_kurok.integer) Fog_DisableGFog ();
	
	if (mymodel->modelformat == mod_spr32)
		MeglEnable			(GL_BLEND);
	else
		MeglEnable			(GL_ALPHA_TEST);

	if (mymodel->modelflags & MOD_RENDERADDITIVE)
	{
		MeglEnable			(GL_BLEND);				// Technically we should turn off the alpha test
		MeglBlendFunc		(GL_ONE, GL_ONE);
	}
	else if (mymodel->modelflags & MOD_RENDERFILTER)
	{
		MeglEnable			(GL_BLEND);				// Technically we should turn off the alpha test
		MeglBlendFunc(GL_ZERO, GL_SRC_COLOR);
	}


	mglFinishedStates ();

	eglBegin (GL_QUADS);

	eglTexCoord2f (0, 1);

	VectorMultiplyAdd (location, frame->down, up, point);
	VectorMultiplyAdd (point, frame->left, right, point);
	eglVertex3fv (point);

	eglTexCoord2f (0, 0);
	VectorMultiplyAdd (location, frame->up, up, point);
	VectorMultiplyAdd (point, frame->left, right, point);
	eglVertex3fv (point);

	eglTexCoord2f (1, 0);
	VectorMultiplyAdd (location, frame->up, up, point);
	VectorMultiplyAdd (point, frame->right, right, point);
	eglVertex3fv (point);

	eglTexCoord2f (1, 1);
	VectorMultiplyAdd (location, frame->down, up, point);
	VectorMultiplyAdd (point, frame->right, right, point);
	eglVertex3fv (point);

	eglEnd ();

	MeglDepthMask		(GL_TRUE);

	if (mymodel->modelformat == mod_spr32)
		MeglDisable			(GL_BLEND);
	else
		MeglDisable			(GL_ALPHA_TEST);

	if (mymodel->modelflags & MOD_RENDERADDITIVE)
	{
		MeglDisable			(GL_BLEND);
		MeglBlendFunc	(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else if (mymodel->modelflags & MOD_RENDERFILTER)
	{
		MeglDisable			(GL_BLEND);
		MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	}

	if (game_kurok.integer) Fog_EnableGFog ();

	mglPopStates ();
}
