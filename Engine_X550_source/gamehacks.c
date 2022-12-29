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
// gamehacks.c -- game hacks

#include "quakedef.h"


modhint_t GameHacks_IsSpecialQuakeAliasModel (const char *model_name)
{
	// NOTE: comparing not only with player.mdl, but with all models
	// begin with "player" coz we need to support DME models as well!
	if 	    (!strncmp(model_name, "progs/player", 12)			)	return MOD_PLAYER;
	else if (COM_StringMatch (model_name, "progs/eyes.mdl")		)	return MOD_EYES;
	else if (COM_StringMatch (model_name, "progs/flame0.mdl")	)	return MOD_FLAME;
	else if (COM_StringMatch (model_name, "progs/flame.mdl")	)	return MOD_FLAME;
	else if (COM_StringMatch (model_name, "progs/flame2.mdl")	)	return MOD_FLAME;
	else if (COM_StringMatch (model_name, "progs/bolt.mdl")		)	return MOD_THUNDERBOLT;
	else if (COM_StringMatch (model_name, "progs/bolt2.mdl")	)	return MOD_THUNDERBOLT;
	else if (COM_StringMatch (model_name, "progs/bolt3.mdl")	)	return MOD_THUNDERBOLT;
	else if (COM_StringMatch (model_name, "progs/v_shot.mdl")	)	return MOD_WEAPON;
	else if (COM_StringMatch (model_name, "progs/v_shot2.mdl")	)	return MOD_WEAPON;
	else if (COM_StringMatch (model_name, "progs/v_nail.mdl")	)	return MOD_WEAPON;
	else if (COM_StringMatch (model_name, "progs/v_nail2.mdl")	)	return MOD_WEAPON;
	else if (COM_StringMatch (model_name, "progs/v_rock.mdl")	)	return MOD_WEAPON;
	else if (COM_StringMatch (model_name, "progs/v_rock2.mdl")	)	return MOD_WEAPON;
	// hipnotic weapons
	else if (COM_StringMatch (model_name, "progs/v_laserg.mdl")	)	return MOD_WEAPON;
	else if (COM_StringMatch (model_name, "progs/v_prox.mdl")	)	return MOD_WEAPON;
	// rogue weapons
	else if (COM_StringMatch (model_name, "progs/v_grpple.mdl")	)	return MOD_WEAPON;
	else if (COM_StringMatch (model_name, "progs/v_lava.mdl")	)	return MOD_WEAPON;
	else if (COM_StringMatch (model_name, "progs/v_lava2.mdl")	)	return MOD_WEAPON;
	else if (COM_StringMatch (model_name, "progs/v_multi.mdl")	)	return MOD_WEAPON;
	else if (COM_StringMatch (model_name, "progs/v_multi2.mdl") )	return MOD_WEAPON;
	else if (COM_StringMatch (model_name, "progs/v_plasma.mdl") )	return MOD_WEAPON;
	else if (COM_StringMatch (model_name, "progs/v_star.mdl")   )	return MOD_WEAPON;
	else if (COM_StringMatch (model_name, "progs/lavaball.mdl") )	return MOD_LAVABALL;

	else if (COM_StringMatch (model_name, "progs/spike.mdl")	)	return MOD_SPIKE;
	else if (COM_StringMatch (model_name, "progs/s_spike.mdl")	)	return MOD_SPIKE;
	else if (COM_StringMatch (model_name, "progs/shambler.mdl")	)	return MOD_SHAMBLER;

	return MOD_NORMAL;
}

// Baker:	I'm not convinced the replacement is effective ... then again
//			I don't have the code setup to check for .md3 first.  Which is illogical
//			as the progs.dat will specify the extension.  So ... I'm not sure
//			what to think at this point really ... well actually I do ..
//			but I'm not fixing this quite yet ...

