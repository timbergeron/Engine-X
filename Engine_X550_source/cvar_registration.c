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
// cvar_registration.c -- in the future, make it look all pretty

#include "quakedef.h"

void Cvar_RegisterVariable (cvar_t *var,  void *function);
// Cvar Registration Sections

void Cvar_Registration_Server (void)
{
	Cvar_RegisterVariable (&sv_maxvelocity,			"Server maximum speed any entity may move."										);
	Cvar_RegisterVariable (&sv_gravity,				"Server level of gravity where 800 is Quake standard and lower values represent less gravity.");
	Cvar_RegisterVariable (&sv_friction,			"Rate of slowdown."																);
	Cvar_RegisterVariable (&sv_edgefriction,		"Rate of slowdown when approaching an edge."									);
	Cvar_RegisterVariable (&sv_stopspeed,			"Rate to come to a complete stop"												);
	Cvar_RegisterVariable (&sv_maxspeed,			"Maximum on-ground movement speed"												);
	Cvar_RegisterVariable (&sv_accelerate,			"Rate of which a player obtains maximum speed"									);
	Cvar_RegisterVariable (&sv_idealpitchscale,		"Amount of pitch (up/down) when player travelling on stairs/ramps when not using freelook");
	Cvar_RegisterVariable (&sv_aim,					"Poorly named server variable.  sv_aim 1 turns off aiming help.  Quake default 0.93 (a little help)");
	Cvar_RegisterVariable (&sv_nostep,				"Stops stepping entities (the monsters) from moving."							);
	Cvar_RegisterVariable (&sv_altnoclip,			"Not in Quake.  Uses 'better' noclip behavior where pitch is factored into movement.  Server-side.  'Noclip' allows a player to fly and walk through walls; often used in debugging maps and mods."); //johnfitz
	Cvar_RegisterVariable (&sv_bouncedownslopes,	"Grenades bounce down slopes.  Not original Quake behavior, grenades would get 'stuck' and vibrate (physics bug?).");
	Cvar_RegisterVariable (&sv_freezenonclients,	"Freeze the physics for non-players."											);

	

	Cvar_RegisterVariable (&host_fullpitch,			"Unlocks extending ability to look up and down beyond Quake norms.  May conflict with server."); 	// JPG 2.01
	Cvar_RegisterVariable (&sv_cullentities,		"Anti-wallhack capability.  Server tests entities against world before sending their data to clients.  Clients receive no data on unseen players.  Set to 2 for all entities to be screened.  Does not screen particles, does not test against submodels (doors, lifts).");	// Baker
	Cvar_RegisterVariable (&sv_cullentities_notify, "Developer testing."															);	// Baker

	Cvar_RegisterVariable (&sv_defaultmap,			"Map to fallback on if specified map does not exist.  Helps prevent dedicated servers missing custom maps from crashing."); //Baker 3.99b: R00k create a default map to load
	Cvar_RegisterVariable (&net_ipmasking,			"Mask player ip addresses to provide a small measure of privacy."				);

	Cvar_RegisterVariable (&sv_progs,				"Specify the 'progs.dat' to run instead of 'progs.dat', which allows more flexibility to re-use gamedir data.");
	Cvar_RegisterVariable (&external_ents,			"Load map entity files instead of native embedded entity list.  Allows CTF to run unmodified maps with flags or testing potential or map variations.");
	Cvar_RegisterVariable (&external_vis,			"Loads external visibility files.  Required by client and server.  Servers generally should not use these (except for vispatching releases) because client relies on vis data to render world and data is not transmitted over network.");

//	Cvar_RegisterVariable (&sv_gameplayfix_monster_lerp, "");
	Cvar_RegisterVariable (&sv_allcolors,			"The server will allow the use of colors 14/15, which are not typically available.");
#pragma message ("Quality assurance:  Client must be able to load ent files to read fog and skybox if changed.  And any future 'read from worldspawn' additions like hullsize or what not.") 
}

void Cvar_Registration_Client_Audio_MP3 (void)
{
	Cvar_RegisterVariable (&mp3_enabled,			"Enables or disables the playing of MP3 tracks if found.  Located in the <gamedir>/music folder with names like track00.mp3, track01.mp3, etc.");		// Baker 3.99e: we want this available even if -nocdaudio is used (for menu)
	Cvar_RegisterVariable (&mp3_volume,				"Volume level of mp3 music"														);		// And we do not want to lose these settings
}

void Cvar_Registration_Client_Sbar (void)
{
	Cvar_RegisterVariable (&scr_hud_teamscores, "");  // JPG - status bar teamscores
	Cvar_RegisterVariable (&scr_hud_timer,			"Displays the status bar time in deathmatch");  // JPG - status bar timer
	Cvar_RegisterVariable (&scr_hud_scoreboard_pings,	"Displays ping times in multiplayer scoreboard."								);  // JPG - ping times in the scoreboard
	Cvar_RegisterVariable (&scr_hud_center,			"Centers the status bar in single player."										);
	Cvar_RegisterVariable (&scr_hud_style,			"The type of heads-up display (or status bar) to show.  Style 0 is original."	);
	//	Cvar_RegisterVariable (&scr_hud_scoreboard_clean, "");

#pragma message ("Quality assurance:  Does status bar center in deathmatch ever?  I'm thinking no") 
}

void Cvar_Registration_Host_Common (void)
{
	Cvar_RegisterVariable (&session_registered,		"Whether or not registered Quake is present.  Although not read-only, changing this value has no meaningful effect except for not trigger entities encouraging the purchase of registered Quake.  Setting 'registered 1' will still not allow the use of custom maps.");
	Cvar_RegisterVariable (&session_cmdline,		"Command line used to start host.  Read-only.  Value returned by 'test2' command");   // Baker 3.99c: needed for test2 command
}


void Cvar_Registration_Client_Image (void)
{
/* Baker: Eliminated, set to best values 
#if SUPPORTS_LIBPNG
	Cvar_RegisterVariable (&png_compression_level, "");
#endif
#if SUPPORTS_LIBJPEG
	Cvar_RegisterVariable (&jpeg_compression_level, "");
#endif
*/
}

