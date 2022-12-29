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
// cl_update_screen.c -- master for refresh, status bar, console, chat, notify, etc
// Baker: Validated 6-27-2011.  Just dot for recording.

#include "quakedef.h"
#ifdef SUPPORTS_AVI_CAPTURE
#include "movie.h"
#endif

void SCR_TileClear (void);
void SCR_DrawNotifyString (void);
void SCR_CheckDrawCenterString (void);
void SCR_DrawRam (void);
void SCR_DrawNet (void);
void SCR_DrawTurtle (void);
void SCR_DrawPause (void);
void SCR_DrawConsole (void);
static void SCR_CalcRefdef (void);
void SCR_SetUpToDrawConsole (int *my_console_forced_up);
void SCR_DrawLoading (void);
void SCR_DrawTest (void);

qbool SCR_DrawVolume (void);
void SCR_DrawFPS (void);
void SCR_DrawSpeed (void);
void SCR_DrawStats (void);
void SCR_DrawCoords (void);


extern qbool	scr_drawloading;
extern float	scr_centertime_off;
extern float	scr_disabled_time;
extern qbool	scr_drawdialog;
extern qbool	scr_initialized;

void SCR_BeginLoadingPlaque (void)
{
	Con_DevPrintf (DEV_PROTOCOL, "Begin loading plaque\n");

	S_StopAllSounds (true);

	if (cls.state != ca_connected || cls.signon != SIGNONS)
		return;

// redraw with no console and the loading plaque
	Con_ClearNotify ();
	scr_centertime_off = 0;
	scr_con_current = 0;

	scr_drawloading = true;
//	scr_fullupdate = 0;
	Sbar_Changed ();
	SCR_UpdateScreen ();
	scr_drawloading = false;

	scr_disabled_for_loading = true;
	scr_disabled_time = realtime;
//	scr_fullupdate = 0;
}

#if HIDE_DAMN_CONSOLE
extern qbool scr_disabled_for_newmap;
#endif
void SCR_EndLoadingPlaque (void)
{
	Con_DevPrintf (DEV_PROTOCOL, "Ending loading plaque\n");
//	if (scr_disabled_for_newmap == true)
//		scr_disabled_for_newmap = false;
//	else	// If newmap, we aren't doing this quite yet ... part of a vile Baker hack.
		scr_disabled_for_loading = false;
//	scr_fullupdate = 0;
	Con_ClearNotify ();
}

float CalcFov (float fov_x, float width, float height)
{
    float   x;

    if (fov_x < 1 || fov_x > 179)
            Sys_Error ("CalcFov: Bad fov (%f)", fov_x);

    x = width / tanf(fov_x / 360 * M_PI);
    return atanf(height / x) * 360 / M_PI;
}

extern vrect_t	scr_vrect;


