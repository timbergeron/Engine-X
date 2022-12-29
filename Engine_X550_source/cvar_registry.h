/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2005 John Fitzgibbons and others

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
// cvar_registry.h


#ifndef __CVAR_REGISTRY_H__
#define __CVAR_REGISTRY_H__


extern cvar_t _windowed_mouse;
extern cvar_t keyboard_speed_angle_multiplier;
extern cvar_t keyboard_speed_move_multiplier;
extern cvar_t keyboard_speed_pitch;
extern cvar_t keyboard_speed_yaw;
extern cvar_t capture_codec;  // Include EVEN if does not support AVI capture in build!  Otherwise we lose the cvar saved setting
#if SUPPORTS_AVI_CAPTURE
extern cvar_t capture_console;
extern cvar_t capture_fps;
extern cvar_t capture_hack;
extern cvar_t capture_mp3;
extern cvar_t capture_mp3_kbps;
#endif
extern cvar_t chase_active;
extern cvar_t chase_back;
extern cvar_t chase_pitch;
extern cvar_t chase_right;
extern cvar_t chase_roll;
extern cvar_t chase_up;
extern cvar_t chase_yaw;
extern cvar_t cl_ent_deadbodyfilter;
extern cvar_t cl_ent_disable_blood;
extern cvar_t cl_ent_doubleeyes;
extern cvar_t cl_ent_gibfilter;
extern cvar_t gl_loadq3models;
extern cvar_t cl_ent_nocolors;
extern cvar_t cl_ent_nolerp;
extern cvar_t cl_ent_rotate_items_bob;
extern cvar_t cl_ent_truelightning;
extern cvar_t cl_net_color;
extern cvar_t cl_net_lag;
extern cvar_t cl_net_name;
extern cvar_t cl_print_shownet;
extern cvar_t cl_speed_back;
extern cvar_t cl_speed_forward;
extern cvar_t cl_speed_side;
extern cvar_t cl_speed_up;
extern cvar_t cl_web_download;
extern cvar_t cl_web_download_url;
extern cvar_t demorewind;
extern cvar_t demospeed;
extern cvar_t developer;
extern cvar_t developer_filter;
extern cvar_t developer_show_stufftext;
extern cvar_t external_ents;
extern cvar_t external_lits;
extern cvar_t external_textures;
extern cvar_t external_vis;
extern cvar_t game_kurok;
extern cvar_t gl_affinemodels;
extern cvar_t gl_clear;
extern cvar_t gl_cull;
extern cvar_t gl_finish;
extern cvar_t gl_polygonoffset_factor;
extern cvar_t gl_polygonoffset_offset;
extern cvar_t gl_smoothmodels;
extern cvar_t host_clearmodels;
extern cvar_t host_framerate;
extern cvar_t host_fullpitch;
extern cvar_t host_maxfps;
#if VARIABLE_EDICTS_AND_ENTITY_SIZE
extern cvar_t host_maxedicts;
extern cvar_t host_maxedicts_pad;
#endif
extern cvar_t host_sleep;
extern cvar_t host_speeds;
extern cvar_t host_sub_missing_models;
extern cvar_t host_timescale;
extern cvar_t in_accel;
extern cvar_t in_filter;
extern cvar_t in_fovscale;
extern cvar_t in_freelook;
extern cvar_t in_invert_pitch;
extern cvar_t in_joystick;
extern cvar_t in_keymap;
extern cvar_t in_keypad;
extern cvar_t in_lookspring;
extern cvar_t in_lookstrafe;
extern cvar_t in_mouse;
extern cvar_t in_sensitivity;
extern cvar_t in_smartjump;
extern cvar_t joy_advanced;
extern cvar_t joy_advaxisr;
extern cvar_t joy_advaxisu;
extern cvar_t joy_advaxisv;
extern cvar_t joy_advaxisx;
extern cvar_t joy_advaxisy;
extern cvar_t joy_advaxisz;
extern cvar_t joy_flysensitivity;
extern cvar_t joy_flythreshold;
extern cvar_t joy_forwardsensitivity;
extern cvar_t joy_forwardthreshold;
extern cvar_t joy_name;
extern cvar_t joy_pitchsensitivity;
extern cvar_t joy_pitchthreshold;
extern cvar_t joy_sidesensitivity;
extern cvar_t joy_sidethreshold;
extern cvar_t joy_wwhack1;
extern cvar_t joy_wwhack2;
extern cvar_t joy_yawsensitivity;
extern cvar_t joy_yawthreshold;
extern cvar_t light_explosions;
extern cvar_t light_explosions_color;
extern cvar_t light_flashblend;
extern cvar_t light_lerplightstyle;
extern cvar_t light_muzzleflash;
extern cvar_t light_overbright;
extern cvar_t light_powerups;
extern cvar_t light_rockets;
extern cvar_t light_rockets_color;
extern cvar_t list_demos_hud_nodraw;
extern cvar_t list_models_r_additive;
extern cvar_t list_models_r_fence_qpal255;
extern cvar_t list_models_r_filter;
extern cvar_t list_models_r_fullbright;
extern cvar_t list_models_r_nolerp;
extern cvar_t list_models_r_noshadow;
extern cvar_t mouse_directinput;
extern cvar_t mouse_lookstrafe_speed_forward;
extern cvar_t mouse_speed_pitch;
extern cvar_t mouse_setparms;
extern cvar_t mouse_lookstrafe_speed_side;
extern cvar_t mouse_speed_yaw;
extern cvar_t mp3_enabled;
extern cvar_t mp3_volume;
extern cvar_t msg_haverl;
extern cvar_t msg_needrl;
extern cvar_t msg_needrox;
extern cvar_t msg_noweapons;
extern cvar_t msg_pent;
extern cvar_t msg_quad;
extern cvar_t msg_ring;
extern cvar_t msg_weapons;
extern cvar_t net_connecttimeout;
extern cvar_t net_ipmasking;
extern cvar_t net_maxclientsperip;
extern cvar_t net_messagetimeout;
extern cvar_t particle_blend;
extern cvar_t particle_explosiontype;
extern cvar_t particle_grenadetrail;
extern cvar_t particle_rockettrail;
extern cvar_t pq_password;
extern cvar_t pr_coop;
extern cvar_t pr_deathmatch;
extern cvar_t pr_fraglimit;
extern cvar_t pr_gamecfg;
extern cvar_t pr_noexit;
extern cvar_t pr_nomonsters;
extern cvar_t pr_pausable;
extern cvar_t pr_samelevel;
extern cvar_t pr_saved1;
extern cvar_t pr_saved2;
extern cvar_t pr_saved3;
extern cvar_t pr_saved4;
extern cvar_t pr_savedgamecfg;
extern cvar_t pr_scratch1;
extern cvar_t pr_scratch2;
extern cvar_t pr_scratch3;
extern cvar_t pr_scratch4;
extern cvar_t pr_skill;
extern cvar_t pr_teamplay;
extern cvar_t pr_temp1;
extern cvar_t pr_timelimit;
extern cvar_t pr_neh_cutscene;
extern cvar_t pr_neh_nehx00;
extern cvar_t pr_neh_nehx01;
extern cvar_t pr_neh_nehx02;
extern cvar_t pr_neh_nehx03;
extern cvar_t pr_neh_nehx04;
extern cvar_t pr_neh_nehx05;
extern cvar_t pr_neh_nehx06;
extern cvar_t pr_neh_nehx07;
extern cvar_t pr_neh_nehx08;
extern cvar_t pr_neh_nehx09;
extern cvar_t pr_neh_nehx10;
extern cvar_t pr_neh_nehx11;
extern cvar_t pr_neh_nehx12;
extern cvar_t pr_neh_nehx13;
extern cvar_t pr_neh_nehx14;
extern cvar_t pr_neh_nehx15;
extern cvar_t pr_neh_nehx16;
extern cvar_t pr_neh_nehx17;
extern cvar_t pr_neh_nehx18;
extern cvar_t pr_neh_nehx19;
extern cvar_t qmb_blobexplosions;
extern cvar_t qmb_blood;
extern cvar_t qmb_bounceparticles;
extern cvar_t qmb_clipparticles;
extern cvar_t qmb_explosions;
extern cvar_t qmb_flames;
extern cvar_t qmb_gunshots;
extern cvar_t qmb_inferno;
extern cvar_t qmb_lavasplash;
extern cvar_t qmb_lightning;
extern cvar_t qmb_spikes;
extern cvar_t qmb_spiketrails;
extern cvar_t qmb_telesplash;
extern cvar_t qmb_trails;
extern cvar_t r_brushmodels_with_world;
extern cvar_t r_drawentities;
extern cvar_t r_drawentities_alpha;
extern cvar_t r_drawmodels_alias;
extern cvar_t r_drawmodels_brush;
extern cvar_t r_drawmodels_sprite;
extern cvar_t r_drawparticles;
extern cvar_t r_drawviewmodel;
extern cvar_t r_fastsky;
extern cvar_t r_fog;
extern cvar_t r_lightmap;
extern cvar_t r_nodrawentnum;
extern cvar_t r_oldsky;
extern cvar_t r_pass_caustics;
extern cvar_t r_pass_detail;
extern cvar_t r_pass_lumas;
extern cvar_t r_pass_lumas_alias;
extern cvar_t r_pass_lumas_brush;
extern cvar_t r_shadows;
extern cvar_t r_skybox;
extern cvar_t r_skycolor;
extern cvar_t r_speeds;
extern cvar_t r_stains;
extern cvar_t r_stains_fadeamount;
extern cvar_t r_stains_fadetime;
extern cvar_t r_subdivide_size;
extern cvar_t r_vertex_blenddist;
extern cvar_t r_vertex_lights;
extern cvar_t r_viewmodel_fovscale;
extern cvar_t r_viewmodel_hackpos;
extern cvar_t r_viewmodel_offset;
extern cvar_t r_viewmodel_pitch;
extern cvar_t r_viewmodel_ringalpha;
extern cvar_t r_viewmodel_size;
extern cvar_t r_water_alpha;
extern cvar_t r_water_fog;
extern cvar_t r_water_fog_density;
extern cvar_t r_water_ripple;
extern cvar_t r_water_warp;
extern cvar_t rcon_password;
extern cvar_t rcon_server;
extern cvar_t scene_dynamiclight;
extern cvar_t scene_farclip;
extern cvar_t scene_fov_scale;
extern cvar_t scene_fov_scaleaspect;
extern cvar_t scene_fov_x;
extern cvar_t scene_fov_y;
extern cvar_t scene_invert_x;
extern cvar_t scene_lerpmodels;
extern cvar_t scene_lerpmove;
extern cvar_t scene_lockpvs;
extern cvar_t scene_novis;
extern cvar_t scene_polyblend;
extern cvar_t scene_viewsize;
extern cvar_t scr_centerprint_log;
extern cvar_t scr_centerprint_nodraw;
extern cvar_t scr_centerprint_time;
extern cvar_t scr_con_alpha;
extern cvar_t scr_con_chatlog; //pq_logbinds
extern cvar_t scr_con_chatlog_dequake;
extern cvar_t scr_con_chatlog_playerslot; //xpq_showedict;
extern cvar_t scr_con_chatlog_removecr;
extern cvar_t scr_con_chatlog_timestamp;
extern cvar_t scr_con_filter;
extern cvar_t scr_con_font;
extern cvar_t scr_con_scale;
extern cvar_t scr_con_size;
extern cvar_t scr_con_speed;
extern cvar_t scr_crosshair;
extern cvar_t scr_crosshair_alpha;
extern cvar_t scr_crosshair_color;
extern cvar_t scr_crosshair_exceptions;
extern cvar_t scr_crosshair_image;
extern cvar_t scr_crosshair_offset_x;
extern cvar_t scr_crosshair_offset_y;
extern cvar_t scr_crosshair_size;
extern cvar_t scr_crosshair_style;
extern cvar_t scr_hud_center;
extern cvar_t scr_hud_extrabuffer;
extern cvar_t scr_hud_scoreboard_pings;
extern cvar_t scr_hud_style;
extern cvar_t scr_hud_teamscores;
extern cvar_t scr_hud_timer;
extern cvar_t scr_menu_center;
extern cvar_t scr_menu_scale;
extern cvar_t scr_notify_chatsound;
extern cvar_t scr_notify_lines;
extern cvar_t scr_notify_time;
extern cvar_t scr_show_fps;
extern cvar_t scr_show_coords;
extern cvar_t scr_show_angles;
extern cvar_t scr_show_cheats;
extern cvar_t scr_show_skill;
extern cvar_t scr_show_location;
extern cvar_t scr_show_pause;
extern cvar_t scr_show_ram;
extern cvar_t scr_show_speed;
extern cvar_t scr_show_stats;
extern cvar_t scr_show_stats_length;
extern cvar_t scr_show_stats_small;
extern cvar_t scr_show_turtle;
extern cvar_t scr_victory_printspeed;
extern cvar_t screenshot_format;
extern cvar_t serverprofile;
extern cvar_t session_cmdline;
extern cvar_t session_confirmquit;
extern cvar_t session_quickstart;
extern cvar_t session_registered;
extern cvar_t session_savevars;
extern cvar_t snd_ambient_fade;
extern cvar_t snd_ambient_level;
extern cvar_t snd_loadas8bit;
extern cvar_t snd_mixahead;
extern cvar_t snd_noextraupdate;
extern cvar_t snd_nosound;
extern cvar_t snd_precache;
extern cvar_t snd_show;
extern cvar_t snd_volume;
extern cvar_t sv_accelerate;
extern cvar_t sv_aim;
extern cvar_t sv_allcolors;
extern cvar_t sv_altnoclip;
extern cvar_t sv_bouncedownslopes;
extern cvar_t sv_chat_changemute;
extern cvar_t sv_chat_connectmute;
extern cvar_t sv_chat_grace;
extern cvar_t sv_chat_rate;
extern cvar_t sv_cullentities;
extern cvar_t sv_cullentities_notify;
extern cvar_t sv_defaultmap;
extern cvar_t sv_edgefriction;
extern cvar_t sv_freezenonclients;
extern cvar_t sv_friction;
extern cvar_t sv_gravity;
extern cvar_t sv_hostname;
extern cvar_t sv_idealpitchscale;
extern cvar_t sv_maxspeed;
extern cvar_t sv_maxvelocity;
extern cvar_t sv_nostep;
extern cvar_t sv_pausable;
extern cvar_t sv_progs;
extern cvar_t sv_signon_plus; // xpq_cvar_cheatfree;
extern cvar_t sv_stopspeed;
extern cvar_t sys_ticrate;
extern cvar_t scr_con_font_smooth;
#if SUPPORTS_GL_DELETETEXTURES
extern cvar_t tex_free_on_newmap;
#endif
extern cvar_t tex_lerp;
extern cvar_t tex_palette_contrast;
extern cvar_t tex_palette_gamma;
extern cvar_t tex_picmip;
extern cvar_t tex_picmip_allmodels;
extern cvar_t tex_scene_texfilter;
extern cvar_t tool_showbboxes;
extern cvar_t tool_showspawns;
extern cvar_t tool_texturepointer;
extern cvar_t user_effectslevel;
extern cvar_t v_bob;
extern cvar_t v_bobcycle;
extern cvar_t v_bobside;
extern cvar_t v_bobsidecycle;
extern cvar_t v_bobsideup;
extern cvar_t v_bobup;
extern cvar_t v_centermove;
extern cvar_t v_centerspeed;
extern cvar_t v_gunkick;
extern cvar_t v_idlescale;
extern cvar_t v_ipitch_cycle;
extern cvar_t v_ipitch_level;
extern cvar_t v_iroll_cycle;
extern cvar_t v_iroll_level;
extern cvar_t v_iyaw_cycle;
extern cvar_t v_iyaw_level;
extern cvar_t v_kickpitch;
extern cvar_t v_kickroll;
extern cvar_t v_kicktime;
extern cvar_t v_pq_smoothcam;
extern cvar_t v_rollangle;
extern cvar_t v_rollspeed;
extern cvar_t v_smoothstairs;
extern cvar_t vb_bonusflash;
extern cvar_t vb_contentblend;
extern cvar_t vb_damagecshift;
extern cvar_t vb_pentcshift;
extern cvar_t vb_quadcshift;
extern cvar_t vb_ringcshift;
extern cvar_t vb_suitcshift;
extern cvar_t vid_bpp;
extern cvar_t vid_brightness_contrast;
extern cvar_t vid_brightness_gamma;
extern cvar_t vid_brightness_method;
#ifdef SUPPORTS_GLVIDEO_MODESWITCH
extern cvar_t vid_displayfrequency;
extern cvar_t vid_fullscreen;
extern cvar_t vid_height;
extern cvar_t vid_altenter_toggle;
extern cvar_t vid_width;
#endif
extern cvar_t vid_vsync;