void Cvar_Registration_Client_Rendering_Options_2D (void)
{

	Cvar_RegisterVariable (&tex_picmip,				"This cvar is being considered for deletion.  Reason: Original purpose was poor video card performance and very small texture memory.");
	Cvar_RegisterVariable (&tex_picmip_allmodels,	"This cvar is being considered for deletion.  Reason: Original purpose was poor video card performance and very small texture memory.");
#if SUPPORTS_GL_DELETETEXTURES
	Cvar_RegisterVariable (&tex_free_on_newmap,		"Frees world textures on start of new map.");
#endif
	Cvar_RegisterVariable (&scr_con_alpha,			"This cvar is being considered for deletion.  Reason: Console alpha is rarely modified by the user.");
	Cvar_RegisterVariable (&scr_con_font_smooth,		"Enables font smoothing.  Works well for resolutions where width and/or height is not divisible by 8 (the size of a console character).");

	Cvar_RegisterVariable (&tex_scene_texfilter,	"Controls the texture minimum and maximum filters for rendering."				);

	Cvar_RegisterVariable (&external_textures, NULL);
//	Cvar_RegisterVariable (&gl_externaltextures_bmodels, NULL);

	Cvar_RegisterVariable (&scr_crosshair_alpha,	"Level of alpha for rendering the crosshair (0.00 to 1.00 range, where 0 is fully transparent and 1 is fully solid).");
	Cvar_RegisterVariable (&scr_crosshair_image,	"Crosshair image.  Loads from '<gamedir>/crosshairs' folder.  Formats supported:  PCX and TGA.");

	Cvar_RegisterVariable (&scr_crosshair,			"Draw the scr_crosshair."															);
	Cvar_RegisterVariable (&scr_crosshair_style,	"The style of the scr_crosshair.  Style 1 is using an external crosshair image if present.  Style 0 is classic '+' scr_crosshair.  Styles 2 through 6 are alternate built-in crosshairs.");
	Cvar_RegisterVariable (&scr_crosshair_color,	"Specifies crosshair color either as a palette number (0-255) or a space-delimited RGB color [which must be surrounded by quotes] such as \"255 128 0\" where 255 is full red, 128 is half-green and 0 is no blue.");
	Cvar_RegisterVariable (&scr_crosshair_size,		"Size of scr_crosshair.");
	Cvar_RegisterVariable (&scr_crosshair_offset_x,	"Offset crosshair horizontally (left/right) by specified amount.  Examples values are -5 or 5.");
	Cvar_RegisterVariable (&scr_crosshair_offset_y,	"Offset crosshair vertically (up/down) by specified amount.  Examples values are -5 or 5.");

	Cvar_RegisterVariable (&scr_con_font,			"Loads a specified external console font (a 'character set') from '<gamedir>/fonts'.  Formats supported:  PCX and TGA.");

//	Cvar_RegisterVariable (&gl_free_world_textures, NULL);//R00k

}

void Cvar_Registration_Client_Joystick (void)
{
	Cvar_RegisterVariable (&joy_name, NULL);
	Cvar_RegisterVariable (&joy_advanced, NULL);
	Cvar_RegisterVariable (&joy_advaxisx, NULL);
	Cvar_RegisterVariable (&joy_advaxisy, NULL);
	Cvar_RegisterVariable (&joy_advaxisz, NULL);
	Cvar_RegisterVariable (&joy_advaxisr, NULL);
	Cvar_RegisterVariable (&joy_advaxisu, NULL);
	Cvar_RegisterVariable (&joy_advaxisv, NULL);
	Cvar_RegisterVariable (&joy_forwardthreshold, NULL);
	Cvar_RegisterVariable (&joy_sidethreshold, NULL);
	Cvar_RegisterVariable (&joy_flythreshold, NULL);
	Cvar_RegisterVariable (&joy_pitchthreshold, NULL);
	Cvar_RegisterVariable (&joy_yawthreshold, NULL);
	Cvar_RegisterVariable (&joy_forwardsensitivity, NULL);
	Cvar_RegisterVariable (&joy_sidesensitivity, NULL);
	Cvar_RegisterVariable (&joy_flysensitivity, NULL);
	Cvar_RegisterVariable (&joy_pitchsensitivity, NULL);
	Cvar_RegisterVariable (&joy_yawsensitivity, NULL);
	Cvar_RegisterVariable (&joy_wwhack1, NULL);
	Cvar_RegisterVariable (&joy_wwhack2, NULL);
}

void Cvar_Registration_Client_Mouse_DirectInput (void)
{
	Cvar_RegisterVariable (&mouse_directinput,		"DirectInput mouse input.  On by default.  Mouse input not affected by Windows control panel.");
}

void Cvar_Registration_Client_Devices (void)
{
	Cvar_RegisterVariable (&in_joystick,			"Enable joystick.  Off by default."												);
	Cvar_RegisterVariable (&in_mouse,				"Enable mouse. On by default."													);
}

void Cvar_Registration_Host_Commands (void)
{
	Cvar_RegisterVariable (&session_confirmquit,	"Bypass 'quit' confirmation."													);
	Cvar_RegisterVariable (&session_quickstart,		"Bypass engine startup animation and start-up demos.  Set to 2 to always bypass start-up demos even on gamedir change.  Off by default.");

	Cvar_RegisterVariable (&game_kurok,				"Enables extended Kurok features."												);
	Cvar_RegisterVariable (&host_sub_missing_models,"Replaces missing external models (not missing world submodels) with a generic model included with the engine content.");

	Cvar_RegisterVariable (&host_clearmodels,		"Clears all models from cache upon every map load."								);

#pragma message ("Quality assurance:  host_sub_missing_models Missing model does not replace sprites?") 
#pragma message ("Quality assurance:  host_sub_missing_models What is impact of replacing a multimodel MD3?  Will engine blow up?") 
#pragma message ("Quality assurance:  host_clearmodels.  Is this necessary?  Implemented for gamedir switching, but not sure it is necessary or even does anything.") 
}

void Cvar_Registration_Client_Mouse (void)
{
	// mouse variables

#if ALTER_WINDOWS_MPARMS
	Cvar_RegisterVariable (&mouse_setparms,			"Enables Engine X to temporarily overrride Windows mouse parameters.  Not likely to be enabled in released builds of engine.");
#endif
}

void Cvar_Registration_Client_Keyboard (void)
{
	Cvar_RegisterVariable (&in_keypad,				"Allows separate binding of keypad keys."										);
	Cvar_RegisterVariable (&in_keymap,				"Not presently implemented.  To do."											); // &KEY_Keymap_f equivalent must be triggered to de-press the keys
#pragma message ("Quality assurance:  Not all engines have all the same keys.  There is a big risk of some other engine nuking some of your extended keys because it doesn't recognize it and therefore doesn't save it.")
}