int GameHacks_MissingFlags (const char *modelname)
{
// we need to replace missing flags. sigh. 
	if		(COM_StringMatch (modelname, "progs/missile.md3")	)	return EF_ROCKET;
	else if (COM_StringMatch (modelname, "progs/lavaball.md3")	)	return EF_ROCKET;
	else if (COM_StringMatch (modelname, "progs/grenade.md3")	)	return EF_GRENADE;
	else if (COM_StringMatch (modelname, "progs/invulner.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/suit.md3")		)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/invisibl.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/quaddama.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/armor.md3")		)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/b_g_key.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/b_s_key.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/m_g_key.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/m_s_key.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/w_g_key.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/w_s_key.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/end1.md3")		)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/end2.md3")		)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/end3.md3")		)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/end4.md3")		)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/g_shot.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/g_nail.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/g_nail2.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/g_rock.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/g_rock2.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/g_light.md3")	)	return EF_ROTATE;
	else if (COM_StringMatch (modelname, "progs/h_player.md3")	)	return EF_GIB;
	else if (COM_StringMatch (modelname, "progs/gib1.md3")		)	return EF_GIB;	
	else if (COM_StringMatch (modelname, "progs/gib2.md3")		)	return EF_GIB;
	else if (COM_StringMatch (modelname, "progs/gib3.md3")		)	return EF_GIB;

	return 0;
}

qbool GameHacks_IsNotQuakeBoxModel (const char *bspname)
{
	if (COM_StringMatch (bspname, "b_batt0.bsp")  ||
	    COM_StringMatch (bspname, "b_batt1.bsp")  ||
	    COM_StringMatch (bspname, "b_bh10.bsp")   ||
	    COM_StringMatch (bspname, "b_bh100.bsp")  ||
	    COM_StringMatch (bspname, "b_bh25.bsp")   ||
	    COM_StringMatch (bspname, "b_explob.bsp") ||
	    COM_StringMatch (bspname, "b_nail0.bsp")  ||
	    COM_StringMatch (bspname, "b_nail1.bsp")  ||
	    COM_StringMatch (bspname, "b_rock0.bsp")  ||
	    COM_StringMatch (bspname, "b_rock1.bsp")  ||
	    COM_StringMatch (bspname, "b_shell0.bsp") ||
	    COM_StringMatch (bspname, "b_shell1.bsp") ||
	    COM_StringMatch (bspname, "b_exbox2.bsp"))
		return false;

	return true;
}

qbool GameHacks_Is_EXMY_Map (const char *mapname)
{
	// Baker: What are we using this for?  EXMY to prevent external textures for non id1 maps because
	//        it looks real stupid.  If we are gamedir'd, non-original maps even baring the same name
	//        won't get an external texture from enginex dir because is path limited.


	if (COM_StringMatchCaseless (mapname, "start")  ||
		COM_StringMatchCaseless (mapname, "e1m1")   ||
		COM_StringMatchCaseless (mapname, "e1m2")   ||
		COM_StringMatchCaseless (mapname, "e1m3")   ||
		COM_StringMatchCaseless (mapname, "e1m4")   ||
		COM_StringMatchCaseless (mapname, "e1m5")   ||
		COM_StringMatchCaseless (mapname, "e1m6")   ||
		COM_StringMatchCaseless (mapname, "e1m7")   ||
		COM_StringMatchCaseless (mapname, "e1m8")   ||
		COM_StringMatchCaseless (mapname, "e2m1")   ||
		COM_StringMatchCaseless (mapname, "e2m2")   ||
		COM_StringMatchCaseless (mapname, "e2m3")   ||
		COM_StringMatchCaseless (mapname, "e2m4")   ||
		COM_StringMatchCaseless (mapname, "e2m5")   ||
		COM_StringMatchCaseless (mapname, "e2m6")   ||
		COM_StringMatchCaseless (mapname, "e2m7")   ||
		COM_StringMatchCaseless (mapname, "e2m8")   ||
		COM_StringMatchCaseless (mapname, "e3m1")   ||
		COM_StringMatchCaseless (mapname, "e3m2")   ||
		COM_StringMatchCaseless (mapname, "e3m3")   ||
		COM_StringMatchCaseless (mapname, "e3m4")   ||
		COM_StringMatchCaseless (mapname, "e3m5")   ||
		COM_StringMatchCaseless (mapname, "e3m6")   ||
		COM_StringMatchCaseless (mapname, "e3m7")   ||
		COM_StringMatchCaseless (mapname, "e3m8")   ||
		COM_StringMatchCaseless (mapname, "e4m1")   ||
		COM_StringMatchCaseless (mapname, "e4m2")   ||
		COM_StringMatchCaseless (mapname, "e4m3")   ||
		COM_StringMatchCaseless (mapname, "e4m4")   ||
		COM_StringMatchCaseless (mapname, "e4m5")   ||
		COM_StringMatchCaseless (mapname, "e4m6")   ||
		COM_StringMatchCaseless (mapname, "e4m7")   ||
		COM_StringMatchCaseless (mapname, "e4m8")   ||
		COM_StringMatchCaseless (mapname, "end")    ||
		COM_StringMatchCaseless (mapname, "dm1")    ||
		COM_StringMatchCaseless (mapname, "dm2")    ||
		COM_StringMatchCaseless (mapname, "dm3")    ||
		COM_StringMatchCaseless (mapname, "dm4")    ||
		COM_StringMatchCaseless (mapname, "dm5")    ||
		COM_StringMatchCaseless (mapname, "dm6"))
			return true;

	return false;
}


