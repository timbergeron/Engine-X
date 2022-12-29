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
// cvar_registry.c -- in the future, make it look all pretty


/*
#define CVAR_NONE			0
#define CVAR_ARCHIVE		1		// set to true to cause it to be saved to config.cfg
#define CVAR_SERVER			2		// notifies players when changed
#define CVAR_ROM			4		// read only

#define CVAR_INIT			8		// can only be set during initialization
#define	CVAR_USER_CREATED	16		// created by a set command (Baker: Not sure this will be enabled in future

#define CVAR_COMMAND		128		// OnChange function takes place post-processing as a pure function call
#define CVAR_CMDLINE		256		// Cvar was forced via the command line
#define CVAR_EXTERNAL		512		// Baker: Video info is written to separate file for early read


// Marking purposes only for future alterations

#define CVAR_LEGACY 		4	// Mark cvars that don't serve a purpose these days
#define CVAR_PARITY			4


typedef struct cvar_s
{
	char			*name;
	char			*string;
	int				flags;
	qbool			(*OnChange)(struct cvar_s *var, char *value);
	float			value;
	char			*default_string;
	struct cvar_s	*next;
} cvar_t;


*/

#include "quakedef.h"

// Baker:  This is very non-cvar like behavior so we need special justification of exactly why
//         they get to have a control function.

// We also need to suggest an idea to eliminate said problem.  This does not mean we will do it.

// Is Problem for default startup:  No because glsetupstate uses .string value which is already set
qbool OnChange_PolygonOffSetCrap (cvar_t *var, const char *string);
// because we need to execute gl commands.  This is only done in glSetupState on video initialization
// and if change them in-game, the cvar needs to run the function to update it
// (A way that this could be avoided is in the beginning of rendering doing the "store old value and check checked" thing.)

// Is Problem for default startup: Only if we use exceptionally dumb value out of validation range
qbool OnChange_demospeed (cvar_t *var, const char *string);
// It is doing a range check validation (0 to 20).  I guess Jozsef wanted cl_demospeed to reflect actual value.
// (Elimination by bounds(0, cl_demospeed, 20)

// Is problem for default startup: Absolutely
qbool OnChange_cl_effectslevel (cvar_t *var, const char *string);
// Baker: to change all the cvars.
// (Elimination by coding cl_effectslevel into everything.)

// Is problem for default startup: Absolutely
//qbool OnChange_cl_quickstart (cvar_t *var, const char *string);
// Sets no_start_demos to true

// Is problem for default startup.  Somehow no?  It works anyway.
// Reason:  Font load calls Draw_LoadCharset
qbool OnChange_gl_consolefont (cvar_t *var, const char *string);
// To load the font

// Is problem for default startup?  Yes.
qbool OnChange_gl_crosshairimage (cvar_t *var, const char *string);
// load it

// Is problem for default startup? No.  It reads hardware.  Check to see what happens if that fails.   It is fine.
//qbool OnChange_gl_max_size (cvar_t *var, const char *string);
// To give a message and range validate it versus powers of 2.
// (Elimination plan:  it could be eliminated, but it is more helpful
// in the current state)

// Is problem for default startup?  No.  Drawloadcharset uses cvar value
qbool OnChange_gl_smoothfont (cvar_t *var, const char *string);
// applies the filter to the console

// Is problem for default startup?  Yes.
qbool OnChange_gl_texturemode (cvar_t *var, const char *string);
// Applies different texture filter to the textures
// ONLY does it to TEX_MIPMAP textures so not to 2D gfx, etc.
// But for sure does it to particles and detail textures

// Is problem for default startup?  Yes.
qbool OnChange_m_directinput (cvar_t *var, const char *string);
// To initialize or shut down

#if SUPPORTS_MP3_TRACKS
qbool OnChange_mp3volume (cvar_t *var, const char *string);
// It has to change the volume, it doesn't "think" everyframe it is rather independent

qbool OnChange_mp3_enabled (cvar_t *var, const char *string);
// It has to stop the music, it doesn't "think" everyframe it is rather independent


#endif
qbool OnChange_pq_lag (cvar_t *var, const char *string);
// Range validation and the PING + message
// Baker: This is better than how it was with code in cvar.c
// (We could eliminate it, by eliminating the pq_lag cvar and just
// making it only accessible via PING +(whatever) command.)

// Well ... this is really an in-game cvar.
// What we need to do is do cvar setdefault on this cvar when map changes.(DONE)
// We need to do this with fog too (DONE)
qbool OnChange_r_skybox (cvar_t *var, const char *string);


qbool OnChange_r_list_models              (cvar_t *var, const char *string);
//qbool OnChange_r_list_models_additive            (cvar_t *var, const char *string);
//qbool OnChange_r_list_textures_prefix_reflective (cvar_t *var, const char *string);
//qbool OnChange_r_list_textures_prefix_glassy	 (cvar_t *var, const char *string);




// Baker: This is to allow a user to load a skybox on the fly
// Now at the start of the map, we clear this out and presumably unload it.
// (Elimation idea: There is no good way to do this.  Except make it command.  Maybe it should be.)

// Don't dare do anything with this
//qbool OnChange_sys_highpriority (cvar_t *var, const char *string);

// Is problem?  Shouldn't be.
qbool OnChange_vid_consize (cvar_t *var, const char *string);

// Is problem?
qbool OnChange_vid_vsync (cvar_t *var, const char *string);

// Remember: the only purpose of default values is so the user can reset them
//           additionally




// EXTERNALS ARE OWNED BY THE ENGINE.  They will not be acknowledged from a config.  They will not be reset unless marked
// resetable.  They are saved in external.cfg in the engine folder.

// First, this ensures they cannot be killed by some config and are unaffected by gamedir switching.


// Cvar init are hard settings that we are making available to the user, but are not available for modification.  EVER!
// Things like the command line, "registered" or

// **** GENERAL **********************************************************************************************************



cvar_t session_cmdline = {"cmdline", "", CVAR_CMDLINE | CVAR_SERVER};		// Only because TEST2 uses this (re-write test2?)
cvar_t session_registered = {"registered", "0"};


// **** CLIENT ***********************************************************************************************************

cvar_t cl_net_name =			{"_cl_name",		"player",				CVAR_ARCHIVE};	// "Not intended to be set directly" ... says Quake engine note
cvar_t cl_net_color =			{"_cl_color",		"0",					CVAR_ARCHIVE};	// "Not intended to be set directly" ... says Quake engine note
cvar_t cl_print_shownet =			{"cl_shownet",		"0"};									// can be 0, 1, or 2 ... does it even do anything?
cvar_t cl_ent_nolerp =			{"cl_nolerp",		"0"};									// Define what this does
cvar_t msg_needrl =			{"pq_needrl",		"I need RL",			CVAR_ARCHIVE};	// JPG - added these for %r formatting
cvar_t msg_haverl =			{"pq_haverl",		"I have RL",			CVAR_ARCHIVE};
cvar_t msg_needrox =			{"pq_needrox",		"I need rockets",		CVAR_ARCHIVE};
cvar_t msg_quad =			{"pq_quad",			"quad",					CVAR_ARCHIVE};	// JPG - added these for %p formatting
cvar_t msg_pent =			{"pq_pent",			"pent",					CVAR_ARCHIVE};
cvar_t msg_ring =			{"pq_ring",			"eyes",					CVAR_ARCHIVE};
cvar_t msg_weapons =			{"pq_weapons",		"SSG:NG:SNG:GL:RL:LG",	CVAR_ARCHIVE};	// JPG 3.00 - added these for %w formatting
cvar_t msg_noweapons =		{"pq_noweapons",	"no weapons",			CVAR_ARCHIVE};
cvar_t in_smartjump =			{"pq_moveup",		"0",					CVAR_ARCHIVE};	// JPG 1.05 - translate +jump to +moveup under water
cvar_t v_pq_smoothcam =		{"pq_smoothcam",	"1",					false};	// JPG 3.00 - added this by request