void Cvar_Registration_Client_Rendering_Options_3D (void)
{
	Cvar_RegisterVariable (&light_overbright, "");
	Cvar_RegisterVariable (&r_subdivide_size, "");

	Cvar_RegisterVariable (&r_pass_lumas, "");
	Cvar_RegisterVariable (&light_lerplightstyle, "");
	
	
	Cvar_RegisterVariable (&r_brushmodels_with_world, "");

	
	Cvar_RegisterVariable (&tool_texturepointer, "");

	
	Cvar_RegisterVariable (&r_water_alpha,			"Level of water transparency to render."										);
	Cvar_RegisterVariable (&r_water_ripple,			"Water ripple effect."															);
	Cvar_RegisterVariable (&r_water_warp,			"Subtly alters the rendering field of view if submerged underwater."			);

	
	Cvar_RegisterVariable (&scene_novis,			"Ignore map visibility.  Slow."													);
	Cvar_RegisterVariable (&scene_lockpvs,			"Lock the PVS to the current frustum for engine rendering debugging."			);

	// Stages
	Cvar_RegisterVariable (&r_drawentities,			"Draw the entities."															);
	Cvar_RegisterVariable (&r_drawentities_alpha,	"Draw entities with transparency"												);
	Cvar_RegisterVariable (&r_drawmodels_alias,		"Draw alias models."															);
	Cvar_RegisterVariable (&r_drawmodels_sprite,	"Draw sprites."																	);
	Cvar_RegisterVariable (&r_drawmodels_brush,		"Draw brush models."															);
	Cvar_RegisterVariable (&r_nodrawentnum,			"Do not draw a specific entity (by entity num); if negative draw all entities except entity."	);
	Cvar_RegisterVariable (&r_drawparticles,		"Draw particles."																);
	Cvar_RegisterVariable (&r_shadows,				"Draw shadows.  Set to 2 for fully dark shadows."								);

	// Lighting Functions
	Cvar_RegisterVariable (&scene_dynamiclight,		"Update dynamic lighting."														);
//	Cvar_RegisterVariable (&r_fullbright,			"Render world without lightmaps."												);
	Cvar_RegisterVariable (&r_lightmap,				"Render world with only lightmaps."												);
	// r_lightmap 1 = expected, -1 is fullbright, 2 is fullbrightskins
	Cvar_RegisterVariable(&r_stains,				"Stain surfaces."																);   //qbism ftestain cvars
	Cvar_RegisterVariable(&r_stains_fadetime,		"Stain fade control in seconds between fade calculations."						);
	Cvar_RegisterVariable(&r_stains_fadeamount,		"Amount of alpha to reduce stains per fade calculation."						);

	Cvar_RegisterVariable (&tool_showbboxes,		"Server only (single player or listen server).  Draw boxes around entities."	);
	Cvar_RegisterVariable (&tool_showspawns,		"Server only (single player or listen server).  Draw only spawnpoints."			);


	Cvar_RegisterVariable (&r_speeds,				"Display rendering speed information."											);
//	Cvar_RegisterVariable (&r_fullbrightskins, "");
//	Cvar_RegisterVariable (&r_fastsky, "");
//	Cvar_RegisterVariable (&r_skycolor, "");

	Cvar_RegisterVariable (&r_skybox,				"Name of skybox to render"														);
	Cvar_RegisterVariable (&r_oldsky,				"Render oldsky");

	Cvar_RegisterVariable (&scene_farclip,			"Maximum distance to render (scene cutoff)."									);
//	Cvar_RegisterVariable (&r_cullsurfaces, "");

	// fenix@io.com: register new cvar for model interpolation
	Cvar_RegisterVariable (&scene_lerpmodels,		"Interpolate frames for animation smoothness."									);
	Cvar_RegisterVariable (&scene_lerpmove,			"Interpolate entity movement (particularly monsters and rotating brush models).");
	Cvar_RegisterVariable (&list_models_r_nolerp,	"List of models to not get animation lerping (smoothing).  Usually because it looks silly.");
	Cvar_RegisterVariable (&list_models_r_noshadow, "List of models that do not get shadows (like flames, that'd be crazy)."		);
	Cvar_RegisterVariable (&list_models_r_fullbright, "List of models that are rendered with full lighting (like flames or a lightning bolt beam).");
	Cvar_RegisterVariable (&list_models_r_additive, "List of models that render with additive blending."							);
	Cvar_RegisterVariable (&list_models_r_filter,	"List of models that render with filter blending."								);
	Cvar_RegisterVariable (&list_models_r_fence_qpal255, "List of models that render as 'fence' with palette color 255 representing the transparent color (GL_ALPHA_TEST). ");

	Cvar_RegisterVariable (&list_demos_hud_nodraw,	"Demos that do not display the status bar when playing."						);
	
	Cvar_RegisterVariable (&gl_finish,				"Engine developer rendering testing."											);
	Cvar_RegisterVariable (&gl_clear,				"Clear the color buffer at the start of every frame.  Default is -1, automatic determination.");

	Cvar_RegisterVariable (&gl_cull,				"Engine developer rendering testing."											);
	Cvar_RegisterVariable (&gl_smoothmodels,		"Engine developer rendering testing."											);
	Cvar_RegisterVariable (&gl_affinemodels,		"Engine developer rendering testing."											);

	Cvar_RegisterVariable (&scene_polyblend,		"Draw flashes on-screen rendering a fullscreen polygon to achieve the effect."	);
	Cvar_RegisterVariable (&light_flashblend,		"Draw light spheres to represent flashes instead of dynamic lighting."			);

	Cvar_RegisterVariable (&tex_lerp,				"Smooth textures when resizing and scaling?  Developer note:  I cannot tell the difference.  Need a test case?");

	Cvar_RegisterVariable (&cl_ent_nocolors,		"Do not color in the player skin.  Developer testing."							);
	Cvar_RegisterVariable (&external_lits,			"Load external colored lighting for standard Quake maps (BSP 29)."				);
	Cvar_RegisterVariable (&cl_ent_doubleeyes,		"Render eyes twice as big.  GLQuake legacy default value."						);
	Cvar_RegisterVariable (&r_vertex_blenddist,		"Vertex interpolation max distance."											);
	Cvar_RegisterVariable (&r_water_fog,			NULL																			);
	Cvar_RegisterVariable (&r_water_fog_density,	NULL																			);
	Cvar_RegisterVariable (&r_pass_detail,			"Render detail texture"															);
	Cvar_RegisterVariable (&r_pass_caustics,		"Render underwater caustics.  Liquid blends will not occur if this is on."		);
	Cvar_RegisterVariable (&r_viewmodel_ringalpha,	"Render ring effect as weapon translucency.  Ring shadow hue will not occur if this is on.");
	Cvar_RegisterVariable (&r_pass_lumas_brush,		"Render lumas on brush models."													);
	Cvar_RegisterVariable (&r_pass_lumas_alias,		"Render lumas on alias models."													);
	Cvar_RegisterVariable (&particle_blend,			"Render solid blocky particles."												);
	Cvar_RegisterVariable (&r_vertex_lights,		"Render lighting taking into account angles."									);

	Cvar_RegisterVariable (&qmb_explosions,			"Render QMB explosions."														);
	Cvar_RegisterVariable (&qmb_trails,				"Render QMB trails (rocket, grenade, scrag, blood, hellknight, vore)"			);
	Cvar_RegisterVariable (&qmb_spikes,				"Render QMB nails."																);
	Cvar_RegisterVariable (&qmb_gunshots,			"Render QMB gunshot effect"														);
	Cvar_RegisterVariable (&qmb_blood,				"Render QMB blood effect"														);
	Cvar_RegisterVariable (&qmb_telesplash,			"Render QMB teleporter effect"													);
	Cvar_RegisterVariable (&qmb_blobexplosions,		"Render QMB 'blob monster explosion' (spawns, tarbabies, whatever they are)"	);
	Cvar_RegisterVariable (&qmb_lavasplash,			"Render QMB lava splashes"														);
	Cvar_RegisterVariable (&qmb_inferno,			"This does not work"															);
	Cvar_RegisterVariable (&qmb_flames,				"Render QMB flames."															);
	Cvar_RegisterVariable (&qmb_lightning,			"Render QMB lightning effect."													);
	Cvar_RegisterVariable (&qmb_spiketrails,		"Render QMB spike trails (and bubbles underwater if shooting nails)."			);

	Cvar_RegisterVariable (&qmb_clipparticles,		"Render QMB clipping particles."												);
	Cvar_RegisterVariable (&qmb_bounceparticles,	"Render QMB bouncing particles."												);


	Cvar_RegisterVariable (&r_fog,					"If disabled, suppresses rendering of fog for testing"										);

//#pragma message ("Quality assurance: What is a QMB lava splash?")		When lavaball falls
#pragma message ("Quality assurance: Waterfog is probably broken.  Fix?")
#pragma message ("Quality assurance: Reminder qmb_inferno does not work.  Maybe even tornado does not work.")

}