/*

  static	vec3_t	playerbeam_end;
float		ExploColor[3];		// joe: for color mapped explosions
static model_t		*cl_bolt1_mod, *cl_bolt2_mod, *cl_bolt3_mod;

static sfx_t		*cl_sfx_wizhit;
static sfx_t		*cl_sfx_knighthit;
static sfx_t		*cl_sfx_tink1;
static sfx_t		*cl_sfx_ric1;
static sfx_t		*cl_sfx_ric2;
static sfx_t		*cl_sfx_ric3;
static sfx_t		*cl_sfx_r_exp3;

void CL_InitTEnts (void)
{
	cl_sfx_wizhit = S_PrecacheSound ("wizard/hit.wav");
	cl_sfx_knighthit = S_PrecacheSound ("hknight/hit.wav");
	cl_sfx_tink1 = S_PrecacheSound ("weapons/tink1.wav");
	cl_sfx_ric1 = S_PrecacheSound ("weapons/ric1.wav");
	cl_sfx_ric2 = S_PrecacheSound ("weapons/ric2.wav");
	cl_sfx_ric3 = S_PrecacheSound ("weapons/ric3.wav");
	cl_sfx_r_exp3 = S_PrecacheSound ("weapons/r_exp3.wav");
}

*/

// 			SV_StartSound (ent, 0, "misc/h2ohit1.wav", 255, 1);
//			SV_StartSound (ent, 0, "demon/dland2.wav", 255, 1);

//			S_PrecacheSound ("ambience/wind2.wav");
//			S_PrecacheSound ("ambience/water1.wav");

//			S_LocalSound ("misc/talk.wav");

// progs