#if HTTP_DOWNLOAD
cvar_t cl_web_download		= {"cl_web_download", "1",					CVAR_EXTERNAL};
cvar_t cl_web_download_url	= {"cl_web_download_url", "http://downloads.quake-1.com/", CVAR_NONE};
#endif


// Console



cvar_t session_quickstart =		{"cl_quickstart",		"0",	CVAR_EXTERNAL};



cvar_t session_confirmquit =		{"cl_confirmquit",		"1",	CVAR_EXTERNAL};
cvar_t scr_notify_chatsound =	{"cl_mute",				"0"};											//R00k

cvar_t scr_centerprint_log = {"con_logcenterprint",	"1"};											//johnfitz
cvar_t scr_centerprint_nodraw =	{"con_nocenterprint",	"0"};											// Rook
cvar_t scr_notify_lines =	{"con_notifylines",		"4"};
cvar_t scr_notify_time =		{"con_notifytime",		"3"};		//seconds
cvar_t session_savevars =		{"cvar_savevars",		"1"};

//cvar_t cl_advancedcompletion	= {"cl_advancedcompletion", "1"};
//cvar_t cvar_viewdefault = {"cvar_viewdefault", "1"};		// This doesn't do anything!

cvar_t game_kurok =			{"game_kurok",			"0"};


// ProQuake general enhancements
cvar_t scr_con_filter =		{"pq_confilter",		"0"};					// JPG 1.05 - filter out the "you got" messages
cvar_t sv_chat_connectmute =		{"pq_connectmute",		"0",	CVAR_SERVER};	// (value in seconds)  // Baker 3.99g - from Rook ... protect against players connecting and spamming before banfile can kick in
cvar_t scr_con_chatlog_dequake =			{"pq_dequake",			"1"};					// JPG 1.05 - translate dedicated server console output to plain text
cvar_t scr_con_chatlog =		{"pq_logbinds",			"0",	CVAR_SERVER};	// JPG 3.20 - optionally write player binds to server log
cvar_t host_maxfps =		{"pq_maxfps",			"72.0", CVAR_EXTERNAL};	// Baker 3.80x - save this to config
#if VARIABLE_EDICTS_AND_ENTITY_SIZE
cvar_t host_maxedicts =		{"max_edicts",			"0", CVAR_EXTERNAL};	// Baker 3.80x - save this to config
cvar_t host_maxedicts_pad =		{"max_edicts_pad",			"300", CVAR_EXTERNAL};	// Baker 3.80x - save this to config
#endif
cvar_t scr_con_chatlog_removecr =		{"pq_removecr",			"1"};					// JPG 3.20 - remove \r from console output
cvar_t scr_con_chatlog_playerslot =		{"pq_showedict",		"0"};					// JPG 3.11 - feature request from Slot Zero (Against impersonation chat binds that change name, say, and then change name.  Just shows player num
cvar_t sv_chat_grace =		{"pq_spam_grace",		"999"};					// Baker 3.80x - Set to default of 999; was 10 -- bad for coop num messages before limiting kicks in
cvar_t sv_chat_rate =		{"pq_spam_rate",		"0"};					// Baker 3.80x - Set to default of 0; was 1.5 -- bad for coop --- num seconds between messages
cvar_t sv_chat_changemute =	{"pq_tempmute",			"0"};					// Baker 3.80x - Changed default to 0; was 1 -- interfered with coop // JPG 3.20 - control muting of players that change colour/name
cvar_t scr_con_chatlog_timestamp =		{"pq_timestamp",		"0"};					// JPG 1.05 - timestamp player binds during a match

//cvar_t cl_gameplayhack_monster_lerp = {"cl_gameplayhack_monster_lerp","1"};
// **** CLASSIC RENDERING OPTIONS: NOT VERY RELEVANT OR COMMONLY USED ****************************************************

// Stock classic OpenGL stuffs
cvar_t gl_clear =			{"gl_clear",			"-1",	false};		// Baker: we are defaulting to -1 due to Intel display adapter "fear"
cvar_t gl_cull =			{"gl_cull",				"1",	CVAR_LEGACY};
//cvar_t gl_ztrick =			{"gl_ztrick",			"0",	CVAR_LEGACY};	// We are doing ztrick off as default
cvar_t gl_finish =			{"gl_finish",			"0",	CVAR_LEGACY};
cvar_t gl_smoothmodels =	{"gl_smoothmodels",		"1",	CVAR_LEGACY};	// Appears to make no rendering or FPS difference at all
cvar_t gl_affinemodels =	{"gl_affinemodels",		"0",	CVAR_LEGACY};
//cvar_t gl_playermip =		{"gl_playermip",		"0",	CVAR_LEGACY};
cvar_t cl_ent_nocolors =		{"gl_nocolors",			"0",	false};
cvar_t cl_ent_doubleeyes =		{"gl_doubleeyes",		"1",	CVAR_LEGACY};
cvar_t r_subdivide_size =	{"gl_subdivide_size", "128",	CVAR_LEGACY};
cvar_t tex_picmip =			{"gl_picmip",			"0",	CVAR_LEGACY};
cvar_t tex_picmip_allmodels ={"gl_picmip_allmodels",	"1",	CVAR_LEGACY};	// World, bmodels, alias, sprites



cvar_t scr_con_alpha =		{"gl_conalpha",		  "0.8",	CVAR_LEGACY};




cvar_t gl_polygonoffset_factor = {"gl_polygonoffset_submodel_factor", "0.05", CVAR_COMMAND, /**/OnChange_PolygonOffSetCrap};
cvar_t gl_polygonoffset_offset = {"gl_polygonoffset_submodel_offset", "0.25", CVAR_COMMAND, /**/OnChange_PolygonOffSetCrap};

// PolygonOffSet helps deal with z-fighting and so forth.  This is really an internal development variable.
qbool OnChange_PolygonOffSetCrap (cvar_t *var, const char *string)
{
//	GL_Manager_ChangeBase_Begin ();
	MeglPolygonOffset(gl_polygonoffset_factor.floater, gl_polygonoffset_offset.floater);
//  GL_Manager_ChangeBase_End ();
	if (!accepts_default_values)
		Con_Printf ("Polygon offset params updated\n");		// Silent if reseting the default
	return false;
}



// More general rendering


cvar_t r_drawentities =			{"r_drawentities",		"1"};
cvar_t r_drawparticles =		{"r_drawparticles",			"1"};
cvar_t r_drawmodels_alias =		{"r_drawmodels_alias",			"1"};

cvar_t r_drawmodels_sprite =	{"r_drawmodels_sprite",			"1"};
cvar_t r_drawmodels_brush =	{"r_drawmodels_bmodels",			"1"};
cvar_t r_drawentities_alpha =	{"r_drawmodels_alphaents",			"1"};



cvar_t r_brushmodels_with_world =		{"r_brushmodels_with_world",		"0"};


cvar_t r_nodrawentnum =		{"r_nodrawentnum",		"0"};
cvar_t r_speeds =			{"r_speeds",			"0"};
//cvar_t r_fullbright =		{"r_fullbright",		"0"};
cvar_t r_lightmap =			{"r_lightmap",			"0"};
cvar_t r_shadows =			{"r_shadows",			"0"};