void Cvar_Registration_Client_Chase (void)
{
	Cvar_RegisterVariable (&chase_active,			"Turns on chase camera.  chase_active 1 targets view for where player is looking.  chase_active 2 uses a fixed relative location to player.");
	
	Cvar_RegisterVariable (&chase_back,				"Camera distance behind player."												);
	Cvar_RegisterVariable (&chase_up,				"Camera distance above player."													);
	Cvar_RegisterVariable (&chase_right,			"Camera distance to the right of player."										);
	Cvar_RegisterVariable (&chase_pitch,			"Camera pitch angle (up/down).  Only for chase_active 2."						);
	Cvar_RegisterVariable (&chase_yaw,				"Camera yaw angle (left/right).  Only for chase_active 2."						);
	Cvar_RegisterVariable (&chase_roll,				"Camera roll angle (tilt).  Only for chase_active 2."							);

}

void Cvar_Registration_Client_Screen (void)
{
//	Cvar_RegisterVariable (&default_fov,			"Considered for deletion."														);
	Cvar_RegisterVariable (&scene_fov_x,			"Field of view in degrees for width of the screen."								);
	Cvar_RegisterVariable (&scene_fov_y,			"Field of view in degrees for height of screen.  Not typically used."			);
	Cvar_RegisterVariable (&scene_fov_scale,		"Adjust field of view for screen aspect ratio specified by scene_fov_scaleaspect.");
	Cvar_RegisterVariable (&scene_fov_scaleaspect,	"Screen aspect ratio to use for field of view adjustment, if feature is on."	);
	Cvar_RegisterVariable (&scene_viewsize,			"Viewsize of screen where 30-100 percent, with special values of 110 and 120"	);

	Cvar_RegisterVariable (&scr_con_size,			"Size of the console when game is active.  Use CTRL plus UP/DOWN to adjust console vertical size."	);	// by joe
	Cvar_RegisterVariable (&scr_con_speed,			"Speed the console raises or lowers.  Quake defaulted to a slow 300 but we are using 99999.");
	Cvar_RegisterVariable (&scr_victory_printspeed,		"Speed the 'victory text' appears on-screen at intermission after defeating a boss.");

	Cvar_RegisterVariable (&scr_hud_extrabuffer,	"Draws the status bar (HUD) an extra frame to attempt to eliminate drawing issues.  Note: This console variable is egregiously misnamed.  It has nothing to do with 3D rendering or performance, just drawing the HUD for 3 frames when it changes instead of 2.");	

	Cvar_RegisterVariable (&scr_show_ram,			"Whether or not to show the RAM icon, if applicable.  However, it only applies to software Quake and hence not this engine.");
	Cvar_RegisterVariable (&scr_show_turtle,		"Whether or not to show the turtle icon."										);
	Cvar_RegisterVariable (&scr_show_pause,			"Whether or not to draw 'PAUSED' on the screen if paused."						);
	Cvar_RegisterVariable (&scr_centerprint_time,	"Amount of time to display a center print message (i.e. 'This hall selects easy').");
	Cvar_RegisterVariable (&scr_show_speed,			"Displays a speed indicator on-screen."											);

	

	Cvar_RegisterVariable (&scr_show_stats,			"Considered for deletion."														);
	Cvar_RegisterVariable (&scr_show_stats_small,	"Considered for deletion."														);
	Cvar_RegisterVariable (&scr_show_stats_length,	"Considered for deletion."														);

	Cvar_RegisterVariable (&screenshot_format,		"Preferred screenshot file format.  Falls back to available file formats if preferred is not available.  JPEG is preferred format.");


	Cvar_RegisterVariable (&scr_show_fps,			"Displays your frames per second on-screen."									);
	Cvar_RegisterVariable (&scr_show_coords,		"Displays your world coordinates on-screen.  Mostly development interest."		);
	Cvar_RegisterVariable (&scr_show_angles,		"Displays your world angles on-screen.  Mostly of development interest."		);
	Cvar_RegisterVariable (&scr_show_location,		"Displays location from a definition file (like start.loc or mymap.loc)."		);
	Cvar_RegisterVariable (&scr_show_cheats,		"If running a local game (single player as an example), shows what 'cheats' are active such a 'god' or 'noclip'."		);
	Cvar_RegisterVariable (&scr_show_skill,			"If dead or using the scoreboard, shows the current skill level."				);

#pragma message ("Quality assurance: Need an option like pq_timer 2 or something to show the game clock in single player.  And/or something to show monster/kills/secrets always.")

}

void Cvar_Registration_Client_ViewBlends (void)
{
	

	Cvar_RegisterVariable (&vb_bonusflash,			"Limit effect of bonus flash (flash when picking up item).  Range 0 to 1, where 0 is none.");
	Cvar_RegisterVariable (&vb_contentblend,		"Limit effect of content blend (water/lava/slime).  Range 0 to 1, where 0 is none.");
	Cvar_RegisterVariable (&vb_damagecshift,		"Limit effect of damage flash (red flash when hurt).  Range 0 to 1, where 0 is none.");
	Cvar_RegisterVariable (&vb_quadcshift,			"Limit effect of quad blend (blue screen blend).  Range 0 to 1, where 0 is none.");
	Cvar_RegisterVariable (&vb_suitcshift,			"Limit effect of suit blend (green screen blend).  Range 0 to 1, where 0 is none.");
	Cvar_RegisterVariable (&vb_ringcshift,			"Limit effect of ring blend (dark hue).  Range 0 to 1, where 0 is none."		);
	Cvar_RegisterVariable (&vb_pentcshift,			"Limit effect of pent blend (red screen blend).  Range 0 to 1, where 0 is none.");

//	Cvar_RegisterVariable (&v_dlightcshift, "");		// No longer supporting dlightcshift.  Joe didn't use.

	Cvar_RegisterVariable (&vid_brightness_gamma,	"Controls screen brightness."													);
	Cvar_RegisterVariable (&vid_brightness_contrast,"Controls screen contrast."														);

#pragma message ("Quality assurance: May need to add back in VLightBlend for when inside bubble for the GLQuake look for gl_flashblend 2")
}