void CL_InitModelnames (void)
{
	int	i;

	memset (cl_modelnames, 0, sizeof(cl_modelnames));

	cl_modelnames[mi_player] 		= "progs/player.mdl";
// Q3
	cl_modelnames[mi_q3torso] 		= "progs/player/upper.md3";
	cl_modelnames[mi_q3head] 		= "progs/player/head.md3";
// Q3
	cl_modelnames[mi_h_player] 		= "progs/h_player.mdl";
	cl_modelnames[mi_eyes] 			= "progs/eyes.mdl";
	cl_modelnames[mi_rocket] 		= "progs/missile.mdl";
	cl_modelnames[mi_grenade] 		= "progs/grenade.mdl";
	cl_modelnames[mi_flame0] 		= "progs/flame0.mdl";
	cl_modelnames[mi_flame1] 		= "progs/flame.mdl";
	cl_modelnames[mi_flame2] 		= "progs/flame2.mdl";
	cl_modelnames[mi_explo1] 		= "progs/s_expl.spr";
	cl_modelnames[mi_explo2] 		= "progs/s_explod.spr";
	cl_modelnames[mi_bubble] 		= "progs/s_bubble.spr";
	cl_modelnames[mi_gib1] 			= "progs/gib1.mdl";
	cl_modelnames[mi_gib2] 			= "progs/gib2.mdl";
	cl_modelnames[mi_gib3] 			= "progs/gib3.mdl";
	cl_modelnames[mi_fish] 			= "progs/fish.mdl";
	cl_modelnames[mi_dog] 			= "progs/dog.mdl";
	cl_modelnames[mi_soldier] 		= "progs/soldier.mdl";
	cl_modelnames[mi_enforcer] 		= "progs/enforcer.mdl";
	cl_modelnames[mi_knight] 		= "progs/knight.mdl";
	cl_modelnames[mi_hknight] 		= "progs/hknight.mdl";
	cl_modelnames[mi_scrag] 		= "progs/wizard.mdl";
	cl_modelnames[mi_ogre] 			= "progs/ogre.mdl";
	cl_modelnames[mi_fiend] 		= "progs/demon.mdl";
	cl_modelnames[mi_vore] 			= "progs/shalrath.mdl";
	cl_modelnames[mi_shambler] 		= "progs/shambler.mdl";
	cl_modelnames[mi_h_dog] 		= "progs/h_dog.mdl";
	cl_modelnames[mi_h_soldier] 	= "progs/h_guard.mdl";
	cl_modelnames[mi_h_enforcer] 	= "progs/h_mega.mdl";
	cl_modelnames[mi_h_knight] 		= "progs/h_knight.mdl";
	cl_modelnames[mi_h_hknight] 	= "progs/h_hellkn.mdl";
	cl_modelnames[mi_h_scrag] 		= "progs/h_wizard.mdl";
	cl_modelnames[mi_h_ogre] 		= "progs/h_ogre.mdl";
	cl_modelnames[mi_h_fiend] 		= "progs/h_demon.mdl";
	cl_modelnames[mi_h_vore] 		= "progs/h_shal.mdl";
	cl_modelnames[mi_h_shambler] 	= "progs/h_shams.mdl";
	cl_modelnames[mi_h_zombie] 		= "progs/h_zombie.mdl";

	for (i=0 ; i<NUM_MODELINDEX ; i++)
	{
		if (!cl_modelnames[i])
			Sys_Error ("cl_modelnames[%d] not initialized", i);
	}
}