cvar_t r_water_alpha =		{"r_wateralpha",		"1"};
cvar_t r_water_ripple =		{"water_ripple",		"0",		false};
cvar_t scene_dynamiclight =			{"r_dynamic",			"1"};
cvar_t scene_novis =			{"r_novis",				"0"};
cvar_t scene_farclip =			{"r_farclip",			"16384",	false};
//cvar_t r_fullbrightskins =	{"r_fullbrightskins",	"0"};						// r_lightmap 2
cvar_t r_fastsky =			{"r_fastsky",			"0",		CVAR_PARITY};
cvar_t r_skycolor =			{"r_skycolor",			"4"};
cvar_t r_skybox =			{"r_skybox",			"",			CVAR_NONE,	/**/OnChange_r_skybox};
cvar_t scene_lockpvs =			{"r_lockpvs",			"0"};
cvar_t tool_showbboxes =		{"tool_showbboxes",		"0"};
cvar_t tool_showspawns =		{"tool_showspawns",		"0"};
cvar_t r_water_warp =		{"water_warp",         "0"};

#if SUPPORTS_OVERBRIGHT_SWITCH
cvar_t light_overbright	=	{"light_overbright",		"1"};
#endif
cvar_t r_pass_lumas				=	{"r_lumas",		"1"};

#if SUPPORTS_GL_DELETETEXTURES
cvar_t tex_free_on_newmap  =	{"tex_free_on_newmap", "0"};
#endif
cvar_t tex_palette_gamma =	{"tex_palette_gamma",		"1"};
cvar_t tex_palette_contrast =	{"tex_palette_contrast",	"1"};

cvar_t r_cullsurfaces =		{"r_cullsurfaces",		"1"};	// Doesn't work; isn't implemented

#if SUPPORTS_TEXTURE_POINTER
cvar_t tool_texturepointer =	{"tool_texturepointer",	"0"};
#endif

//Note:  client_no will ignore all of these flags
cvar_t	list_models_r_nolerp             = {"list_models_nolerp",          "progs/flame.mdl,progs/flame2.mdl,progs/braztall.mdl,progs/brazshrt.mdl,progs/longtrch.mdl,progs/flame_pyre.mdl,progs/v_saw.mdl,progs/v_xfist.mdl,progs/h2stuff/newfire.mdl", CVAR_COMMAND, OnChange_r_list_models};
cvar_t	list_models_r_noshadow           = {"list_models_noshadow",        "progs/flame.mdl,progs/flame2.mdl,progs/flame0.mdl,progs/bolt1.mdl,progs/bolt2.mdl,progs/bolt3.mdl,progs/laser.mdl", CVAR_COMMAND, OnChange_r_list_models};
cvar_t	list_models_r_fullbright         = {"list_models_fullbright",      "progs/flame.mdl,progs/flame2.mdl,progs/flame0.mdl,progs/boss.mdl", CVAR_COMMAND, OnChange_r_list_models};

cvar_t	list_demos_hud_nodraw			= {"list_title_demos", ""};


// Baker: We can always upload the model textures with alpha mask (255 = no alpha) and simply not use the alpha channel right?
//        Or can this fuck up multitexture somehow?
cvar_t  list_models_r_additive           = {"list_models_additive",        "", CVAR_COMMAND, OnChange_r_list_models};
cvar_t  list_models_r_filter             = {"list_models_filter",          "", CVAR_COMMAND, OnChange_r_list_models};
cvar_t  list_models_r_fence_qpal255      = {"list_models_fence_qpal255",   "", CVAR_COMMAND, OnChange_r_list_models};

qbool OnChange_r_list_models (cvar_t *var, const char *string)
{
	void Mod_SetExtraFlags (model_t *mod);

	int i;

	for (i=0; i < MAX_MODELS; i++)
		Mod_SetExtraFlags (cl.model_precache[i]);

	return false;
}

// Interpolation

cvar_t scene_lerpmodels = {"r_lerpmodels", "1", false};  // fenix@io.com: model interpolation
cvar_t scene_lerpmove = {"r_lerpmove", "1", false};// fenix@io.com: model interpolation

// General rendering
cvar_t r_pass_caustics =				{"gl_caustics",				"0"};
cvar_t r_pass_detail =			{"gl_detail_texture",		"0"};
cvar_t r_pass_lumas_brush =		{"gl_fullbright_bmodels",			"1"};
cvar_t r_pass_lumas_alias =		{"gl_fullbright_models",			"1"};
cvar_t r_vertex_blenddist =				{"r_interpolate_distance",	"17000"};
cvar_t external_lits =			{"external_lits",			"1"};
cvar_t r_viewmodel_ringalpha =				{"viewmodel_ringalpha",			"0.4",		false};
cvar_t particle_blend =			{"particle_blend",		"0"};


cvar_t r_vertex_lights =			{"gl_vertexlights",			"0"};
cvar_t r_water_fog =				{"water_fog",				"1"};
cvar_t r_water_fog_density =		{"water_fog_density",	"1"};
cvar_t tex_lerp     =		{"gl_lerptextures",		"0"};

#if SUPPORTS_XFLIP
cvar_t scene_invert_x =				{"scene_invert_x", "0"}; //Atomizer - GL_XFLIP
#endif
// Particle rendering
cvar_t qmb_bounceparticles = {"qmb_bounceparticles", "1"};
cvar_t qmb_clipparticles = {"qmb_clipparticles", "0"};
cvar_t qmb_blobexplosions = {"qmb_blobs", "0"};
cvar_t qmb_blood = {"qmb_blood", "0"};
cvar_t qmb_explosions = {"qmb_explosions", "0"};
cvar_t qmb_flames = {"qmb_flames", "0"};
cvar_t qmb_gunshots = {"qmb_gunshots", "0"};
cvar_t qmb_inferno = {"qmb_inferno", "0"};
cvar_t qmb_lavasplash = {"qmb_lavasplash", "0"};
cvar_t qmb_lightning = {"qmb_lightning", "0"};
cvar_t qmb_spikes = {"qmb_spikes", "0"};
cvar_t qmb_spiketrails = {"qmb_spiketrails", "0"};
cvar_t qmb_telesplash = {"qmb_telesplash", "0"};
cvar_t qmb_trails = {"qmb_trails", "0"};

//cvar_t gl_fogenable		= {"gl_fogenable", "0", /*CVAR_NORESET*/};
//cvar_t gl_fogdensity	= {"gl_fogdensity", "0.4", /*CVAR_NORESET*/};
//cvar_t gl_fogdensity		= {"gl_fogdensity", "0.153"};

//cvar_t gl_fogred			= {"gl_fogred", "0.4", /*CVAR_NORESET*/};
//cvar_t gl_fogblue			= {"gl_fogblue", "0.4" /*, CVAR_NORESET*/};
//cvar_t gl_foggreen			= {"gl_foggreen", "0.4", /*CVAR_NORESET*/};

//cvar_t gl_fogstart			= {"gl_fogstart", "50.0", /*CVAR_NORESET*/};
//cvar_t gl_fogend			= {"gl_fogend", "800.0", /*CVAR_NORESET*/};

//cvar_t gl_fogsky			= {"gl_fogsky", "1", /*CVAR_NORESET*/};  // Baker: This isn't even used?


cvar_t r_fog			= {"r_fog", "1", /*CVAR_NORESET*/};


// Interactive renderer stuffs

