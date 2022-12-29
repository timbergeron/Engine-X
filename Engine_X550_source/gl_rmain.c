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



//void R_MarkLeaves (void);
void R_InitBubble (void);


//
//void R_RotateForEntity (entity_t *ent)
//{
//	eglTranslatef (ent->origin[0], ent->origin[1], ent->origin[2]);
//
//	eglRotatef (ent->angles[1], 0, 0, 1);
//	eglRotatef (-ent->angles[0], 0, 1, 0);
//	eglRotatef (ent->angles[2], 1, 0, 0);
//}

/*
===============
R_RotateForEntity -- johnfitz -- modified to take origin and angles instead of pointer to entity
===============
*/
void R_RotateForEntity (vec3_t origin, vec3_t angles)
{
	eglTranslatef (origin[0],  origin[1],  origin[2]);
	eglRotatef (angles[1],  0, 0, 1);
	eglRotatef (-angles[0],  0, 1, 0);
	eglRotatef (angles[2],  1, 0, 0);
}





/*
=============
R_DrawEntitiesOnList
=============
*/

void R_DrawEntitiesOnList (const qbool alphapass)
{
	int	i;
	entity_t	*current_ent;

	if (!r_drawentities.integer)						return;		// Don't draw entities
	
	if (!r_drawentities_alpha.integer && alphapass)		return;		// Don't draw alpha entities

//	Con_Printf ("%i cl_numvisedicts\n", cl_numvisedicts);
	
	//johnfitz -- sprites are not a special case
	for (i=0 ; i<cl_numvisedicts ; i++)
	{
		int			entnum		= cl_visedicts[i] - cl_entities;
		char		*curmodel 	= cl_visedicts[i]->model->name;
		current_ent = cl_visedicts[i];
		

		//johnfitz -- if alphapass is true, draw only alpha entites this time
		//if alphapass is false, draw only nonalpha entities this time
		if ((ENTALPHA_DECODE(current_ent->alpha) < 1 && !alphapass) ||
			(ENTALPHA_DECODE(current_ent->alpha) == 1 && alphapass))
			continue;

//#ifdef _DEBUG
		if (r_nodrawentnum.integer > 0 && r_nodrawentnum.integer ==  entnum) continue;
		if (r_nodrawentnum.integer < 0 && r_nodrawentnum.integer != -entnum) continue;
//#endif

		// Checks if a flame model and if QMB replaces we do not draw here
		if (qmb_initialized && SetFlameModelState(current_ent))
			continue;

#if 0
		//johnfitz -- chasecam
		if (currententity == &cl_entities[cl.viewentity])
			currententity->angles[0] *= 0.3;
		//johnfitz
#endif

		switch (current_ent->model->modelformat)
		{
		case mod_alias:
		
			R_DrawModel_Alias (current_ent, false /*shadowpass */);
			break;

		case mod_md3:
			R_DrawQ3Model (current_ent);
			break;

		case mod_brush:

			R_DrawModel_Brush (current_ent);
			break;

		case mod_sprite:
			R_DrawModel_Sprite (current_ent->model, current_ent->origin, current_ent->angles, current_ent->frame, current_ent->syncbase);
			break;

		case mod_spr32:
			R_DrawModel_Sprite (current_ent->model, current_ent->origin, current_ent->angles, current_ent->frame, current_ent->syncbase);
			break;

		default:
			break;
		}
	}
#pragma message ("Quality assurance: Shadows are stripped now")
#pragma message ("Quality assurance: gl_clear 1 I bet we have that busted")
}



// Baker: Split everything into
//        "Setup"
//        "The screen" --- 2D stuff not owned by the HUD (console, etc). and things like total screen brightening, fullscreen non-overlay HUD
//        "The view"   --- The scene with other attributes (flashes, viewweapon, maybe overlay HUD) overlayed
//        "The scene"  --- The scene is the bare 3D world.  With whatever view properities.




/*
================
R_RenderView

r_refdef must be set before the first call
================
*/



void Scene_Setup		(void);
void Scene_Render		(void);
void Scene_PostScene	(void);