void Cvar_Registration_Client_ViewModel (void)
{

	Cvar_RegisterVariable (&v_gunkick,				"Turns on WinQuake style gun kick when firing (subtle screen shake).  Nice effect for rapid fire weapons like nail guns. Set to 2 for a more subtle effect.");
	Cvar_RegisterVariable (&r_viewmodel_pitch,		"Controls the gun height on-screen, used by Kurok for weapon change transition (old gun lowers, then new gun rises).");

	Cvar_RegisterVariable (&r_drawviewmodel,		"Draw the view model.");
#pragma message ("Quality assurance: Remove the frivolous ability to use this cvar to set the alpha except maybe if developing offline.")
	Cvar_RegisterVariable (&r_viewmodel_size,		"Can lower the viewmodel position on-screen a bit.");
	Cvar_RegisterVariable (&r_viewmodel_offset,		"Extra view model positioning.");
	Cvar_RegisterVariable (&r_viewmodel_hackpos,	"Whether to adjust the gun position based on the HUD display type (cl_hudstyle and viewsize).");
	Cvar_RegisterVariable (&r_viewmodel_fovscale,	"Whether to scale the gun to appear the same regardless of field of view (FOV).");		// Baker: Should we make this cvar work?


}

void Cvar_Registration_Host_Security (void)
{
	Cvar_RegisterVariable (&sv_signon_plus,		"Not implemented.");
}


void Cvar_Registration_Host (void)
{
	Cvar_RegisterVariable (&host_framerate,			"Specifies a framerate for the host.");
	Cvar_RegisterVariable (&host_speeds,			"Prints host timing statistics to the console.");
	Cvar_RegisterVariable (&host_timescale,			"Speeds up or slows down game play."); //johnfitz
	Cvar_RegisterVariable (&host_sleep,				"CPU saver.  Not in original Quake.  Client/server sleeps a bit between frames if exceeding frames per second rate.  Not recommended for network play.");
	Cvar_RegisterVariable (&sys_ticrate,			"Length of dedicated server frame in seconds.  Will sleep a few milliseconds between frames.");
	Cvar_RegisterVariable (&serverprofile,			"Prints some server frame statistics to the console.");

	Cvar_RegisterVariable (&pr_fraglimit,			"Intended to control maximum score before level is won.  Controlled by mod.  Use may vary.");
	Cvar_RegisterVariable (&pr_timelimit,			"Intended to control maximum time limit in minutes of a pr_deathmatch.  Controlled by mod.  Use may vary.");
	Cvar_RegisterVariable (&pr_teamplay,			"Intended to specify team play rules.  Controlled by mod.  Use may vary.");
	Cvar_RegisterVariable (&pr_samelevel,			"In Quake, setting samelevel means same level plays over and over.  Controlled by mod.  Use may vary.");
	Cvar_RegisterVariable (&pr_noexit,				"In Quake, controls whether exit teleporters work or kill player.  Controlled by mod.  Use may vary.");
	
	Cvar_RegisterVariable (&developer,				"Turns on developer messages.  Use dev command to limit to category.			");
	Cvar_RegisterVariable (&developer_filter,		"Specifies limiting developer messages to category.  Use dev command (easier).	");
	Cvar_RegisterVariable (&pr_deathmatch,			"Intended use to control type of deathmatch play.  Controlled by mod.");
	Cvar_RegisterVariable (&pr_coop,				"Intended use to control type cooperative play.  Controlled by mod.");
	Cvar_RegisterVariable (&pr_skill,				"Intended use to control monsters level of difficulty in single player/pr_coop.  Controlled by mod.");
	Cvar_RegisterVariable (&pr_temp1,				"Extra server and game mod control variable.");
	
	Cvar_RegisterVariable (&sv_pausable,			"Whether or not gameplay can be paused.");


//	Cvar_RegisterVariable (&proquake, "");		// JPG - added this so QuakeC can find it
#pragma message ("Quality assurance: Was removing the ProQuake cvar detrimental to ProQuake QCCX in QuakeC?")
	Cvar_RegisterVariable (&sv_chat_rate,			"Intended as an engine fix to limit talkative/disruptive player from being annoying (chat spamming).");	// JPG - spam protection
	Cvar_RegisterVariable (&sv_chat_grace,			"Intended as an engine fix to limit talkative/disruptive player from being annoying (chat spamming).");	// JPG - spam protection
	Cvar_RegisterVariable (&sv_chat_connectmute,	"Number of seconds a player must be connected before they can talk.  Allows ban file a few seconds to take effect.  Prevents repeated connecting/spamming a line of text or two/ban file kick cycle abuse.");	// Baker 3.99g: from Rook, protection against repeatedly connecting + spamming
	Cvar_RegisterVariable (&sv_chat_changemute,		"Seconds before player performing name change or color change may talk.  Feature intended to curb impersonation and quick color change and back team messages.");	// JPG 3.20 - temporary muting
	Cvar_RegisterVariable (&scr_con_chatlog_playerslot,			"Prints server edict number for players to console log so administrators can validate impersonators in a console log.");	// JPG 3.11 - feature request from Slot Zero
	Cvar_RegisterVariable (&scr_con_chatlog_dequake,				"Translates whitespace characters into real characters and colored Quake letters into normal letters for console output.");	// JPG 1.05 - translate dedicated console output to plain text
	Cvar_RegisterVariable (&host_maxfps,			"Maximum host frames per second for both client and server."					);		// JPG 1.05
#if VARIABLE_EDICTS_AND_ENTITY_SIZE
	Cvar_RegisterVariable (&host_maxedicts,			NULL					);		// JPG 1.05
	Cvar_RegisterVariable (&host_maxedicts_pad,		NULL					);		// JPG 1.05
#endif
	Cvar_RegisterVariable (&scr_con_chatlog,		"Server console saves player messages to the console log."						);	// JPG 3.20 - log player binds


}

void Cvar_Registration_Client_Menu (void)
{
	Cvar_RegisterVariable (&scr_menu_center,		"Centers the menu based in part on video mode resolution."						);
	Cvar_RegisterVariable (&scr_menu_scale,			"Scales the menu based in part on video mode resolution."						);
}

void Cvar_Registration_Host_Network (void)
{
	Cvar_RegisterVariable (&net_messagetimeout,		"Related to dropping inactive connections.");
	Cvar_RegisterVariable (&net_maxclientsperip,	"Maximum number of clients with same ip address.  Helps prevent vote cheating by connecting multiple clients."	);								
	Cvar_RegisterVariable (&net_connecttimeout,		"Server limit to timed out clients. Defaults to 10 seconds."					);	// JPG 2.01 - qkick/qflood protection
	Cvar_RegisterVariable (&sv_hostname,			"'Name' of the server, if hosting a dedicated or listen server."				);																				
	Cvar_RegisterVariable (&pq_password,			"Password protects a server.  Client uses this to authenticate."				);			// JPG 3.00 - password protection
	Cvar_RegisterVariable (&rcon_password,			"Remote server password for rcon access."										);			// JPG 3.00 - rcon password
	Cvar_RegisterVariable (&rcon_server,			"Remote server IP address.  Used to remotely administer a server as a client."	); // JPG 3.00 - rcon server

#pragma message ("Quality assurance: rcon_password Security risk.  Accidentally giving a server run by unethical admin your rcon password.")
#pragma message ("Quality assurance: Rcon brute force protection.")  
#pragma message ("Quality assurance: Implement maxclientsperip.")  
}