//cvar_t gl_max_size = {"gl_max_size", "1024", 0, /**/OnChange_gl_max_size};
cvar_t tex_scene_texfilter = {"gl_texturemode", "GL_LINEAR_MIPMAP_NEAREST", 0, /**/OnChange_gl_texturemode};
cvar_t external_textures = {"external_textures", "1"};
cvar_t gl_externaltextures_bmodels = {"gl_externaltextures_bmodels", "1"};
cvar_t scr_con_font = {"console_font", "default", 0, /**/OnChange_gl_consolefont};
cvar_t scr_con_font_smooth = {"console_font_smooth", "0", CVAR_NONE, /**/OnChange_gl_smoothfont};   // Baker: in other engines this is a command!

// **** VIEW: NOT RELATED TO THE WORLD RENDERING SEGMENT******************************************************************
cvar_t v_rollspeed = {"cl_rollspeed", "200", false};
cvar_t v_rollangle = {"cl_rollangle", "2.0", false}; // Quake classic default = 2.0
cvar_t v_bob = {"cl_bob","0.02", false}; // Quake classic default = 0.02
cvar_t v_bobcycle = {"cl_bobcycle","0.6", false}; // Leave it
cvar_t v_bobup = {"cl_bobup","0.5", false}; // Quake classic default is 0.5
cvar_t v_bobside = {"cl_bobside","0", false};
cvar_t v_bobsidecycle = {"cl_bobsidecycle","0", false}; // Leave it
cvar_t v_bobsideup = {"cl_bobsideup","0", false};

cvar_t v_kicktime = {"v_kicktime", "0.5", false};  //"0.5", true};  // Baker 3.80x - Save to config
cvar_t v_kickroll = {"v_kickroll", "0.6", false};  //"0.6", true};  // Baker 3.80x - Save to config
cvar_t v_kickpitch = {"v_kickpitch", "0.6", false};  //"0.6", true};  // Baker 3.80x - Save to config

cvar_t v_gunkick = {"v_gunkick", "0", false};
cvar_t v_iyaw_cycle = {"v_iyaw_cycle", "2", false};
cvar_t v_iroll_cycle = {"v_iroll_cycle", "0.5", false};
cvar_t v_ipitch_cycle = {"v_ipitch_cycle", "1", false};
cvar_t v_iyaw_level = {"v_iyaw_level", "0.3", false};
cvar_t v_iroll_level = {"v_iroll_level", "0.1", false};
cvar_t v_ipitch_level = {"v_ipitch_level", "0.3", false};
cvar_t v_idlescale = {"v_idlescale", "0", false};

cvar_t v_centermove = {"v_centermove", "0.15", false};
cvar_t v_centerspeed = {"v_centerspeed","500"};



//#if SUPPORTS_SMOOTH_STAIRS
cvar_t v_smoothstairs = {"v_smoothstairs", "0"}; // Baker no like this but someone else likely does
//#endif

//cvar_t default_fov = {"default_fov","0", false};	// Baker 3.85 - Default_fov from FuhQuake

// Kurok is a HUD and hull size changes and some cvar changes.

// **** CLIENT-SIDE 3D WORLD RENDERING PREFERENCES (PARTICLES, COLOR, ETC.) **********************************************
cvar_t cl_ent_truelightning		= {"ent_truelightning",		"0"};					// Hmmmm ...
//cvar_t cl_rocket2grenade		= {"cl_r2g",				"0"};					// Baker: kill this
//cvar_t cl_mapname				= {"mapname",				"",		CVAR_ROM};		// Baker: DESTROY THIS CVAR
cvar_t light_powerups			= {"light_powerups",			"1"};
cvar_t particle_explosiontype			= {"particle_explosiontype",		"0"};
cvar_t light_explosions			= {"light_explosionlight",		"1"};
cvar_t light_rockets			= {"light_rocketlight",			"1"};
cvar_t light_explosions_color	= {"light_explosionlightcolor", "0"};
cvar_t light_rockets_color		= {"light_rocketlightcolor",	"0"};
cvar_t particle_rockettrail			= {"particle_rockettrail",			"1"};
cvar_t particle_grenadetrail			= {"particle_grenadetrail",		"1"};

cvar_t cl_ent_rotate_items_bob		= {"ent_rotate_items_bob",			"0"};
cvar_t cl_ent_deadbodyfilter		= {"ent_deadbodyfilter",		"0"};
cvar_t cl_ent_gibfilter			= {"ent_gibfilter",			"0"};
cvar_t gl_loadq3models			= {"gl_loadq3models", "1"};

// Dynamic lights

cvar_t light_flashblend			=			{"light_flashblend",		"0",		false /*CVAR_LEGACY*/ };	// Affects manner the flashes are done
cvar_t light_muzzleflash			= {"light_muzzleflash",		"1"};					// Does this actually work?
cvar_t light_lerplightstyle			=	{"light_lerplightstyle",	"0"};

// **** CLIENT OVERLAYS (STATUS BAR, SCOREBOARD, 2D SIZES, DISPLAY INDICATORS ) ******************************************
cvar_t scr_hud_extrabuffer = {"gl_triplebuffer", "1", false};

cvar_t scr_centerprint_time = {"scr_centertime", "2"};

cvar_t scr_con_speed = {"scr_conspeed", "99999", false};
cvar_t scene_fov_x = {"fov", "90", true};	// 10 - 170
cvar_t scene_fov_y = {"scr_fovy", "90", false};	// 10 - 170
cvar_t scene_fov_scale = {"scr_fov_scale", "1", false};	// 10 - 170
cvar_t scene_fov_scaleaspect = {"scr_fov_scaleaspect", "1.33333333", false};	// 10 - 170
cvar_t scr_victory_printspeed = {"scr_printspeed", "8"};
cvar_t scr_show_pause = {"showpause", "1"};
cvar_t scr_show_ram = {"showram", "1"};
cvar_t scr_show_turtle = {"showturtle", "0"};


cvar_t scene_viewsize = {"viewsize", "100", false};
cvar_t scr_show_fps = {"show_fps", "1", CVAR_EXTERNAL};
cvar_t scr_show_coords = {"show_coords", "0", CVAR_EXTERNAL};
cvar_t scr_show_angles = {"show_angles", "0", CVAR_EXTERNAL};
cvar_t scr_show_location = {"show_location", "0", CVAR_EXTERNAL};
cvar_t scr_show_cheats = {"show_cheats", "0", CVAR_EXTERNAL};
cvar_t scr_show_skill = {"show_skill", "0", CVAR_EXTERNAL};
// cvar_t show_fps_x = {"show_fps_x", "-5"};
// cvar_t show_fps_y = {"show_fps_y", "2"};
cvar_t scr_show_speed = {"show_speed", "0", CVAR_NONE};
cvar_t scr_show_stats = {"show_stats", "0"};
cvar_t scr_show_stats_length = {"show_stats_length", "0.5"};
cvar_t scr_show_stats_small = {"show_stats_small", "1"};
cvar_t scr_con_size = {"console_consize", "0.5"};		// by joe

cvar_t	scr_menu_scale = {"scr_scalemenu", "1"};
cvar_t	scr_menu_center = {"scr_centermenu", "1"};



// ProQuake display improvements

cvar_t scr_hud_style =			{"hud_style",			"1",		CVAR_NONE	};	// Baker: this name should be changed to vary from ProQuake --- messes with the defaults

// **** SCOREBOARD: MULTIPLAYER ******************************************************************************************
cvar_t scr_hud_scoreboard_pings =	{"hud_scoreboard_pings", "1"						};	// JPG - show ping times in the scoreboard
cvar_t scr_hud_teamscores =			{"hud_teamscores",		"1"						};	// JPG - show teamscores
cvar_t scr_hud_timer	=				{"hud_timer",			"1"						};	// JPG - show timer