qbool Monster_isDead (int modelindex, int frame)
{
	if (cl_ent_deadbodyfilter.integer == 2)
	{
		// Dying frames
		if ((modelindex == cl_modelindex[mi_fish] && frame >= 18 && frame <= 38) ||
		    (modelindex == cl_modelindex[mi_dog] && frame >= 8 && frame <= 25) ||
		    (modelindex == cl_modelindex[mi_soldier] && frame >= 8 && frame <= 28) ||
		    (modelindex == cl_modelindex[mi_enforcer] && frame >= 41 && frame <= 65) ||
		    (modelindex == cl_modelindex[mi_knight] && frame >= 76 && frame <= 96) ||
		    (modelindex == cl_modelindex[mi_hknight] && frame >= 42 && frame <= 62) ||
		    (modelindex == cl_modelindex[mi_scrag] && frame >= 46 && frame <= 53) ||
		    (modelindex == cl_modelindex[mi_ogre] && frame >= 112 && frame <= 135) ||
		    (modelindex == cl_modelindex[mi_fiend] && frame >= 45 && frame <= 53) ||
		    (modelindex == cl_modelindex[mi_vore] && frame >= 16 && frame <= 22) ||
		    (modelindex == cl_modelindex[mi_shambler] && frame >= 83 && frame <= 93) ||
		    (modelindex == cl_modelindex[mi_player] && frame >= 41 && frame <= 102))
			return true;
	}
	else
	{	// Final death frames
		if ((modelindex == cl_modelindex[mi_fish] && frame == 38) ||
		    (modelindex == cl_modelindex[mi_dog] && (frame == 16 || frame == 25)) ||
		    (modelindex == cl_modelindex[mi_soldier] && (frame == 17 || frame == 28)) ||
		    (modelindex == cl_modelindex[mi_enforcer] && (frame == 54 || frame == 65)) ||
		    (modelindex == cl_modelindex[mi_knight] && (frame == 85 || frame == 96)) ||
		    (modelindex == cl_modelindex[mi_hknight] && (frame == 53 || frame == 62)) ||
		    (modelindex == cl_modelindex[mi_scrag] && frame == 53) ||
		    (modelindex == cl_modelindex[mi_ogre] && (frame == 125 || frame == 135)) ||
		    (modelindex == cl_modelindex[mi_fiend] && frame == 53) ||
		    (modelindex == cl_modelindex[mi_vore] && frame == 22) ||
		    (modelindex == cl_modelindex[mi_shambler] && frame == 93) ||
		    (modelindex == cl_modelindex[mi_player] && (frame == 49 || frame == 60 || frame == 69 ||
			frame == 84 || frame == 93 || frame == 102)))
			return true;
	}

	return false;
}

qbool Model_isHead (int modelindex)
{
	if (modelindex == cl_modelindex[mi_h_dog] || modelindex == cl_modelindex[mi_h_soldier] ||
	    modelindex == cl_modelindex[mi_h_enforcer] || modelindex == cl_modelindex[mi_h_knight] ||
	    modelindex == cl_modelindex[mi_h_hknight] || modelindex == cl_modelindex[mi_h_scrag] ||
	    modelindex == cl_modelindex[mi_h_ogre] || modelindex == cl_modelindex[mi_h_fiend] ||
	    modelindex == cl_modelindex[mi_h_vore] || modelindex == cl_modelindex[mi_h_shambler] ||
	    modelindex == cl_modelindex[mi_h_zombie] || modelindex == cl_modelindex[mi_h_player])
		return true;

	return false;
}


/*
=================
Mod_SetExtraFlags -- johnfitz -- set up extra flags that aren't in the mdl
=================
*/



void Mod_SetExtraFlags (model_t *mod)
{

	// These flags current don't apply to a dedicated server
	// and r_list cvars are NOT registered for dedicated
	// Trying to reference then will crash us since they are NULL
	// So we leave ...
	if (isDedicated) return;

	if (!mod || !mod->name || mod->modelformat != mod_alias)
		return;

	// Only preserve non-listed flags.  Listed flags begin at MOD_NOLERP 1024
	mod->modelflags &= (MOD_NOLERP - 1) /* MOD_NOLERP = 1024 -1 = 1023 */;

	// Special flags
	if (COM_IsInList(mod->name, list_models_r_nolerp.string))			mod->modelflags |= MOD_NOLERP;
	if (COM_IsInList(mod->name, list_models_r_noshadow.string))			mod->modelflags |= MOD_NOSHADOW;
	if (COM_IsInList(mod->name, list_models_r_fullbright.string))		mod->modelflags |= MOD_FBRIGHTHACK;
	if (COM_IsInList(mod->name, list_models_r_additive.string))			mod->modelflags |= MOD_RENDERADDITIVE;
	if (COM_IsInList(mod->name, list_models_r_filter.string))			mod->modelflags |= MOD_RENDERFILTER;
	if (COM_IsInList(mod->name, list_models_r_fence_qpal255.string))	mod->modelflags |= MOD_RENDERFENCE;

}