#if SUPPORTS_AVI_CAPTURE
void Cvar_Registration_Client_Movie (void)
{
	Cvar_RegisterVariable (&capture_codec,			"The 4C Video codec code for AVI movie capture encoding (DIVX, XVID, etc.)"		);
	Cvar_RegisterVariable (&capture_fps,			"Frame rate for AVI movie capture.  Defaults to 30 frames per second."			);
	Cvar_RegisterVariable (&capture_console,		"When active, frames are not written to AVI movie capture if the console is onscreen.");
	Cvar_RegisterVariable (&capture_hack,			"Relates to improving audio capture during AVI movie capture."					);
	Cvar_RegisterVariable (&capture_mp3, NULL);
	Cvar_RegisterVariable (&capture_mp3_kbps, NULL);
}
#endif

void Cvar_Registration_Host_Keys (void)
{
#ifdef SUPPORTS_GLVIDEO_MODESWITCH
	Cvar_RegisterVariable (&vid_altenter_toggle,	"The ALT-ENTER key will attempt to switch between fullscreen and windowed mode.");
#endif

#pragma message ("Quality Assurance: Do a cvarlist in dedicated and see what client cvars get registered")
#pragma message ("Quality Assurance: Mark cvars in future so dedicated server ignores/warns when client cvar is attempted to register")
}

void Cvar_Registration_Extension_Nehahra (void)
{

	// Nehahra uses these to pass data around cutscene demos
	Cvar_RegisterVariable (&pr_neh_nehx00,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx01,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx02,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx03,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx04,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx05,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx06,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx07,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx08,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx09,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx10,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx11,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx12,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx13,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx14,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx15,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx16,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx17,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx18,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_nehx19,			"Nehahra server and demo variable."												);
	Cvar_RegisterVariable (&pr_neh_cutscene,		"Nehahra server and demo variable."												);

#pragma message ("Quality Assurance: Test Nehahra and see if skybox changes work ok still. Prescreen_r_skybox doesn't look right")

}
void Cvar_Registration_Client_Main (void)
{
	
	Cvar_RegisterVariable (&cl_net_name,			"Your name, not intended to be directly manipulated.  Use 'name' instead."		);
	Cvar_RegisterVariable (&cl_net_color,			"Your color, not intended to be directly manipulated.  Use 'color' instead."	);

	Cvar_RegisterVariable (&cl_print_shownet,		"Developer.  Prints network messages to console"								);
	Cvar_RegisterVariable (&developer_show_stufftext, "Developer. Prints stufftxt commands from server/demo to console."			);

	Cvar_RegisterVariable (&cl_ent_nolerp,			"Rarely used. Client doesn't smooth (lerp or interpolate) movement of entities.");
	Cvar_RegisterVariable (&cl_ent_truelightning,	"Lightning bolt from thunderbolt attack is drawn where you are aiming instead of where lightning bolt was last updated.  Smoother looking and perhaps makes aiming easier for some users, but does not actually represent server lightning location.  Some users are distracted on an online connection with higher latency as to the drawing of the bolt.  It is an optional visual smoothing enhancement similar to movement smoothing, animation smoothing, etc.  Some feel use of this feature and the untrueness of the in-game representation makes aim worse.");	

	Cvar_RegisterVariable (&particle_explosiontype,	"Type of explosion effect."														);

	Cvar_RegisterVariable (&light_muzzleflash,		"Renders a flash when weapon is fired."											);										
	Cvar_RegisterVariable (&light_powerups,			"Renders a light around a player with a powerup."								);
	Cvar_RegisterVariable (&light_explosions,		"Color of explosion light."														);
	Cvar_RegisterVariable (&light_rockets,			"Light effect around a rocket."													);
	Cvar_RegisterVariable (&light_explosions_color,	"The color of explosions (rocket, grenade, etc.)."								);
	Cvar_RegisterVariable (&light_rockets_color,	"The color of a glow around a rocket, if rocket lights are on."					);
	
	Cvar_RegisterVariable (&particle_rockettrail,	"Rocket trail type.  0 is none."												);
	Cvar_RegisterVariable (&particle_grenadetrail,	"Grenade trail type.  0 is none."												);

	Cvar_RegisterVariable (&demospeed,			"Demo playback speed."															);
	Cvar_RegisterVariable (&demorewind,			"Demo playback is reversed (rewinded)."											);
	Cvar_RegisterVariable (&cl_ent_rotate_items_bob,"Items that rotate around will also bob up/down (like Quake III).  Typically weapons/armor.");

	Cvar_RegisterVariable (&cl_ent_deadbodyfilter,	"'1' = Deadbodies will not be rendered.  '2' = Any death animation frame is not renderered.");
	Cvar_RegisterVariable (&cl_ent_gibfilter,		"Gibs are not rendered."														);
	Cvar_RegisterVariable (&gl_loadq3models, 		"Load the Q3 player triple model hack");
	Cvar_RegisterVariable (&cl_ent_disable_blood,	"Presentation of the game is made more tame."									);
	Cvar_RegisterVariable (&user_effectslevel,		"Controls effects levels.  Determined by the gamedir mod."						);



	// JPG - added these for %r formatting
	Cvar_RegisterVariable (&msg_needrl,				"Formatting string for multiplayer team messages (%r)."								);
	Cvar_RegisterVariable (&msg_haverl,				"Formatting string for multiplayer team messages (%r)."								);
	Cvar_RegisterVariable (&msg_needrox,			"Formatting string for multiplayer team messages (%r)."								);

	// JPG - added these for %p formatting
	Cvar_RegisterVariable (&msg_quad,				"Formatting string for multiplayer team messages (%p)."								);
	Cvar_RegisterVariable (&msg_pent,				"Formatting string for multiplayer team messages (%p)."								);
	Cvar_RegisterVariable (&msg_ring,				"Formatting string for multiplayer team messages (%p)."								);

	// JPG 3.00 - %w formatting
	Cvar_RegisterVariable (&msg_weapons,			"Formatting string for multiplayer team messages (%w)."								);
	Cvar_RegisterVariable (&msg_noweapons,			"Formatting string for multiplayer team messages (%w)."								);

	// JPG 1.05 - added this for +jump -> +moveup translation
	Cvar_RegisterVariable (&in_smartjump,			"The jump key does 'moveup' in water instead of jump.  The '+moveup' is faster for vertical swimming than '+jump' (Quake bug or design oversight?) and this feature increases the ease of maximized swimming speed, probably resulting in more fairness those less serious in their play.");

	// JPG 3.02 - added this by request
	Cvar_RegisterVariable (&v_pq_smoothcam,			"Smooths the view of the player by lerping the view angles, particularly noticeable with high latency connections.");

#if HTTP_DOWNLOAD
	Cvar_RegisterVariable (&cl_web_download,		"Enables client to attempt to download missing game models/sounds when connected to a server.  Download location specified with cl_web_download_url.");	
	Cvar_RegisterVariable (&cl_web_download_url,	"Specifies website a client will attempt to download missing files if connected to a server.");
#endif
}

