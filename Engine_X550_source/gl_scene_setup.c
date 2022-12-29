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
//entity_t	r_worldentity;


//vec3_t		modelorg;
//static		vec3_t	r_entorigin;
//entity_t	*the_currententity;

int		r_visframecount;	// bumped when going to a new PVS
int		r_framecount;		// used for dlight push checking

mplane_t	frustum[4];




// view origin
vec3_t	vup;
vec3_t	vpn;
vec3_t	vright;
vec3_t	r_origin;

float	r_world_matrix[16];
float	r_base_world_matrix[16];

// screen size info
refdef_t	r_refdef;

mleaf_t		*r_viewleaf, *r_oldviewleaf;
mleaf_t		*r_viewleaf2, *r_oldviewleaf2;	// for watervis hack

float r_fovx, r_fovy; //johnfitz -- rendering fov may be different becuase of r_waterwarp and r_stereo

int		d_lightstylevalue[256];	// 8.8 fraction of base light value (Move to light?)

void CL_RunParticles (void);
void R_Build_Chains_World (void);
void R_Build_Chains_World_SubModels (void);
void Viewblends_FadeDamageBonus_Buckets (void);


static void Scene_CalculateViewLeafs (void)
{
	vec3_t			testorigin;
	mleaf_t			*leaf;
	
	// current viewleaf
	r_oldviewleaf = r_viewleaf;
	r_oldviewleaf2 = r_viewleaf2;

	r_viewleaf = Mod_PointInLeaf (r_origin, cl.worldmodel);
	r_viewleaf2 = NULL;

	// check above and below so crossing solid water doesn't draw wrong
	if (r_viewleaf->contents <= CONTENTS_WATER && r_viewleaf->contents >= CONTENTS_LAVA)
	{
		// look up a bit
		VectorCopy (r_origin, testorigin);
		testorigin[2] += 10;
		leaf = Mod_PointInLeaf (testorigin, cl.worldmodel);
		if (leaf->contents == CONTENTS_EMPTY)
			r_viewleaf2 = leaf;
	}
	else if (r_viewleaf->contents == CONTENTS_EMPTY)
	{
		// look down a bit
		VectorCopy (r_origin, testorigin);
		testorigin[2] -= 10;
		leaf = Mod_PointInLeaf (testorigin, cl.worldmodel);
		if (leaf->contents <= CONTENTS_WATER &&	leaf->contents >= CONTENTS_LAVA)
			r_viewleaf2 = leaf;
	}
}


// build the transformation matrix for the given view angles
static void Scene_SetOrigin_Angles_Vars (void)
{
	VectorCopy (r_refdef.vieworg, r_origin);

	AngleVectors (r_refdef.viewangles, vpn, vright, vup);
}
	


void Frustum_Set_fovx_fovy (float *my_r_fovx, float *my_r_fovy)
{
//	extern float r_fovx, r_fovy;
//	r_fovx = r_refdef.fov_x;
//	r_fovy = r_refdef.fov_y;
	*my_r_fovx = r_refdef.fov_x;
	*my_r_fovy = r_refdef.fov_y;
}


void Frustum_Add_WaterWarp (float *my_r_fovx, float *my_r_fovy, const int contents)
{
	if (!r_water_warp.integer) return;
	//johnfitz -- calculate r_fovx and r_fovy here


	if (!(contents == CONTENTS_WATER || contents == CONTENTS_SLIME || contents == CONTENTS_LAVA)) return;

	//variance is a percentage of width, where width = 2 * tan(fov / 2) otherwise the effect is too dramatic at high FOV and too subtle at low FOV.  what a mess!
	// Kurok 12.1 - alternate waterwarp
	if (game_kurok.integer)
	{
		*my_r_fovx = atan(tan(DEG2RAD(r_refdef.fov_x) / 2) * (0.97 + sin(cl.time * 1) * 0.04)) * 2 / M_PI_DIV_180;
		*my_r_fovy = atan(tan(DEG2RAD(r_refdef.fov_y) / 2) * (1.03 - sin(cl.time * 2) * 0.04)) * 2 / M_PI_DIV_180;
	}
	else
	{
		*my_r_fovx = atan(tan(DEG2RAD(r_refdef.fov_x) / 2) * (0.97 + sin(cl.time * 1.5) * 0.03)) * 2 / M_PI_DIV_180;
		*my_r_fovy = atan(tan(DEG2RAD(r_refdef.fov_y) / 2) * (1.03 - sin(cl.time * 1.5) * 0.03)) * 2 / M_PI_DIV_180;
	}


	//johnfitz	
}