#ifdef MACOSX
extern cvar_t	vid_overbright;
extern cvar_t	gl_anisotropic;
extern cvar_t	gl_fsaa;
extern cvar_t	gl_truform;
#endif


extern cvar_t modelviewer_name;
extern cvar_t modelviewer_framenum;
extern cvar_t modelviewer_skinnum;
extern cvar_t modelviewer_origin;
extern cvar_t modelviewer_axis;
extern cvar_t modelviewer_angles;
extern cvar_t modelviewer_scale;


void Cvar_Registration_Client_Audio_MP3 (void);
void Cvar_Registration_Client_Chase (void);
void Cvar_Registration_Client_Devices (void);
void Cvar_Registration_Client_Image (void);
void Cvar_Registration_Client_Input (void);
void Cvar_Registration_Client_Joystick (void);
void Cvar_Registration_Client_Keyboard (void);
void Cvar_Registration_Client_Main (void);
void Cvar_Registration_Client_Menu (void);
void Cvar_Registration_Client_Mouse (void);
void Cvar_Registration_Client_Mouse_DirectInput (void);
void Cvar_Registration_Client_Movie (void);
void Cvar_Registration_Client_Rendering_Options_2D (void);
void Cvar_Registration_Client_Rendering_Options_3D (void);
void Cvar_Registration_Client_Sbar (void);
void Cvar_Registration_Client_Screen (void);
void Cvar_Registration_Client_Sound (void);
void Cvar_Registration_Client_System (void);
void Cvar_Registration_Client_Video (void);
void Cvar_Registration_Client_View (void);
void Cvar_Registration_Client_ViewBlends (void);
void Cvar_Registration_Client_ViewModel (void);
void Cvar_Registration_Extension_Nehahra (void);
void Cvar_Registration_Host (void);
void Cvar_Registration_Host_Commands (void);
void Cvar_Registration_Host_Common (void);
void Cvar_Registration_Host_Keys (void);
void Cvar_Registration_Host_Network (void);
void Cvar_Registration_Host_Security (void);
void Cvar_Registration_Mixed_Console (void);	// Console isn't 100% client, like con_printf ... so ....
void Cvar_Registration_Server (void);
void Cvar_Registration_Server_PR (void);

#endif