void R_RenderView (void)
{
	double	time1 = 0.0, time2;
//	GLfloat colors[4] = {(GLfloat) 0.0, (GLfloat) 0.0, (GLfloat) 1, (GLfloat) 0.20};


	if (/*!r_worldentity.model ||*/ !cl.worldmodel)
		Sys_Error ("R_RenderView: NULL worldmodel");

	if (r_speeds.integer)
	{
		eglFinish ();
		time1 = Sys_DoubleTime ();
//		c_brush_polys = 0;
//		c_alias_polys = 0;
	}


	Scene_Setup ();
	Scene_Render ();
	Scene_PostScene ();

	if (gl_finish.integer)
		eglFinish ();

	if (r_speeds.integer)
	{
		time2 = Sys_DoubleTime ();
		Con_Printf ("%3i ms  %4i wpoly %4i epoly m3poly %4i\n", (int)((time2 - time1) * 1000), c_brush_polys, c_alias_polys, c_md3_polys);
	}
}



/*
====================
R_TimeRefresh_f

For program optimization
====================
*/
void R_TimeRefresh_f (void)
{
	int	i;
	float	start, stop, time;

	if (cls.state != ca_connected)
		return;

	eglDrawBuffer (GL_FRONT);
	eglFinish ();

	start = Sys_DoubleTime ();
	for (i=0 ; i<128 ; i++)
	{
		r_refdef.viewangles[1] = i * (360.0 / 128.0);
		R_RenderView ();
	}

	eglFinish ();
	stop = Sys_DoubleTime ();
	time = stop - start;
	Con_Printf ("%f seconds (%f fps)\n", time, 128.0 / time);

	eglDrawBuffer (GL_BACK);
	GL_EndRendering ();
}



static void SpecialTextures_Init (void)
{
	ImageWork_Start ("Special textures", "specials");

	// Load up our special textures
	underwatertexture = GL_LoadExternalTextureImage ("gfx/effects/watercaustics", NULL, TEX_MIPMAP | TEX_ALPHA_TEST | TEX_COMPLAIN, NULL /*PATH LIMIT ME*/);
	damagetexture = GL_LoadExternalTextureImage ("gfx/effects/damagescreen", NULL, TEX_MIPMAP | TEX_ALPHA_TEST | TEX_COMPLAIN, NULL /*PATH LIMIT ME*/);
	poweruptexture = GL_LoadExternalTextureImage ("gfx/effects/powerup", NULL, TEX_MIPMAP | TEX_ALPHA_TEST | TEX_COMPLAIN, NULL /*PATH LIMIT ME*/);
	glasstexture = GL_LoadExternalTextureImage ("gfx/effects/glass", NULL, TEX_MIPMAP | TEX_COMPLAIN, NULL /*PATH LIMIT ME*/);

	detailtexture = GL_LoadExternalTextureImage ("gfx/effects/detail", NULL, /*256, 256,*/ TEX_MIPMAP | TEX_COMPLAIN, NULL /*PATH LIMIT ME*/);
	selectiontexture = GL_LoadExternalTextureImage ("gfx/effects/selection", NULL, /*256, 256,*/ TEX_MIPMAP | TEX_ALPHA_TEST | /*(r_pass_detail.integer ? */TEX_COMPLAIN /*: 0*/, NULL /*PATH LIMIT ME*/);

	// Baker: We need to make sure size locked textures are enforced

	ImageWork_Finish ();
}

/*
===============
R_Init
===============
*/
void R_Init (void)
{
	void R_ToggleParticles_f (void);
	void Sky_LoadSky_f (void);

	Cmd_AddCommand ("timerefresh", R_TimeRefresh_f);
	Cmd_AddCommand ("pointfile", R_ReadPointFile_f);
	Cmd_AddCommand ("toggleparticles", R_ToggleParticles_f);
	Cmd_AddCommand ("set_interpolated_weapon", Set_Interpolated_Weapon_f);

	Cvar_Registration_Client_Rendering_Options_3D ();

//	Cvar_RegisterVariable (&gl_fogsky, NULL);
	Cmd_AddCommand		  ("loadsky", Sky_LoadSky_f);

	if (COM_StringMatch (gl_vendor, "METABYTE/WICKED3D"))
		Cvar_SetDefaultFloatByRef (&particle_blend, 1);



	R_InitTextures ();
	Light_Init ();
	Particles_Init ();
	VertexLights_Init ();

	SpecialTextures_Init ();


}