cvar_t scr_hud_center =			{"hud_centersbar",		"1"						};	// Single player only?  Yikes!  This doesn't doo anything!

cvar_t scr_con_scale =			{"vid_consize",			"-1",		CVAR_EXTERNAL, /**/OnChange_vid_consize};	// Baker 3.97


// **** WEAPON AND CROSSHAIR PRESENTATION ********************************************************************************

cvar_t scr_crosshair =				{"crosshair",			"1",		CVAR_ARCHIVE};
cvar_t scr_crosshair_style =		{"crosshair_style",	"1",		false};
cvar_t scr_crosshair_color =		{"crosshair_color",	"31",		false};
cvar_t scr_crosshair_size	=		{"crosshair_size",	"1.5",		false};
cvar_t scr_crosshair_alpha =		{"crosshair_alpha",	"1"};
cvar_t scr_crosshair_exceptions =	{"crosshair_exceptions", ""};
cvar_t scr_crosshair_image =		{"crosshair_image", "default", 0, /**/OnChange_gl_crosshairimage};
cvar_t scr_crosshair_offset_x =		{"crosshair_offset_x",			"0",		false	};	// Baker: I don't believe this is needed nowadays
cvar_t scr_crosshair_offset_y =		{"crosshair_offset_y",			"0",		false	};	// Baker: I don't believe this is needed nowadays



cvar_t r_drawviewmodel =			{"r_drawviewmodel",		"1",		false};
cvar_t r_viewmodel_hackpos =		{"viewmodel_hackpos",		"1",		false};
cvar_t r_viewmodel_pitch =			{"viewmodel_pitch",		"0",		false};
cvar_t r_viewmodel_offset =			{"viewmodel_offset",	"0",		false};
cvar_t r_viewmodel_size =			{"viewmodel_size",		"1",		false};

cvar_t r_viewmodel_fovscale =		{"viewmodel_fovscale",		"110",		false};

cvar_t screenshot_format	 =		{"screenshot_format",		"jpg",		false};


// **** VIEW BLENDS ******************************************************************************************************
//cvar_t gl_cshiftpercent =		{"gl_cshiftpercent",	"100"					};
//cvar_t gl_hwblend =				{"gl_hwblend",			"1"						};

cvar_t scene_polyblend =			{"gl_polyblend",		"1",		false /*CVAR_LEGACY*/ };

cvar_t vb_contentblend =			{"v_contentblend",		"1",		false};
cvar_t vb_pentcshift =			{"v_pentcshift",		"1",		false};
cvar_t vb_quadcshift =			{"v_quadcshift",		"1",		false};
cvar_t vb_ringcshift =			{"v_ringcshift",		"1",		false};
cvar_t vb_suitcshift =			{"v_suitcshift",		"1",		false};
cvar_t vb_bonusflash =			{"v_bonusflash",		"1"						};
cvar_t vb_damagecshift =			{"v_damagecshift",		"1",		false};
//cvar_t v_dlightcshift =		{"v_dlightcshift",		"1"						};		// gl_flashblend 1 bubble blend.  Pointless



cvar_t vid_brightness_gamma =				{"gamma",				".5",		CVAR_EXTERNAL};	// 50 to 100 range
cvar_t vid_brightness_contrast =				{"contrast",			"2",		CVAR_EXTERNAL};	// 25 to 100 range



// **** DEVICE INPUT *****************************************************************************************************
cvar_t in_keypad = {"cl_keypad","0", CVAR_EXTERNAL};
cvar_t vid_altenter_toggle = {"cl_key_altenter", "1", CVAR_EXTERNAL}; // Baker 3.99q: allows user to disable new ALT-ENTER behavior

cvar_t in_keymap =				{"in_keymap",			"1",		false};
cvar_t in_mouse =				{"in_mouse",			"1",		false};
cvar_t in_joystick =			{"joystick",			"0",		CVAR_ARCHIVE};


// **** MOUSE INPUT ******************************************************************************************************


#if SUPPORTS_DIRECTINPUT
cvar_t mouse_directinput =			{"m_directinput",		"1",		CVAR_EXTERNAL | CVAR_COMMAND, /**/OnChange_m_directinput};
#else
cvar_t mouse_directinput =			{"m_directinput",		"0",		CVAR_EXTERNAL | CVAR_COMMAND};
#endif


cvar_t in_sensitivity =			{"sensitivity",			"3",		CVAR_EXTERNAL};
cvar_t in_invert_pitch =			{"invertmouse",			"0",		CVAR_EXTERNAL};
cvar_t in_fovscale ={"mouse_scale_sensitivity",			"1",		CVAR_EXTERNAL};

cvar_t mouse_speed_pitch =				{"m_pitch",				"0.022",	false};
cvar_t mouse_speed_yaw =					{"m_yaw",				"0.022",	false};

cvar_t mouse_lookstrafe_speed_forward =				{"m_forward",			"1",		false};
cvar_t mouse_lookstrafe_speed_side =					{"m_side",				"0.8",		false};

cvar_t in_accel =				{"m_accel",				"0",		false};
cvar_t in_freelook =				{"freelook",			"1",		CVAR_EXTERNAL};
cvar_t in_filter =				{"m_filter",			"0"};

#if ALTER_WINDOWS_MPARMS
cvar_t mouse_setparms	=			{"m_setparms",			"0"};
#endif

// **** KEYBOARD INPUT ***************************************************************************************************

cvar_t cl_speed_up =				{"cl_upspeed",			"200",		false};
cvar_t cl_speed_forward =		{"cl_forwardspeed",		"400",		false}; // Quake default is actually 200 ... changed
cvar_t cl_speed_back =			{"cl_backspeed",		"400",		false}; // Quake default is actually 200 ... changed
cvar_t cl_speed_side =			{"cl_sidespeed",		"350",		false};

cvar_t keyboard_speed_move_multiplier =		{"cl_movespeedkey",		"2.0"					};
cvar_t keyboard_speed_angle_multiplier =		{"cl_anglespeedkey",	"1.5"					};

cvar_t keyboard_speed_yaw =			{"cl_yawspeed",			"140"					};
cvar_t keyboard_speed_pitch =			{"cl_pitchspeed",		"150"					};

cvar_t in_lookspring =				{"lookspring",			"0",		CVAR_ARCHIVE};
cvar_t in_lookstrafe =				{"lookstrafe",			"0",		CVAR_ARCHIVE};



cvar_t cl_net_lag =					{"pq_lag",				"0",		CVAR_NONE, /**/OnChange_pq_lag};

qbool OnChange_pq_lag (cvar_t *var, const char *string)
{
	float newval = atof(string);

	if (newval < 0 || newval > 200)
	{
		Con_Printf ("pq_lag only accepts values between 0 and 200\n");
		return true;	// Change rejected
	}

	if (cls.state != ca_connected)
		return false;	// Change accepted ... not connected

	Cbuf_AddText (va("say \"ping +%d\"\n", (int)newval));
	return false;
}


// **** JOYSTICK INPUT ***************************************************************************************************