// Finalizes all states, lightmaps, visibility surfaces for the scene 
// to achieve separation from rendering
//
// What must be here ...
//
// 0. Determine frustum and view leaf										We need this information to determine content blend
//																			We need this information for culling
//																			We need this information for visibility 
//
// 1. Determine final visibility of all map and brush model surfaces.		We need to know what surfaces need lightmap calculation.
// 2. Do any dynamic light calculations that need to be made.				We need this information.for lightmap calculations.
// 3. Finalize all lightmaps.  We need them for shadow calculations.		
// 2. Run through models and do ..
// ------ Interpolation calculations.  This occurs even if culled.
// ------ Frustum culling
// ------ Light calculations
// ------ Shadow calculations


// The best way to do multiview would be to aggregate all the surfaces that can possibly be seen and all the entities that could possibly be seen

qbool	in_scene_setup;
void QMB_RunParticles (void);
void Scene_Setup (void)
{
	// Baker: REMEMBER that all culling is dangerous to multiview ambitions

	in_scene_setup = true;								// Use this to Con_Printf anything that shouldn't occur during scene setup (like OpenGL commands that are not uploading)
														// Or anything that occurs during rendering that shouldn't occur then ... like building lightmaps, dlight calcs or texture upload
														// Or the status bar or QMB doing calculations that persist from frame to frame, cannot have multiview with 3 viewports
														// Advancing the timing 3 times, you know ...


	CL_RunParticles ();		/* multiview neutral*/		// Needs client time (uses cl.ctime, which is the same as cl.time except when rewinding a demo
	QMB_RunParticles ();


	Light_FrameSetup ();	/* multiview neutral*/		// sStain_FrameSetup_LessenStains			()	// Reduces the stains in the lightmap blocks
		/* this doesn't do brush models -> */			// sLight_FrameSetup_PushDlights			()	// Updates which surfaces are affected by a dynamic light
														// sLight_FrameSetup_AnimateLight_Styles	()	// AnimateLight updates animated lights like flickering and pulsing light

														// Light_PushDlights_DrawModel_Brush_Ent (needed!) ...	this DOES occur for r_brushmodels_with_world (submodels)
														//														However, r_brushmodels_with_world does not occur for models with 
														//														with an origin or with an angle (moving, spinning, etc.)

														//	Even with draw_with_world option moving/angled submodels and brush models, sLight_MarkLights occurs in R_DrawModel_Brush
														//  Under no circumstances do instanced brush models receive lighting from dynamic lights, must be world or world submodel

	r_framecount++;										// Advances the framecount.  Not quite sure why this couldn't occur earlier, although above functions are not written for that.


	Scene_SetOrigin_Angles_Vars ();						// Sets r_origin					used by tons of rendering functions and client audio updates			
														// Sets vpn, vright, vup		... used by frustum, client audio updates, particle calculations, QMB, flashblend lights
														// I don't want these variables used for prescene calculations, only rendering and audio
														// Because prescene could be setup for multiple viewports

	Scene_CalculateViewLeafs ();						// Sets r_viewleaf, r_viewleaf2 and old leaf information


	Viewblends_CalculateFrame (r_viewleaf->contents);	// Calculates viewblend for powerups, contents and mixes the final color.


	Frustum_Set_fovx_fovy				(&r_fovx, &r_fovy);							// Set r_fovx and r_fovy variables to refdef values
	Frustum_Add_WaterWarp				(&r_fovx, &r_fovy, r_viewleaf->contents);	// Modify if water warp is on and contents is a liquid
	Frustum_ViewSetup_SetFrustum		(r_fovx, r_refdef.fov_y);					// Set the frustum 

	Frustum_ViewSetup_MarkVisibleLeaves ();											// done here so we know if we're in water
																					// r_visframecount++ here
																					// r_oldviewleaf = r_viewleaf; Occurs here
																					// Goes through and marks nodes .. node->visframe = r_visframecount;
																					// Unlike FitzQuake is not marking surfaces
																					// Is building a visibility table (vis)
																					// Why is FitzQuake marking the leaves, how is it doing it?

	// We are still doing the lightmap for certain things and all dynamic light within the rendering sequence
	// Baker: THIS IS CONTAMINATED WITH OPENGL RENDERING!!!!
	R_Build_Chains_World ();							// Marks surfaces + Efrags; Uses recursive world node to build the texture chains for everything except lightmaps
	R_Build_Chains_World_SubModels ();

	Light_UploadLightmaps_Modified ();	// Baker: If multiview, we would need to build the world chains for all views first
										//        OR ... have this be part of the rendering loop?

	in_scene_setup = false;
}

void Scene_PostScene (void)
{
	// Anything that runs at the end of the frame that prepares for next frame
	Light_DecayLights ();										// Baker: For the next frame
	Viewblends_FadeDamageBonus_Buckets ();						// Once post frame
}