void Cvar_Registration_Client_Input (void)
{

	Cvar_RegisterVariable (&cl_speed_up,			"Vertical movement speed (while swimming or flying)."							);
	Cvar_RegisterVariable (&cl_speed_forward,		"Forward movement speed [wanted] (server controls speed.  sv_maxspeed, ...)."	);					
	Cvar_RegisterVariable (&cl_speed_back,			"Backward movement speed [wanted] (server controls speed, sv_maxspeed, ...)."	);
	Cvar_RegisterVariable (&cl_speed_side,			"Max requested left/right movement speed when strafing."						);

	Cvar_RegisterVariable (&keyboard_speed_angle_multiplier,	"Keyboard typically: how much +speed multiplies keyboard turning speed"			);
	Cvar_RegisterVariable (&keyboard_speed_move_multiplier,		"Keyboard typically: how much +speed multiplies keyboard movement speed."		);
	Cvar_RegisterVariable (&keyboard_speed_yaw,		"Keyboard typically: yaw (left/right) looking speed."							);
	Cvar_RegisterVariable (&keyboard_speed_pitch,	"Keyboard typically: pitch (up/down) looking speed."							);
	
#pragma message ("Quality assurance: Scale yaw and pitch for FOV 90 like done for mouse input.  Maybe at some point do joystick too for completeness")


	Cvar_RegisterVariable (&in_lookspring,			"Returns view to level with ground if moving (pitch up/down) and not freelook (i.e. not using mouse/joystick look)."			);
	Cvar_RegisterVariable (&in_lookstrafe,			"Rare.  Weird.  If strafing, mouse/joystick for movement instead of turning."	);

	Cvar_RegisterVariable (&in_sensitivity,			"Mouse movement speed."															);
	Cvar_GetExternal      (&in_sensitivity);

	Cvar_RegisterVariable (&in_invert_pitch,		"Mouse pitch (up/down) are reversed."											);
	Cvar_RegisterVariable (&in_fovscale,	"Scales (down) mouse speed based on field of view"								);
	Cvar_RegisterVariable (&in_freelook,			"Full time mouse look requested."												);


	Cvar_RegisterVariable (&mouse_speed_pitch,		"Mouse pitch (up/down angles) looking speed multiplier."						);
	Cvar_RegisterVariable (&mouse_speed_yaw,		"Mouse yaw (left/right angles) looking speed multiplier."						);
	Cvar_RegisterVariable (&mouse_lookstrafe_speed_forward,	"Rare. Mouse forward movement multiplier if not using mouse look."				);
	Cvar_RegisterVariable (&mouse_lookstrafe_speed_side, "Rare, Mouse side movement multiplier if not using mouse look."					);
	Cvar_RegisterVariable (&in_filter,				"Interpolates mouse input, which may considerably jerky mouse behavior.");
	Cvar_RegisterVariable (&in_accel,				"Mouse acceleration factor.  Not same as sensitivity.  Mouse acceleration magnifies effect of larger mouse movements.");


#pragma message ("Quality assurance: At some point, you need to decide to keep or kill m_accel")
	Cvar_RegisterVariable (&cl_net_lag,					"Adds synthetic lag to increase your ping for fairness."						);  // JPG - synthetic lag

}

void Cvar_Registration_Server_PR (void)
{
	Cvar_RegisterVariable (&pr_nomonsters,			"Not used in original Quake.  Server and QuakeC variable available for game modifications.");
	Cvar_RegisterVariable (&pr_gamecfg,				"Server and QuakeC variable available for game modifications.  Not used in original Quake.");
	Cvar_RegisterVariable (&pr_scratch1,			"Server and QuakeC variable available for game modifications.  Not used in original Quake.");
	Cvar_RegisterVariable (&pr_scratch2,			"Server and QuakeC variable available for game modifications.  Not used in original Quake.");
	Cvar_RegisterVariable (&pr_scratch3,			"Server and QuakeC variable available for game modifications.  Not used in original Quake.");
	Cvar_RegisterVariable (&pr_scratch4,			"Server and QuakeC variable available for game modifications.  Not used in original Quake.");
	Cvar_RegisterVariable (&pr_savedgamecfg,		"Server and QuakeC variable available for game modifications.  Saved to file at configuration save.  Not used in original Quake.");
	Cvar_RegisterVariable (&pr_saved1,				"Server and QuakeC variable available for game modifications.  Saved to file at configuration save.  Not used in original Quake.");
	Cvar_RegisterVariable (&pr_saved2,				"Server and QuakeC variable available for game modifications.  Saved to file at configuration save.  Not used in original Quake.");
	Cvar_RegisterVariable (&pr_saved3,				"Server and QuakeC variable available for game modifications.  Saved to file at configuration save.  Not used in original Quake.");
	Cvar_RegisterVariable (&pr_saved4,				"Server and QuakeC variable available for game modifications.  Saved to file at configuration save.  Not used in original Quake.");
}

void Cvar_Registration_Client_Sound (void)
{

	Cvar_RegisterVariable (&snd_nosound,			"Disables the playing sounds."													);
	Cvar_RegisterVariable (&snd_volume,				"The level of loudness sound effects are played.  Does not control background music.");
	Cvar_RegisterVariable (&snd_precache,			"Rarely used.  Loads sounds before they are used."								);
	Cvar_RegisterVariable (&snd_loadas8bit,			"Rarely used.  Loads sound as 8-bit."											);
	Cvar_RegisterVariable (&snd_ambient_level,		"Controls water, lava, sky ambient sound effects snd_volume."						);
	Cvar_RegisterVariable (&snd_ambient_fade,		"Controls water, lava, sky ambient sound fade distance."						);
	Cvar_RegisterVariable (&snd_noextraupdate,		"Disables extra sound updates that prevent sound from skipping at low framerates.");
	Cvar_RegisterVariable (&snd_show,				"Prints sound info to console."													);
	Cvar_RegisterVariable (&snd_mixahead,			"Rarely changed. Seconds to mix ahead sound updates."							);

#pragma message ("Quality assurance: Tie volume to mp3volume somehow")
}



//void Cvar_Registration_Client_System (void)
//{
//	Cvar_RegisterVariable (&sys_highpriority, ""); //  OnChange_sys_highpriority);
//
//}