cvar_t joy_name =				{"joyname",				"joystick"				};
cvar_t joy_advanced =			{"joyadvanced",			"0"						};
cvar_t joy_advaxisx =			{"joyadvaxisx",			"0"						};
cvar_t joy_advaxisy =			{"joyadvaxisy",			"0"						};
cvar_t joy_advaxisz =			{"joyadvaxisz",			"0"						};
cvar_t joy_advaxisr =			{"joyadvaxisr",			"0"						};
cvar_t joy_advaxisu =			{"joyadvaxisu",			"0"						};
cvar_t joy_advaxisv =			{"joyadvaxisv",			"0"						};
cvar_t joy_forwardthreshold =	{"joyforwardthreshold", "0.15"					};
cvar_t joy_sidethreshold =		{"joysidethreshold",	"0.15"					};
cvar_t joy_flysensitivity =		{"joyflysensitivity",	"-1.0"					};
cvar_t joy_flythreshold =		{"joyflythreshold",		"0.15"					};
cvar_t joy_pitchthreshold =		{"joypitchthreshold",	"0.15"					};
cvar_t joy_yawthreshold =		{"joyyawthreshold",		"0.15"					};
cvar_t joy_forwardsensitivity = {"joyforwardsensitivity","-1.0"					};
cvar_t joy_sidesensitivity =	{"joysidesensitivity", "-1.0"					};
cvar_t joy_pitchsensitivity =	{"joypitchsensitivity", "1.0"					};
cvar_t joy_yawsensitivity =		{"joyyawsensitivity",	"-1.0"					};
cvar_t joy_wwhack1 =			{"joywwhack1",			"0.0"					};
cvar_t joy_wwhack2 =			{"joywwhack2",			"0.0"					};

// **** AUDIO: MP3 PLAYER ************************************************************************************************
#if SUPPORTS_MP3_TRACKS
#ifdef MACOSX // fixme
qbool OnChange_mp3volume (cvar_t *var, const char *string)
{
	return false;
}
qbool OnChange_mp3_enabled (cvar_t *var, const char *string)
{
	return false;
}
#endif
qbool OnChange_mp3volume (cvar_t *var, const char *string);
qbool OnChange_mp3_enabled (cvar_t *var, const char *string);

cvar_t mp3_enabled =			{"mp3_enabled",			"1",		CVAR_EXTERNAL,       /**/OnChange_mp3_enabled};
cvar_t mp3_volume =				{"mp3volume",			"0.3",		CVAR_EXTERNAL,		/**/OnChange_mp3volume};
#else
// Baker: you are a horrible person for doing this
cvar_t mp3_enabled =			{"mp3_enabled",			"1",		CVAR_EXTERNAL};	// We do not want to lose these settings
cvar_t mp3_volume =				{"mp3volume",			"0.3",		CVAR_EXTERNAL}; // We do not want to lose these settings
#endif

// **** AUDIO: ACTUAL SOUND **********************************************************************************************

cvar_t snd_mixahead =			{"_snd_mixahead",		"0.1",		false};
cvar_t snd_ambient_fade =			{"ambient_fade",		"100"					};
cvar_t snd_ambient_level =			{"ambient_level",		"0.3",		CVAR_ARCHIVE}; // Baker 3.60 - Save to config
cvar_t snd_loadas8bit =				{"loadas8bit",			"0"};
cvar_t snd_nosound =				{"nosound",				"0"};
cvar_t snd_precache =				{"precache",			"1"};
cvar_t snd_noextraupdate =		{"snd_noextraupdate",	"0"};
cvar_t snd_show =				{"snd_show",			"0"};
cvar_t snd_volume =					{"volume",			"0.7",		CVAR_EXTERNAL};

// **** DEMO PLAYBACK ****************************************************************************************************
cvar_t demospeed =			{"demospeed",		"1",		CVAR_NONE,		/**/OnChange_demospeed};
cvar_t demorewind =			{"demorewind",		"0"						};

// **** CHASE CAMERA *****************************************************************************************************
cvar_t chase_back =				{"chase_back",				"100"				};
cvar_t chase_up =				{"chase_up",				"16"				};
cvar_t chase_right =			{"chase_right",				"0"					};
cvar_t chase_active =			{"chase_active",			"0"					};
cvar_t chase_yaw	=			{"chase_yaw"	,			"0"					};
cvar_t chase_roll =				{"chase_roll",				"0"					};
cvar_t chase_pitch =			{"chase_pitch",				"45"				};

// **** VIDEO ************************************************************************************************************

cvar_t _windowed_mouse = {"_windowed_mouse", "1", CVAR_NONE};

cvar_t vid_fullscreen =			{"vid_fullscreen",			"1"  ,	CVAR_EXTERNAL};
cvar_t vid_width =				{"vid_width",				"640",	CVAR_EXTERNAL};
cvar_t vid_height =				{"vid_height",				"480",	CVAR_EXTERNAL};
cvar_t vid_bpp =				{"vid_bpp",					"32" ,	CVAR_EXTERNAL};
cvar_t vid_displayfrequency =	{"vid_displayfrequency",	"60" ,	CVAR_EXTERNAL};

#ifndef MACOSX
cvar_t vid_vsync =				{"vid_vsync",				"0",	CVAR_EXTERNAL, OnChange_vid_vsync}; // D3D needs to know this early
#else
cvar_t vid_vsync =				{"vid_vsync",				"0",	CVAR_EXTERNAL}; // D3D needs to know this early

#endif // OSX FAIL
#pragma message ("OSX vid_vsync onchange")



cvar_t vid_brightness_method =		{"vid_brightness_method",		"2",	CVAR_EXTERNAL}; //Baker: I think 1 is better for now. R00k changed to 2 to support windowed modes!


// **** SCREENSHOT AND VIDEO CAPTURE *************************************************************************************

#if SUPPORTS_AVI_CAPTURE
cvar_t capture_codec	=		{"capture_codec",		"0",		CVAR_EXTERNAL};
cvar_t capture_fps	=			{"capture_fps",			"30.0"					};
cvar_t capture_console	=		{"capture_console",		"1"						};
cvar_t capture_hack	=			{"capture_hack",		"0"						};
cvar_t capture_mp3	=			{"capture_mp3",			"0"						};
cvar_t capture_mp3_kbps =		{"capture_mp3_kbps",	"128"					};
#endif



#ifndef NO_MODELVIEWER
// **** MODEL VIEWER *****************************************************************************************************
cvar_t modelviewer_name     =	{"modelviewer_name",     "progs/player.mdl"		};
cvar_t modelviewer_framenum =	{"modelviewer_framenum", "0"					};
cvar_t modelviewer_skinnum  =	{"modelviewer_skinnum",  "0"					};
cvar_t modelviewer_origin   =	{"modelviewer_origin",   "100 0 0"				};	// X is closeness
cvar_t modelviewer_angles   =	{"modelviewer_angles",   "-20 135 -20"			};
cvar_t modelviewer_scale    =	{"modelviewer_scale",    "1"					};
cvar_t modelviewer_axis     =	{"modelviewer_axis",     "0"					};
#endif

// **** DEVELOPER CVARS **************************************************************************************************

cvar_t developer =				{"developer",			"0"						};
cvar_t developer_filter =		{"developer_filter",	"0"						};
cvar_t developer_show_stufftext = {"developer_show_stufftext", "0"};


// **** NETWORK **********************************************************************************************************
cvar_t net_messagetimeout =		{"net_messagetimeout",	"300"					};
cvar_t net_connecttimeout =		{"net_connecttimeout",	"10"					};	// JPG 2.01 - qkick/qflood protection
cvar_t sv_hostname =				{"hostname",			"UNNAMED"				};
cvar_t pq_password =			{"pq_password",			""						};	// JPG 3.00 - password protection
cvar_t rcon_password =			{"rcon_password",		""						};	// JPG 3.00 - rcon password
cvar_t rcon_server =			{"rcon_server",			""						};	// JPG 3.00 - rcon server
cvar_t sv_signon_plus =		{"pq_cheatfree",		"0"						};

// **** SERVER QUAKEC GAME LOGIC CONTROL VARIABLES ***********************************************************************

