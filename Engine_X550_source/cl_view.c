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
// cl_view.c -- player eye positioning

#include "quakedef.h"

/*

The view is allowed to move slightly from its true position for bobbing,
but if it exceeds 8 pixels linear distance (spherical, not box), the list of
entities sent from the server may not include everything in the pvs, especially
when crossing a water boudnary.

*/

float	v_dmg_time, v_dmg_roll, v_dmg_pitch;



// Also used by sv_user.c
float View_CalcRoll (const vec3_t angles, const vec3_t velocity)
{
	float	sign;
	float	side;
	vec3_t	right;

	AngleVectors (angles, NULL, right, NULL);
//	AngleVectors (angles, forward, right, up);
	side = DotProduct(velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabsf(side);

	if (side < v_rollspeed.floater)
		side = side * v_rollangle.floater / v_rollspeed.floater;
	else
		side = v_rollangle.floater;


	return side * sign;

}


/*
===============
V_CalcBob
===============
*/
static float sView_CalcBob (const float bobcycle, const float bobup, const float bobamount)
{
	float		bob;
	float		cycle;

	if (!bobcycle)
		return 0;

	cycle = cl.ctime - (int)(cl.ctime / bobcycle) * bobcycle;
	cycle /= bobcycle;
	if (cycle < bobup)
		cycle = M_PI * cycle / bobup;
	else
		cycle = M_PI + M_PI * (cycle - bobup) / (1.0 - bobup);

// bob is proportional to velocity in the xy plane
// (don't count Z, or jumping messes it up)

	bob = sqrtf(cl.velocity[0]*cl.velocity[0] + cl.velocity[1]*cl.velocity[1]) * bobamount;
//Con_Printf ("speed: %5.1f\n", Length(cl.velocity));
	bob = bob * 0.3 + bob * 0.7 * sinf(cycle);
	bob = CLAMP (-7, bob, 4);

	return bob;
}

//=============================================================================



void View_StartPitchDrift_f (void)
{
#if 1
	if (cl.laststop == cl.ctime)
		return;		// something else is keeping it from drifting
#endif
	if (cl.nodrift || !cl.pitchvel)
	{
		cl.pitchvel = v_centerspeed.floater;
		cl.nodrift = false;
		cl.driftmove = 0;
	}
}

void View_StopPitchDrift (void)
{
	cl.laststop = cl.ctime;
	cl.nodrift = true;
	cl.pitchvel = 0;
}

/*
===============
V_DriftPitch

Moves the client pitch angle towards cl.idealpitch sent by the server.

If the user is adjusting pitch manually, either with lookup/lookdown,
mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.

Drifting is enabled when the center view key is hit, mlook is released and
lookspring is non 0, or when
===============
*/
static void sView_DriftPitch (void)
{
	float	delta, move;

	if (cl.noclip_anglehack || !cl.onground || cls.demoplayback)
	//FIXME: noclip_anglehack is set on the server, so in a nonlocal game this won't work.
	{
		cl.driftmove = 0;
		cl.pitchvel = 0;
		return;
	}

// don't count small mouse motion
	if (cl.nodrift)
	{
		if (fabsf(cl.cmd.forwardmove) < cl_speed_forward.floater)
			cl.driftmove = 0;
		else
			cl.driftmove += host_frametime;

		if (cl.driftmove > v_centermove.floater)
			View_StartPitchDrift_f ();

		return;
	}

	delta = cl.idealpitch - cl.viewangles[PITCH];

	if (!delta)
	{
		cl.pitchvel = 0;
		return;
	}

	move = host_frametime * cl.pitchvel;
	cl.pitchvel += host_frametime * v_centerspeed.floater;

//Con_Printf ("move: %f (%f)\n", move, host_frametime);

	if (delta > 0)
	{
		if (move > delta)
		{
			cl.pitchvel = 0;
			move = delta;
		}
		cl.viewangles[PITCH] += move;
	}
	else if (delta < 0)
	{
		if (move > -delta)
		{
			cl.pitchvel = 0;
			move = -delta;
		}
		cl.viewangles[PITCH] -= move;
	}
}


/*
==============================================================================

				VIEW RENDERING

==============================================================================
*/

static float sAngleDelta (float a)
{
	a = anglemod(a);
	if (a > 180)
		a -= 360;
	return a;
}
#pragma message ("Quality assurance: sAngleDelta needs promoted to mathlib.c or .h maybe.  Similar thing used in cl_tent.c")

/*
==================
CalcGunAngle
==================
*/
static void sView_CalcGunAngle (void)
{
	float		yaw, pitch, move;
	static	float	oldyaw = 0;
	static	float	oldpitch = 0;

	yaw = r_refdef.viewangles[YAW];
	pitch = -r_refdef.viewangles[PITCH];

	yaw = sAngleDelta(yaw - r_refdef.viewangles[YAW]) * 0.4;
	yaw = CLAMP (-10, yaw, 10);

	pitch = sAngleDelta(-pitch - r_refdef.viewangles[PITCH]) * 0.4;
	pitch = CLAMP (-10, pitch, 10);

	move = host_frametime * 20;
	if (yaw > oldyaw)
	{
		if (oldyaw + move < yaw)
			yaw = oldyaw + move;
	}
	else
	{
		if (oldyaw - move > yaw)
			yaw = oldyaw - move;
	}

	if (pitch > oldpitch)
	{
		if (oldpitch + move < pitch)
			pitch = oldpitch + move;
	}
	else
	{
		if (oldpitch - move > pitch)
			pitch = oldpitch - move;
	}

	oldyaw = yaw;
	oldpitch = pitch;

	cl.viewmodel_ent.angles[YAW] = r_refdef.viewangles[YAW] + yaw;
	cl.viewmodel_ent.angles[PITCH] = -(r_refdef.viewangles[PITCH] + pitch);

	cl.viewmodel_ent.angles[ROLL] = r_refdef.viewangles[ROLL];		// joe: this makes it fix when strafing

	if (game_kurok.integer) // Kurok weapon switch transition and movement bobbing
//	    if (scr_fov.floater == 90)
	        if (r_viewmodel_pitch.floater)
	            cl.viewmodel_ent.angles[PITCH] -= r_viewmodel_pitch.floater;

#pragma message ("Quality assurance:  Kurok FOV 90 stuff can go if we aren't using scope.mdl.  Enable gun pitch for all mods as v_viewmodel_pitch")

	cl.viewmodel_ent.angles[ROLL] -= v_idlescale.floater * sinf(cl.ctime*v_iroll_cycle.floater) * v_iroll_level.floater;
	cl.viewmodel_ent.angles[PITCH] -= v_idlescale.floater * sinf(cl.ctime*v_ipitch_cycle.floater) * v_ipitch_level.floater;
	cl.viewmodel_ent.angles[YAW] -= v_idlescale.floater * sinf(cl.ctime*v_iyaw_cycle.floater) * v_iyaw_level.floater;
}

/*
==============
V_BoundOffsets
==============
*/
static void sView_BoundOffsets (void)
{
	entity_t	*ent;

	ent = &cl_entities[cl.player_point_of_view_entity];

	// absolutely bound refresh relative to entity clipping hull
	// so the view can never be inside a solid wall
	r_refdef.vieworg[0] = max(r_refdef.vieworg[0], ent->origin[0] - 14);
	r_refdef.vieworg[0] = min(r_refdef.vieworg[0], ent->origin[0] + 14);
	r_refdef.vieworg[1] = max(r_refdef.vieworg[1], ent->origin[1] - 14);
	r_refdef.vieworg[1] = min(r_refdef.vieworg[1], ent->origin[1] + 14);
	r_refdef.vieworg[2] = max(r_refdef.vieworg[2], ent->origin[2] - 22);
	r_refdef.vieworg[2] = min(r_refdef.vieworg[2], ent->origin[2] + 30);
}

/*
==============
V_AddIdle

Idle swaying
==============
*/
static void sView_AddIdle (const float idlescale)
{
//	return;
// Kurok 31.5
    if (game_kurok.integer)
    {
        double xyspeed;
	    float bspeed;

        xyspeed = sqrtf(cl.velocity[0]*cl.velocity[0] + cl.velocity[1]*cl.velocity[1]);
        bspeed = xyspeed * 0.15f;

	    r_refdef.viewangles[ROLL] += idlescale * sinf(cl.ctime*v_iroll_cycle.floater) * (v_iroll_level.floater * bspeed);
	    r_refdef.viewangles[PITCH] += idlescale * sinf(cl.ctime*v_ipitch_cycle.floater) * (v_ipitch_level.floater * bspeed);
	    r_refdef.viewangles[YAW] += idlescale * sinf(cl.ctime*v_iyaw_cycle.floater) * (v_iyaw_level.floater * bspeed);

  	    r_refdef.viewangles[ROLL] += (idlescale * 3 ) * sinf(cl.ctime*v_iroll_cycle.floater * 0.25 ) * v_iroll_level.floater;
	    r_refdef.viewangles[PITCH] += (idlescale * 3 ) * sinf(cl.ctime*v_ipitch_cycle.floater * 0.25 ) * v_ipitch_level.floater;
	    r_refdef.viewangles[YAW] += (idlescale * 3 ) * sinf(cl.ctime*v_iyaw_cycle.floater * 0.25 ) * v_iyaw_level.floater;
    }
    else
    {
		r_refdef.viewangles[ROLL] += idlescale * sinf(cl.ctime*v_iroll_cycle.floater) * v_iroll_level.floater;
		r_refdef.viewangles[PITCH] += idlescale * sinf(cl.ctime*v_ipitch_cycle.floater) * v_ipitch_level.floater;
		r_refdef.viewangles[YAW] += idlescale * sinf(cl.ctime*v_iyaw_cycle.floater) * v_iyaw_level.floater;
	}
}


/*
==============
V_CalcViewRoll

Roll is induced by movement and damage
==============
*/
void View_CalcViewRoll (void)
{
	float	side;

	side = View_CalcRoll (cl_entities[cl.player_point_of_view_entity].angles, cl.velocity);
	r_refdef.viewangles[ROLL] += side;

	if (v_dmg_time > 0)
	{
		r_refdef.viewangles[ROLL] += v_dmg_time / v_kicktime.floater * v_dmg_roll;
		r_refdef.viewangles[PITCH] += v_dmg_time / v_kicktime.floater * v_dmg_pitch;
		v_dmg_time -= host_frametime;
	}
}

void View_AddViewWeapon (const float bob, const float bobside)
{
//	int		i;
	vec3_t		forward, right, up;
	entity_t	*view;

	view = &cl.viewmodel_ent;


	// Copy over angles
	sView_CalcGunAngle ();
//	view->angles[YAW] = r_refdef.viewangles[YAW];
//	view->angles[PITCH] = -r_refdef.viewangles[PITCH];
//	view->angles[ROLL] = r_refdef.viewangles[ROLL];

	// Copy over origin
	AngleVectors (r_refdef.viewangles, forward, right, up);
	VectorCopy (r_refdef.vieworg, view->origin);

	// Add bob and bobside
	if (bobside == 0)
		VectorMultiplyAdd (view->origin, bob * 0.4, forward, view->origin);
	else
	{
		int i;
		for (i=0 ; i<3 ; i++)
		{
		//		view->origin[i] += forward[i]*bob*0.4;
				view->origin[i] += right[i]*bobside*0.2;
				view->origin[i] += up[i]*bob*0.2;
		}
	}

#if 1
	// Add r_viewmodeloffset cvar into the equation
	if (r_viewmodel_offset.string[0])
	{
		float offset[3];
		int size = sizeof(offset)/sizeof(offset[0]);

		ParseFloats(r_viewmodel_offset.string, offset, &size);
		VectorMultiplyAdd (view->origin,  offset[0], right,   view->origin);
		VectorMultiplyAdd (view->origin, -offset[1], up,      view->origin);
		VectorMultiplyAdd (view->origin,  offset[2], forward, view->origin);
	}
#endif 

	// fudge position around to keep amount of weapon visible roughly equal with different FOV
	if (!r_viewmodel_hackpos.integer)
	{
		if (scene_viewsize.floater == 110)
			view->origin[2] += 1;
		else if (scene_viewsize.floater == 100)
			view->origin[2] += 2;
		else if (scene_viewsize.floater == 90)
			view->origin[2] += 1;
		else if (scene_viewsize.floater == 80)
			view->origin[2] += 0.5;
	}

	// Populate other information
	view->model = cl.model_precache[cl.stats[STAT_WEAPON]];
	view->frame = cl.stats[STAT_WEAPONFRAME];
	view->colormap = vid.colormap;
}



/*
==================
V_CalcIntermissionRefdef
==================
*/
void ViewSet_CalcIntermissionRefdef (void)
{
	entity_t	*ent, *view;

	
	ent = &cl_entities[cl.player_point_of_view_entity];			// ent is the player model (visible when out of body)
	view = &cl.viewmodel_ent;									// view is the weapon model (only visible from inside body)

	VectorCopy (ent->origin, r_refdef.vieworg);
	VectorCopy (ent->angles, r_refdef.viewangles);
	view->model = NULL;

	// always idle in intermission
// Kurok 31.6
    if (!game_kurok.integer)
    {
		sView_AddIdle (1);
	}
#pragma message ("Quality assurance: Possibly add a cvar called intermission idlescale to de-hack from Kurok")
}


float	cl_punchangle, cl_ideal_punchangle;

static void sView_AddGunKick (void)
{
	vec3_t		forward;

// set up the refresh position
	if (v_gunkick.integer == 2)
	{
		// add weapon kick offset
		AngleVectors (r_refdef.viewangles, forward, NULL, NULL);
		VectorMultiplyAdd (r_refdef.vieworg, cl_punchangle, forward, r_refdef.vieworg);

		// add weapon kick angle
		r_refdef.viewangles[PITCH] += cl_punchangle * 0.5;
	}
	else if (v_gunkick.integer)
		VectorAdd (r_refdef.viewangles, cl.punchangle, r_refdef.viewangles);
}

#if 0
	if (chase_active.integer)
	{
		// set up the refresh position
		VectorCopy (ent->origin, r_refdef.vieworg);		
	// never let it sit exactly on a node line, because a water plane can
	// disappear when viewed with the eye exactly on it.
	// the server protocol only specifies to 1/16 pixel, so add 1/32 in each axis
	r_refdef.vieworg[0] += 1.0 / 32;
	r_refdef.vieworg[1] += 1.0 / 32;
	r_refdef.vieworg[2] += 1.0 / 32;

	// add view height
	r_refdef.vieworg[2] += cl.viewheight + bob + cl.crouch;	// smooth out stair step ups
		
		// set up refresh view angles
		VectorCopy (cl.lerpangles /* cl.viewangles */, r_refdef.viewangles);  // JPG - viewangles -> lerpangles
		Chase_Update ();
		sView_AddIdle (v_idlescale.floater);
		sView_AddGunKick ();
		return;
	}
#endif

/*
==================
V_CalcRefdef
==================
*/
void ViewSet_CalcRefdef (void)
{
	entity_t		*ent, *view;
	static float	bob =0, bobside = 0;

	sView_DriftPitch ();

	ent = &cl_entities[cl.player_point_of_view_entity];			// ent is the player model (visible when out of body)
	view = &cl.viewmodel_ent;									// view is the weapon model (only visible from inside body)
	
	if (!chase_active.integer)	// Chase_active doesn't get input yaw or pitch, nor does it get bobbing
	{
		// transform the view offset by the model's matrix to get the offset from
		// model origin for the view
		// JPG - viewangles -> lerpangles
		ent->angles[YAW] = cl.lerpangles[YAW]; 	// the model should face the view dir
		ent->angles[PITCH] = -cl.lerpangles[PITCH]; // the model should face the view dir

		// Kurok 31.7

		// Baker: If kurok only calc bob if onground 	
		if (!game_kurok.integer || !cl.noclip_anglehack) // Kurok's onground flag appears to be broken.
		{
			bob = sView_CalcBob (v_bobcycle.floater, v_bobup.floater, v_bob.floater);
			bobside = sView_CalcBob (v_bobsidecycle.floater, v_bobsideup.floater, v_bobside.floater);
		}
		// Otherwise use staticly preserved values
		// That we should reset on map change but don't
	}

	// set up the refresh position
	VectorCopy (ent->origin, r_refdef.vieworg);

	// never let it sit exactly on a node line, because a water plane can
	// disappear when viewed with the eye exactly on it.
	// the server protocol only specifies to 1/16 pixel, so add 1/32 in each axis
	r_refdef.vieworg[0] += 1.0 / 32;
	r_refdef.vieworg[1] += 1.0 / 32;
	r_refdef.vieworg[2] += 1.0 / 32;

	// add view height
	r_refdef.vieworg[2] += cl.viewheight + bob + cl.crouch;	// smooth out stair step ups

	// set up refresh view angles
	VectorCopy (cl.lerpangles, r_refdef.viewangles);  // JPG - viewangles -> lerpangles
	
	if (chase_active.integer)	// Chase active 
		Chase_Update ();

	if (!chase_active.integer)	// Chase active does not get view calculations
	{
		View_CalcViewRoll ();
	}
	

	sView_AddIdle (v_idlescale.floater);
	sView_AddGunKick ();

	
	if (!chase_active.integer)		// Chase active gets no death angles and no gun
	{
		if (cl.stats[STAT_HEALTH] <= 0)
			r_refdef.viewangles[ROLL] = 80;	// dead view angle

	//	CalcGunAngle (); ... Moved into V_AddViewWeapon

		View_AddViewWeapon (bob, bobside);
	}
}



// the client maintains its own idea of view angles, which are
// sent to the server each frame.  The server sets punchangle when
// the view is temporarliy offset, and an angle reset commands at the start
// of each level and after teleporting.


static void sView_DropPunchAngle (void)
{
	if (cl_ideal_punchangle < cl_punchangle)
	{
		if (cl_ideal_punchangle >= -2)		// small kick
			cl_punchangle -= 20 * host_frametime;
		else					// big kick
			cl_punchangle -= 40 * host_frametime;

		if (cl_punchangle < cl_ideal_punchangle)
		{
			cl_punchangle = cl_ideal_punchangle;
			cl_ideal_punchangle = 0;
		}
	}
	else
	{
		cl_punchangle += 20 * host_frametime;
		if (cl_punchangle > 0)
			cl_punchangle = 0;
	}
}

void View_CalculateDamageRollPitchTime (const float severity, const float rollside, const float pitchside)
{
	v_dmg_roll  = severity * rollside * v_kickroll.floater;
	v_dmg_pitch = severity * pitchside * v_kickpitch.floater;
	v_dmg_time  = v_kicktime.floater;
}




/*
==================
V_RenderView

The player's clipping box goes from (-16 -16 -24) to (16 16 32) from
the entity origin, so any view position inside that will be valid
==================
*/
void View_RenderView (void)
{

	if (con_forcedup)
		return;

// Baker:  Not multiviewport neutral!!!!!

// don't allow cheats in multiplayer
	if (cl.gametype == GAME_DEATHMATCH && cl.maxclients > 1)
	{
//		if (r_fullbright.integer)		Cvar_SetFloatByRef (&r_fullbright, 0);
		if (r_lightmap.integer)			Cvar_SetFloatByRef (&r_lightmap, 0);
//		if (r_fullbrightskins.integer)	Cvar_SetFloatByRef (&r_fullbrightskins, 0);


//#ifndef GLQUAKE
//		Cvar_SetFloatByRef (&r_draworder, 0);
//		Cvar_SetFloatByRef (&r_ambient, 0);
//		Cvar_SetFloatByRef (&r_drawflat, 0);
//#endif
	}


	sView_DropPunchAngle ();	// Baker!  

	if (cl.intermission)	// intermission / finale rendering
		ViewSet_CalcIntermissionRefdef ();
	else if (!cl.paused)
		ViewSet_CalcRefdef ();




	R_RenderView ();



#if SUPPORTS_TEXTURE_POINTER

	if (tool_texturepointer.integer)			// Texture pointer
		TexturePointer_SurfacePoint ();

#endif

	Insertions_Draw ();

	R_DrawViewModel ();
}




//============================================================================


/*
=============
V_Init
=============
*/
void View_Init (void)
{

	Cmd_AddCommand ("v_cshift", Viewblends_DefineEmptyColor_cshiftcmd_f);
	Cmd_AddCommand ("bf", Viewblends_SetBonus_ColorBucket_f);
	Cmd_AddCommand ("centerview", View_StartPitchDrift_f);
//	Cmd_AddCommand ("cl_gunpitch", Empty_f);

	// Cvar Registration For View

	Cvar_Registration_Client_View ();

	Cvar_Registration_Client_ViewBlends ();

	Cvar_Registration_Client_ViewModel ();


}