//Must be called whenever vid changes
static void SCR_CalcRefdef (void)
{
	float		size;
#ifdef GLQUAKE
	int			max_screen_height;
	qbool		leave_room_for_sbar = false;
#else
	vrect_t vrect;
#endif

#define screen_width vid.width //; //"(developer.integer ? glwidth : vid.width)
#define screen_height vid.height //; //(developer.integer ? glheight : vid.height)


//	MessageBox(NULL,"Ref1","Stage1",MB_OK);

//	scr_fullupdate = 0;		// force a background redraw
	vid.recalc_refdef = 0;

// force the status bar to redraw ... Baker: obsolete ... it always will
// But maybe we shouldn't if we want to take advantage of gl_clear ?
// Well ... either way ... we need this
	Sbar_Changed ();

// bound viewsize
	if (scene_viewsize.floater < 30)
		Cvar_SetStringByRef (&scene_viewsize, "30");
	if (scene_viewsize.floater > 120)
		Cvar_SetStringByRef (&scene_viewsize, "120");

// bound field of view
	if (scene_fov_x.floater < 10)
		Cvar_SetStringByRef (&scene_fov_x, "10");
	if (scene_fov_x.floater > 170)
		Cvar_SetStringByRef (&scene_fov_x, "170");

	if (scene_fov_y.floater < 10)
		Cvar_SetStringByRef (&scene_fov_y, "10");
	if (scene_fov_y.floater > 170)
		Cvar_SetStringByRef (&scene_fov_y, "170");


// intermission is always full screen


	// Special situations ...
	//    cl.intermission means no sb_lines and a viewsize of 100
	//
	// Otherwise a viewsize of 120 means no sbar lines and size of 100 plus a fullscreen view
	//             viewsize of 110 means very little sbar and a size of 100 plus a fullscreen view
	//             viewsize of 100 means full status bar a size of 100

	if (cl.intermission)
	{
		leave_room_for_sbar = false;		// We aren't drawing it
		size = min(100, scene_viewsize.floater);		// Baker: this is kinda of stupid ... ignore viewsize during intermission?  Gay ...fixed
		sb_lines = 0;
	}
	else if (cls.demofile && cls.titledemo) // Title demo
	{
		leave_room_for_sbar = false;
		size = 100;
		sb_lines = 0;
	}
	else if (scene_viewsize.floater >=120 /*|| scr_hud_style.integer != 0*/)
	{
		leave_room_for_sbar = false;		// We aren't drawing it
		size = 100;
		sb_lines = 0;
	}
	else if (scene_viewsize.floater >=110)
	{
		leave_room_for_sbar = true;			// We draw some of it.
		size = 100;
		sb_lines = 24;
	}
	else if (scene_viewsize.floater >=100)
	{
		leave_room_for_sbar =true;
		size = 100;
		sb_lines = 24 + 16 + 8;
	}
	else
	{
		leave_room_for_sbar =true;
		size = scene_viewsize.floater;
		sb_lines = 24 + 16 + 8;
	}

	if (scr_hud_style.integer !=0 /* classic HUD */)
		leave_room_for_sbar = false; // Because it is a transparent overlay style

	size /= 100.0;		// Becomes a percent

	// Calculate screen height

	max_screen_height = screen_height;

	if (leave_room_for_sbar)
		max_screen_height = screen_height - sb_lines;

//	r_refdef.vrect.width = max(96, screen_width * size);	// Limit smallest size to 96 "pixels" .. for icons?  Which ... well ... aren't drawn in the gl game window ... so fixed.
//	if (r_refdef.vrect.width < 96)
//	{
//		size = 96.0 / r_refdef.vrect.width;
//		r_refdef.vrect.width = 96;	// min for icons
//	}

	r_refdef.vrect.width = screen_width * size;
	r_refdef.vrect.height = screen_height * size;

	if (r_refdef.vrect.height > max_screen_height)
		r_refdef.vrect.height = max_screen_height;			// Limit screenheight to maximum allowed

	// Center within viewport
	r_refdef.vrect.x = (screen_width - r_refdef.vrect.width) / 2;
	if (r_refdef.vrect.height == screen_height) // It is ALL ours ... no centering
		r_refdef.vrect.y = 0; //(max_screen_height - r_refdef.vrect.height) / 2;
	else
		r_refdef.vrect.y = (max_screen_height - r_refdef.vrect.height) / 2;

	switch (scene_fov_scale.integer)
	{
	case 0:	r_refdef.fov_x = scene_fov_x.floater;		// Standard Quake
			r_refdef.fov_y = CalcFov (r_refdef.fov_x, r_refdef.vrect.width, r_refdef.vrect.height);
			break;

			// Aspect ratio correct to 4:3 or whatever scr_fov_scaleaspect's value is
	case 1: r_refdef.fov_y = CalcFov (scene_fov_x.floater, r_refdef.vrect.height * scene_fov_scaleaspect.floater, r_refdef.vrect.height);
			r_refdef.fov_x = CalcFov (r_refdef.fov_y, r_refdef.vrect.height, r_refdef.vrect.width);
			break;

			// Aspect ratio correct favoring Y
	case 2: r_refdef.fov_x = CalcFov (scene_fov_x.floater, r_refdef.vrect.height * scene_fov_scaleaspect.floater, r_refdef.vrect.height);
			r_refdef.fov_y = CalcFov (r_refdef.fov_x, r_refdef.vrect.height, r_refdef.vrect.width);
			break;

	case -1:r_refdef.fov_x = scene_fov_x.floater;		// -1 use cvars
			r_refdef.fov_y = scene_fov_y.floater;
			break;
	}

	// Move to calc r_refdef sheesh!  This is an fov constant
	{
		extern float r_partscale;
		r_partscale = 0.004 * tanf(r_refdef.fov_x * (M_PI / 180) * 0.5f);
	}

//	Con_Printf ("FOV x and y:  %3.3f and %3.3f\n", r_refdef.fov_x, r_refdef.fov_y);
	scr_vrect = r_refdef.vrect;

}