cvar_t pr_coop =					{"coop",				"0"						};		// 0 or 1
cvar_t pr_deathmatch =				{"deathmatch",			"0"						};		// 0, 1, or 2
cvar_t pr_fraglimit =				{"fraglimit",			"0",		CVAR_SERVER	};
cvar_t pr_gamecfg =				{"gamecfg",				"0"						};
cvar_t pr_noexit =					{"noexit",				"0",		CVAR_SERVER	};
cvar_t pr_nomonsters =				{"nomonsters",			"0"						};
cvar_t sv_pausable =				{"pausable",			"1"						};
cvar_t pr_samelevel =				{"samelevel",			"0"						};
cvar_t pr_saved1 =					{"saved1",				"0",		CVAR_ARCHIVE};
cvar_t pr_saved2 =					{"saved2",				"0",		CVAR_ARCHIVE};
cvar_t pr_saved3 =					{"saved3",				"0",		CVAR_ARCHIVE};
cvar_t pr_saved4 =					{"saved4",				"0",		CVAR_ARCHIVE};
cvar_t pr_savedgamecfg =			{"savedgamecfg",		"0",		CVAR_ARCHIVE};
cvar_t pr_scratch1 =				{"scratch1",			"0"						};
cvar_t pr_scratch2 =				{"scratch2",			"0"						};
cvar_t pr_scratch3 =				{"scratch3",			"0"						};
cvar_t pr_scratch4 =				{"scratch4",			"0"						};
cvar_t pr_skill =					{"skill",				"1"						};		// 0 - 3
cvar_t pr_teamplay =				{"teamplay",			"0",		CVAR_SERVER	};
cvar_t pr_temp1 =					{"temp1",				"0"						};
cvar_t pr_timelimit =				{"timelimit",			"0",		CVAR_SERVER	};


// **** SERVER QUAKEC GAME LOGIC CONTROL VARIABLES ***********************************************************************

cvar_t host_framerate =			{"host_framerate",		"0"						};	// set for slow motion
cvar_t host_sleep =				{"host_sleep",			"0"						};
cvar_t host_speeds =			{"host_speeds",			"0"						};	// set for running times
cvar_t host_timescale =			{"host_timescale",		"0"						};	//johnfitz

#if SUPPORTS_MISSING_MODELS
cvar_t  host_sub_missing_models = {"host_sub_missing_models", "1",	false};
#endif

cvar_t host_clearmodels =		{"host_clearmodels",	"0"					};	//johnfitz
cvar_t serverprofile =			{"serverprofile",		"0"						};
cvar_t sys_ticrate =			{"sys_ticrate",			"0.05",		CVAR_SERVER	};

// Game rules, really

cvar_t sv_defaultmap =			{"sv_defaultmap",		""						}; //Baker 3.95: R00k
cvar_t sv_progs =				{"sv_progs",			"progs.dat"				};

cvar_t sv_altnoclip =			{"sv_altnoclip",		"1"						};	//don't save to config ... no reason to do so
cvar_t sv_cullentities =		{"sv_cullentities",		"1",		CVAR_SERVER };	// Baker 3.99c: Rook and I both think sv_cullentities is better cvar name than sv_cullplayers
cvar_t host_fullpitch =			{"pq_fullpitch",		"0",		false};	// JPG 2.01 .. Baker ... should be split into client and server control really
cvar_t sv_allcolors =			{"sv_allcolors",		"1",		CVAR_SERVER };	// BAKER: This *can* bust "old" ProQuake scoreboard because old proquake supports 13 teams.


cvar_t sv_accelerate =			{"sv_accelerate",		"10"					};
cvar_t sv_aim =					{"sv_aim",				"0.93"					};
cvar_t sv_edgefriction =		{"edgefriction",		"2"						};
cvar_t sv_friction =			{"sv_friction",			"4",		CVAR_SERVER	};
cvar_t sv_gravity =				{"sv_gravity",			"800",		CVAR_SERVER	};
cvar_t sv_idealpitchscale =		{"sv_idealpitchscale",	"0.8"					};
cvar_t net_ipmasking =			{"net_ipmasking",		"1",		CVAR_SERVER }; //Baker 3.95: R00k
cvar_t net_maxclientsperip =	{"net_maxclientsperip",		"0",		CVAR_SERVER }; //Baker 3.95: R00k
cvar_t sv_maxspeed =			{"sv_maxspeed",			"320",		CVAR_SERVER	};
cvar_t sv_maxvelocity =			{"sv_maxvelocity",		"2000"					};
cvar_t sv_nostep =				{"sv_nostep",			"0"						};
cvar_t sv_stopspeed =			{"sv_stopspeed",		"100"					};


#if SUPPORTS_EXTERNAL_ENTS
cvar_t external_ents =		{"external_ents",		"1"						};
#endif
#if SUPPORTS_EXTERNAL_VIS
cvar_t external_vis =		{"external_vis",		"1"						};
#endif

//#ifdef _DEBUG	// Serves no practical real world use except engine testing
cvar_t sv_cullentities_notify = {"sv_cullentities_notify", "0",		CVAR_SERVER	};		// in event there are multiple modes for anti-wallhack (Rook has a more comprehensive mode)
//#endif

// From DarkPlaces
cvar_t sv_bouncedownslopes = {"sv_gameplayfix_grenadebouncedownslopes", "1"};
cvar_t sv_freezenonclients =	{"sv_freezenonclients", "0"};

// **** SYSTEM ***********************************************************************************************************

//cvar_t sys_highpriority = {"sys_highpriority", "0", false, /**/OnChange_sys_highpriority}; // Baker 3.99r: sometimes this worsens online performance, so not saving this to config



// Nehahra Support
//cvar_t gl_notrans	=	{"gl_notrans",	"0"};
cvar_t r_oldsky		=	{"r_oldsky",	"1"};
//cvar_t r_nospr32	=	{"nospr32",		"0"};

cvar_t pr_neh_nehx00		=	{"nehx00",		"0"};
cvar_t pr_neh_nehx01		=	{"nehx01",		"0"};
cvar_t pr_neh_nehx02		=	{"nehx02",		"0"};
cvar_t pr_neh_nehx03		=	{"nehx03",		"0"};
cvar_t pr_neh_nehx04		=	{"nehx04",		"0"};
cvar_t pr_neh_nehx05		=	{"nehx05",		"0"};
cvar_t pr_neh_nehx06		=	{"nehx06",		"0"};
cvar_t pr_neh_nehx07		=	{"nehx07",		"0"};
cvar_t pr_neh_nehx08		=	{"nehx08",		"0"};
cvar_t pr_neh_nehx09		=	{"nehx09",		"0"};
cvar_t pr_neh_nehx10		=	{"nehx10",		"0"};
cvar_t pr_neh_nehx11		=	{"nehx11",		"0"};
cvar_t pr_neh_nehx12		=	{"nehx12",		"0"};
cvar_t pr_neh_nehx13		=	{"nehx13",		"0"};
cvar_t pr_neh_nehx14		=	{"nehx14",		"0"};
cvar_t pr_neh_nehx15		=	{"nehx15",		"0"};
cvar_t pr_neh_nehx16		=	{"nehx16",		"0"};
cvar_t pr_neh_nehx17		=	{"nehx17",		"0"};
cvar_t pr_neh_nehx18		=	{"nehx18",		"0"};
cvar_t pr_neh_nehx19		=	{"nehx19",		"0"};
cvar_t pr_neh_cutscene		=	{"cutscene",	"1"};



#if SUPPORTS_FTESTAINS
//qbism fte stain cvars
cvar_t r_stains_fadeamount = {"stain_fadeamount", "1"};
cvar_t r_stains_fadetime = {"stain_fadetime", "1"};
cvar_t r_stains = {"r_stains", "1"}; //zero to one
#endif


