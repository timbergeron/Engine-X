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
// gl_rmain.c
// Baker: Validated 6-27-2011.  Just actual interpolation fix.



#include "quakedef.h"

int		c_brush_polys, c_alias_polys, c_md3_polys;
void R_DrawEntitiesOnList (const qbool alphapass);

void Scene_Render (void)
{
// Baker: No drawing has occurred before here
// And if it did, it shouldn't

	c_brush_polys = 0;
	c_alias_polys = 0;
	c_md3_polys = 0;

	R_Clear ();

	R_SetupGL ();

//	Fog_Waterfog_SetupFrame (r_viewleaf->contents);

//	if (nehahra || gl_fogenable.integer)
	Fog_SetupFrame ();

	/*
	=============
	R_DrawWorld
	=============
	*/
	{
		Fog_DisableGFog ();
		MeglDisable(GL_FOG);

		if (r_skyboxloaded && !r_oldsky.integer)
			Sky_SkyBox_Draw ();
		else
			Sky_OldSky_DrawChain ();

		// Mr. Fitz draws skychains from brush models here, believe it or not

		Fog_EnableGFog ();

		// draw the world
		R_DrawWorld_Solid_TextureChains (cl.worldmodel, 0, 1, 0);

		R_DrawGlassyChain ();
		R_DrawFenceChain ();
	}

	S_ExtraUpdate ();	// don't let sound get messed up if going slow

	R_DrawEntitiesOnList (false);	// Solid pass ... Alpha chain is part of rendering.  I wonder about the entity sorting.  Submodels can't have water/lava surfaces, but can have sky
	R_DrawEntitiesOnList (true);	// Alpha pass ... Alpha chain is part of rendering.  I wonder about the entity sorting.  Submodels can't have water/lava surfaces, but can have sky

	R_DrawWaterSurfaces ();

	GL_DisableMultitexture ();

	Light_Render_FlashBlend_Dlights ();		// Definitely part of scene (Flashblend Dlights)

	Particles_DrawParticles ();		// Definitely part of scene

	

	Fog_DisableGFog ();
}