qbool IsChangedPreserve (float *my_old_value, const float my_new_value)
{
	if (*my_old_value == my_new_value)
		return false;

	*my_old_value = my_new_value;
	return true; // It changed

}

//This is called every frame, and can also be called explicitly to flush text to the screen.
//WARNING: be very careful calling this from elsewhere, because the refresh needs almost the entire 256k of stack space!

int scr_showstuff_row;
extern float			v_blend[4];
void Viewport_PolyBlend_RGBAfv (const GLfloat *v);
		extern int sb_updates;
		extern qbool gl_clear_for_frame;


void SCR_UpdateScreen (void)
{
//	static qbool last_was_draw_dialog = false;
//	qbool draw_dialog_again = false;

//	if (last_was_draw_dialog && scr_drawdialog)
//		draw_dialog_again = true;
//
//	if (!scr_drawdialog)
//		last_was_draw_dialog = false;



	if (scr_disabled_for_loading) // /*|| (scr_disabled_for_newmap && scr_drawloading)*/ /*|| !game_initialized*/)
	{
		if (realtime - scr_disabled_time > 60)
			scr_disabled_for_loading = false; // Con_Printf ("load failed.\n");
		else
			return;
	}

//	if (scr_disabled_for_newmap && scr_drawloading)
//		return;

	if (!scr_initialized || !con_initialized)
		return;				// not initialized yet

#ifdef _WIN32 // don't suck up any cpu if minimized
	{	
//		extern	int	Minimized;

		if (Minimized)
			return;
	}
#endif

	vid.numpages = 2 + (scr_hud_extrabuffer.integer ? 1 : 0); //johnfitz -- in case gl_triplebuffer is not 0 or 1

//	scr_copytop = 0;
//	scr_copyeverything = 0;

	
	do		// check for vid changes
	{
		static float	old_screensize;
		static float	old_fov_x;
		static float	old_fov_y;
		static float    old_fov_scale;
		static float    old_fov_scaleaspect;
		static float	old_sbar;
	
		// Temporary anti Kurok title.dem hack
		if (cls.demofile && cls.titledemo && scene_viewsize.floater == 130 && old_screensize != scene_viewsize.floater)
		{
			if (!old_screensize)
				Cvar_SetFloatByRef (&scene_viewsize, 100);
			else
				Cvar_SetFloatByRef (&scene_viewsize, old_screensize);
		}

		if (IsChangedPreserve (&old_fov_x, scene_fov_x.floater))						vid.recalc_refdef = true;
		if (IsChangedPreserve (&old_fov_y, scene_fov_y.floater))						vid.recalc_refdef = true;
		if (IsChangedPreserve (&old_fov_scale, scene_fov_scale.floater))				vid.recalc_refdef = true;
		if (IsChangedPreserve (&old_fov_scaleaspect, scene_fov_scaleaspect.floater))	vid.recalc_refdef = true;
		if (IsChangedPreserve (&old_screensize, scene_viewsize.floater))				vid.recalc_refdef = true;
		if (IsChangedPreserve (&old_sbar, scr_hud_style.floater))				vid.recalc_refdef = true;
	
		if (!vid.recalc_refdef) continue;
		
		SCR_CalcRefdef ();
		Sbar_Changed ();
	} while (0);

	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	SCR_SetUpToDrawConsole (&con_forcedup);

	do		// do 3D refresh drawing, and then update the screen
	{
//		if (con_forcedup)	continue;

		View_RenderView ();

	} while (0);

	do
	{
	GL_Set2D ();

		//  Baker: I do not want this here ... it is part of view .... Yikes this depends on 2D canvas!!!!!!
			Viewport_PolyBlend_RGBAfv (v_blend);

	// draw any areas not covered by the refresh
		// If we are performing a clear, we need to draw the sbar every frame.
		if (gl_clear_for_frame)
			Sbar_Changed ();
		else if (engine.HWGamma && !engine.HWGamma->hwgamma_enabled && vid_brightness_contrast.floater > 1)
			Sbar_Changed ();	// Baker: Because the brightening pass with continue and continue and continue on an area not updated per frame
#pragma message ("OSX fixme!!!! engine.HWGamma is null")		
		if (sb_updates == 0) // Otherwise if we aren't clearing and we are drawing the sbar, we need to draw the tile ONLY if the sbar is being updated
			SCR_TileClear ();	

		if (scr_drawloading)
		{
			//loading
			SCR_DrawLoading ();
	//		Sbar_Draw ();		// Baker: why were we drawing the sbar on loading??  For in-between levels
		}
		else if (cl.intermission == 1 && key_dest == key_game)
		{
			//end of level
			Sbar_IntermissionOverlay ();
//			SCR_DrawVolume ();
		}
		else if (cl.intermission == 2 && key_dest == key_game)
		{
			//end of episode
	
			Sbar_FinaleOverlay ();
			SCR_CheckDrawCenterString ();
//			SCR_DrawVolume ();
		}
		else
		{
			Draw_Crosshair ();
			SCR_DrawRam ();
			SCR_DrawNet ();
			SCR_DrawTurtle ();
			SCR_DrawPause ();
	//		SCR_DrawAutoID ();  Baker says: unimplemented as of now
			if (nehahra)
				SHOWLMP_drawall ();
			SCR_CheckDrawCenterString ();
	//		SCR_DrawLocalTime ();		// Baker: I don't like the drawing of the clock
	
			Draw_DisplayBox ();
//			if (SCR_DrawVolume ())
//				scr_showstuff_row = -1; 	// We don't draw fps or skill level if volume indicator is shown
//			else
//				scr_showstuff_row = 1;
//	
			Sbar_Draw ();
//			if (scr_showstuff_row >= 0)
//				scr_showstuff_row = 2;
	
//			SCR_DrawFPS ();
	
//			if (scr_showstuff_row >= 0)
//			{
//				// Draw locs
//				// Draw position
//	
//	
//	
//			}
	
	
			SCR_DrawSpeed ();
//			SCR_DrawStats ();  //Baker: where is this drawing in GL?
//			SCR_DrawCoords ();				// Baker: draw coords if developer 2 or higher
			SCR_DrawConsole ();
	
			M_Draw ();
		}

		if (scr_drawdialog)
		{
			Draw_FadeScreen ();
			SCR_DrawNotifyString ();
		}

	} while (0);
#ifndef MACOSX
	R_BrightenScreen ();
#endif
#pragma message ("OSX needs this especially if we can't figure out a way to do contrast in OSX, but see RMQEngine for SDL method?")
	
#ifndef MACOSX
	Gamma_Maybe_Update (); // V_UpdatePalette ();
#endif
#pragma message ("Gamma_Maybe_Update OSX Fix me?")


#if SUPPORTS_AVI_CAPTURE
	Movie_UpdateScreen ();
#endif

	GL_EndRendering ();

}