// Boss cvars

cvar_t cl_ent_disable_blood		= {"ent_disable_blood",	"0", CVAR_EXTERNAL};
cvar_t user_effectslevel		= {"cl_effectslevel",	"0", CVAR_EXTERNAL, /**/OnChange_cl_effectslevel};	// Default = unaware

qbool OnChange_cl_effectslevel (cvar_t *var, const char *string)
{
	int newlevel = CLAMP (-2, atoi(string), 1);

	switch (newlevel)
	{
	case -2:		// Customizable
		break;		// We just don't care

	case -1:		// Minimum
		Cvar_SetFloatByRef (&r_pass_caustics,			0);
		Cvar_SetFloatByRef (&r_pass_detail,		0);
//		Cvar_SetFloatByRef (&gl_fogenable,			0);			// Maybe needs something to absolutely disable fog, this cvar is reset between levels and affects underwater fog
		Cvar_SetFloatByRef (&external_lits,		1);
		Cvar_SetFloatByRef (&qmb_blobexplosions,			0);
		Cvar_SetFloatByRef (&qmb_blood,			0);
		Cvar_SetFloatByRef (&qmb_explosions,	0);
		Cvar_SetFloatByRef (&qmb_flames,		0);
		Cvar_SetFloatByRef (&qmb_gunshots,		0);
		Cvar_SetFloatByRef (&qmb_inferno,		0);
		Cvar_SetFloatByRef (&qmb_lavasplash,	0);
		Cvar_SetFloatByRef (&qmb_lightning,		0);
		Cvar_SetFloatByRef (&qmb_spikes,		0);
		Cvar_SetFloatByRef (&qmb_spiketrails,	0);
		Cvar_SetFloatByRef (&qmb_telesplash,	0);
		Cvar_SetFloatByRef (&qmb_trails,		0);
//		Cvar_SetFloatByRef (&particle_blend,		0);		// Baker: undesirable (triangular ... makes no speed diff anyway)
		Cvar_SetFloatByRef (&r_vertex_lights,		0);
		Cvar_SetFloatByRef (&r_water_fog,			0);
		Cvar_SetFloatByRef (&r_water_fog_density,	0);

		Cvar_SetFloatByRef (&scene_polyblend,			0);
		Cvar_SetFloatByRef (&light_flashblend,			2);

		Cvar_SetFloatByRef (&scene_lerpmodels,0);		// Interpolation in multiplayer is bad
//		Cvar_SetFloatByRef (&scene_lerpmove,0);		// Interpolation in multiplayer is bad ... what about monsters though?

		Cvar_SetFloatByRef (&r_shadows,				0);		// We don't want this on except for "maximum"
		break;

	case  0:	// Automatic

		Cvar_SetFloatByRef (&r_pass_caustics,			1);		// In single player only turn this on
		Cvar_SetFloatByRef (&r_pass_detail,		0);
//		Cvar_SetFloatByRef (&gl_fogenable,			0);		// Not in multiplayer
		Cvar_SetFloatByRef (&external_lits,		1);
		Cvar_SetFloatByRef (&qmb_blobexplosions,			1);
		Cvar_SetFloatByRef (&qmb_blood,			1);
		Cvar_SetFloatByRef (&qmb_explosions,	1);
		Cvar_SetFloatByRef (&qmb_flames,		1);
		Cvar_SetFloatByRef (&qmb_gunshots,		1);
		Cvar_SetFloatByRef (&qmb_inferno,		1);
		Cvar_SetFloatByRef (&qmb_lavasplash,	1);
		Cvar_SetFloatByRef (&qmb_lightning,		0);		// Ok, this is really bad for LG aiming.  Turn off in multiplayer
		Cvar_SetFloatByRef (&qmb_spikes,		1);
		Cvar_SetFloatByRef (&qmb_spiketrails,	1);
		Cvar_SetFloatByRef (&qmb_telesplash,	1);
		Cvar_SetFloatByRef (&qmb_trails,		1);
//		Cvar_SetFloatByRef (&particle_blend,		0);
		Cvar_SetFloatByRef (&r_vertex_lights,		1);
		Cvar_SetFloatByRef (&r_water_fog,			0);		// Water fog is distraction .. no multiplayer
		Cvar_SetFloatByRef (&r_water_fog_density,	0);		// Water fog is distraction

		Cvar_SetFloatByRef (&scene_polyblend,			1);
		Cvar_SetFloatByRef (&light_flashblend,			0);

		Cvar_SetFloatByRef (&scene_lerpmodels,			1);		// Detect if monsters > 0 or single player, if so turn on otherwise turn off
//		Cvar_SetFloatByRef (&scene_lerpmove,			1);		// Detect if monsters > 0 or single player, if so turn on otherwise turn off

		Cvar_SetFloatByRef (&r_shadows,				0);		// We don't want this on except for "maximum"
		break;

	case  1:	// Maximum

		Cvar_SetFloatByRef (&r_pass_caustics,			1);		// In single player only turn this on
		Cvar_SetFloatByRef (&r_pass_detail,		1);
//		Cvar_SetFloatByRef (&gl_fogenable,			1);		// Not in multiplayer
		Cvar_SetFloatByRef (&external_lits,		1);
		Cvar_SetFloatByRef (&qmb_blobexplosions,	1);
		Cvar_SetFloatByRef (&qmb_blood,			1);
		Cvar_SetFloatByRef (&qmb_blobexplosions,	1);
		Cvar_SetFloatByRef (&qmb_flames,		1);
		Cvar_SetFloatByRef (&qmb_gunshots,		1);
		Cvar_SetFloatByRef (&qmb_inferno,		1);
		Cvar_SetFloatByRef (&qmb_lavasplash,	1);
		Cvar_SetFloatByRef (&qmb_lightning,		1);		// Ok, this is really bad for LG aiming.  Turn off in multiplayer
		Cvar_SetFloatByRef (&qmb_spikes,		1);
		Cvar_SetFloatByRef (&qmb_spiketrails,	1);
		Cvar_SetFloatByRef (&qmb_telesplash,	1);
		Cvar_SetFloatByRef (&qmb_trails,		1);
//		Cvar_SetFloatByRef (&particle_blend,		0);
		Cvar_SetFloatByRef (&r_vertex_lights,		1);
		Cvar_SetFloatByRef (&r_water_fog,			2);		// Water fog is distraction .. no multiplayer
		Cvar_SetFloatByRef (&r_water_fog_density,	1);		// Water fog is distraction

		Cvar_SetFloatByRef (&scene_polyblend,			1);
		Cvar_SetFloatByRef (&light_flashblend,			0);

		Cvar_SetFloatByRef (&scene_lerpmodels,			1);		// Detect if monsters > 0 or single player, if so turn on otherwise turn off
//		Cvar_SetFloatByRef (&scene_lerpmove,			1);		// Detect if monsters > 0 or single player, if so turn on otherwise turn off

		Cvar_SetFloatByRef (&r_shadows,				1);		// We don't want this on except for "maximum"

		break;
	}

	return false; // Change accepted
}

#ifdef MACOSX
cvar_t	vid_overbright = { "gamma_overbright", "1", CVAR_ARCHIVE};
cvar_t	gl_anisotropic = { "gl_anisotropic", "0", CVAR_ARCHIVE };
cvar_t	gl_fsaa = { "gl_fsaa", "0", CVAR_ARCHIVE };
cvar_t	gl_truform = { "gl_truform", "-1", CVAR_ARCHIVE };
#endif