void Cvar_Registration_Mixed_Console (void)	// Console isn't 100% client, like con_printf ... so ....
{
	Cvar_RegisterVariable (&scr_notify_time,		"Number of seconds to display 'notify text' onscreen.  'Notify text' are messages that print in the top left corner when console is closed like 'You got the nails' or 'Player was eaten by a Shambler'.");
	Cvar_RegisterVariable (&scr_notify_lines,		"Number of lines of 'notify text' onscreen.  'Notify text' are messages that print in the top left corner when console is closed like 'You got the nails' or 'Player was eaten by a Shambler'.");
	Cvar_RegisterVariable (&scr_notify_chatsound,	"Don't play a sound [typically a beep] when a new 'player message' is displayed (i.e. multiplayer talk beep when someone types a message.  Sometimes used for recording an AVI movie (like capturedemo command) or to better ignore a player in a multiplayer game that won't stop talking.");//R00k
	Cvar_RegisterVariable (&scr_centerprint_log,	"Centerprinted messages like 'This hall selects easy level of difficulty' also print to the console for convenience (i.e. so you don't forget what it said).");
	Cvar_RegisterVariable (&scr_centerprint_nodraw,	"Do not display centerprinted messages.  Useful when recording an AVI movie using capturedemo or similar commands."); // Rook

	Cvar_RegisterVariable (&scr_con_filter,			"Makes 'you got ...' messages temporary so the next console message bumps it so the console is not filled with unimportant messages.  Off by default.");	// JPG 1.05 - make "you got" messages temporary
	Cvar_RegisterVariable (&scr_con_chatlog_timestamp,	"Add the time to formatting strings for multiplayer team messages if message does not contain time.");	// JPG 1.05 - timestamp player binds during a match
	Cvar_RegisterVariable (&scr_con_chatlog_removecr,	"Removes carriage returns from console print.  Defaults to on.");	// JPG 3.20 - timestamp player binds during a match

}

void Cvar_Registration_Client_Video (void)
{
#ifdef SUPPORTS_GLVIDEO_MODESWITCH // Not OSX build at the moment
	Cvar_RegisterVariable (&vid_fullscreen,			"Sets requested video mode to fullscreen (opposite is requesting windowed).  The 'video_restart' command initiates a video mode change request."); 
	Cvar_RegisterVariable (&vid_width,				"Sets requested video mode width. The 'video_restart' command initiates a video mode change request.");
	Cvar_RegisterVariable (&vid_height,				"Sets requested video mode height.  The 'video_restart' command initiates a video mode change request.");; 
	Cvar_RegisterVariable (&vid_bpp,				"Sets requested video mode bits per pixel (color depth), 16 and 32 being recognized.  This is the color depth.  The 'video_restart' command initiates a video mode change request."); 
	Cvar_RegisterVariable (&vid_displayfrequency,	"Sets requested video mode refresh rate in Hz (i.e. '60' or '72' etc).  The 'video_restart' command initiates a video mode change request."); 
#endif

	Cvar_RegisterVariable (&scr_con_scale,			"Controls sizing of 2D graphics (console, sbar).  Default is '-1' (Automatic)." ); 
	Cvar_RegisterVariable (&_windowed_mouse,		"Releases mouse to operating system if in windowed mode."						);
	Cvar_RegisterVariable (&vid_brightness_method,	"Brightness: 1 hardware, -1 slow polyblend, 0 hardware for only fullscreen."	);
	Cvar_RegisterVariable (&vid_vsync,				"Limits framerate to display refresh.  Reduces tearing, lowers framerate."		);

	Cvar_RegisterVariable (&gl_polygonoffset_factor, "Z-fighting related OpenGL parameter utilized by the OpenGL version of this engine to eliminate/reduce flickering where the world model and a world submodel have a shared surface in rendering.  The unmoving map is 'the world' and the sometimes moving sub-components of the map are 'submodels'.");
	Cvar_RegisterVariable (&gl_polygonoffset_offset, "Z-fighting related OpenGL parameter utilized by the OpenGL version of this engine to eliminate/reduce flickering where the world model and a world submodel have a shared surface in rendering.  The unmoving map is 'the world' and the sometimes moving sub-components of the map are 'submodels'.");

#ifdef MACOSX
    Cvar_RegisterVariable (&vid_overbright, NULL);
 
    Cvar_RegisterVariable (&gl_anisotropic, NULL);
    Cvar_RegisterVariable (&gl_fsaa, NULL);
    Cvar_RegisterVariable (&gl_truform, NULL);
#endif


#pragma message ("Quality assurance:  The edicts command sometimes prints backslashes like for WAD entries if a Windows path and these render as escape sequences.  Are there other places where this is a problem?")
}

void Cvar_Registration_Client_View (void)
{
	Cvar_RegisterVariable (&v_centermove,			"Amount of movement before view centering occurs (if freelooking of all types are disabled)");
	Cvar_RegisterVariable (&v_centerspeed,			"Speed that view centering occurs (if freelooking of all types are disabled)");
	Cvar_RegisterVariable (&v_smoothstairs,			"Smooths stair movement so is more like a ramp than stairs.  Off in this engine by default.  Feature not present (to this extent) in original Quake.  'Smooth stairs' makes environment less perceptible (i.e. may not realize on stairs at all unless looking down).");

	Cvar_RegisterVariable (&v_iyaw_cycle,			"Idling yaw (left/right) speed.  In Quake, only the intermission normally uses idling (subtle shifting of the view.");
	Cvar_RegisterVariable (&v_iroll_cycle,			"Idling roll (view tilt) speed.  In Quake, only the intermission normally uses idling (subtle shifting of the view.");
	Cvar_RegisterVariable (&v_ipitch_cycle,			"Idling pitch (up/down) speed.  In Quake, only the intermission normally uses idling (subtle shifting of the view.");
	Cvar_RegisterVariable (&v_iyaw_level,			"Idling yaw (left/right) amount.  In Quake, only the intermission normally uses idling (subtle shifting of the view.");
	Cvar_RegisterVariable (&v_iroll_level,			"Idling roll (view tilt) amount.  In Quake, only the intermission normally uses idling (subtle shifting of the view.");
	Cvar_RegisterVariable (&v_ipitch_level,			"Idling pitch (up/down) amount.  In Quake, only the intermission normally uses idling (subtle shifting of the view.");
	Cvar_RegisterVariable (&v_idlescale,			"Enables or disables idling.  Represents a percent of the other v_idling console variables.");

	Cvar_RegisterVariable (&v_rollspeed,			"The speed of accumulating tilt when strafing (moving side to side)."			);
	Cvar_RegisterVariable (&v_rollangle,			"The angles of tilt experienced when strafing (moving side to side)."			);
	Cvar_RegisterVariable (&v_bob,					"The amount of bobbing when running."											);
	Cvar_RegisterVariable (&v_bobcycle,				"The cycle variance of an entire up/down bobbing cycle."						);
	Cvar_RegisterVariable (&v_bobup,				"Makes the up/down part of the cycle last longer."								);
	Cvar_RegisterVariable (&v_bobside,				"Side bobbing (left/right) equivalent of cl_bob (up/down).  Not present in Quake.");
	Cvar_RegisterVariable (&v_bobsidecycle,			"Side bobbing equivalent of v_bobcycle.  Not present in Quake."				);
	Cvar_RegisterVariable (&v_bobsideup,			"Side bobbing equivalent of v_bobsideup.  Not present in Quake."				);

	Cvar_RegisterVariable (&v_kicktime,				"Amount of time a damage view kick lasts."										);
	Cvar_RegisterVariable (&v_kickroll,				"Amount of tilt from a damage view kick."										);
	Cvar_RegisterVariable (&v_kickpitch,			"Amount of pitch (up/down) from a damage view kick."							);

}