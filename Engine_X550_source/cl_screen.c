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
// cl_screen.c -- master for refresh, status bar, console, chat, notify, etc
// Baker: Validated 6-27-2011.  Just screenshot auto-name.

#include "quakedef.h"

#if SUPPORTS_GLVIDEO_MODESWITCH
#include "winquake_video_modes.h"
#endif

#ifdef SUPPORTS_AVI_CAPTURE
#include "movie.h"
#endif

viddef_t	vid;				// global video state OSX no have vid_wgl.c so ....


#ifdef GLQUAKE
int			glx, gly, glwidth, glheight;
int			sb_lines;
#else
//viddef_t	vid;				// global video state
//vrect_t		*pconupdate;  Baker: JoeQuake does not use this
qbool		scr_skipupdate;
#endif

extern byte	current_pal[768];	// Tonik
// only the refresh window will be updated unless these variables are flagged
//int		scr_copytop;
//int		scr_copyeverything;

float		scr_con_current;
float		scr_conlines;		// lines of console to display
qbool		scr_skipupdate;		// Used to skip drawing, for example if app is minimized

qbool		scr_drawdialog;     // Baker: this always seems to be false???


qbool		scr_initialized;		// ready to draw

mpic_t		*scr_ram;
mpic_t		*scr_net;
mpic_t		*scr_turtle;

//int		scr_fullupdate;

int			clearconsole;
int			clearnotify;



vrect_t		scr_vrect;

qbool		scr_disabled_for_loading;
#if HIDE_DAMN_CONSOLE
qbool   	scr_disabled_for_newmap;
#endif

qbool		scr_drawloading;
float		scr_disabled_time;

//qbool	block_drawing;


/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

char		scr_centerstring[1024];
float		scr_centertime_start;	// for slow victory printing
float		scr_centertime_off;
int			scr_center_lines;
int			scr_erase_lines;
int			scr_erase_center;

//Called for important messages that should stay in the center of the screen for a few moments
void SCR_CenterPrint (const char *myString)
{
	const char *strwalk = myString;
	StringLCopy (scr_centerstring, myString);
//	Con_Printf ("size of centersize is %i and sizeof source str is %i and centerstring size is %i", strlen(scr_centerstring), strlen(str), sizeof(scr_centerstring));
	scr_centertime_off = scr_centerprint_time.floater;
	scr_centertime_start = cl.time;

// count the number of lines for centering
	scr_center_lines = 1;
	while (*strwalk)
	{
		if (*strwalk == '\n')
			scr_center_lines++;
		strwalk++;
	}
}

void SCR_DrawCenterString (void)
{
	char	*start;
	int	l, j, x, y, remaining;

// the finale prints the characters one at a time
	remaining = cl.intermission ? scr_victory_printspeed.floater * (cl.time - scr_centertime_start) : 9999;

	scr_erase_center = 0;
	start = scr_centerstring;

	y = (scr_center_lines <= 4) ? vid.height * 0.35 : 48;

	do
	{
	// scan the width of the line
		for (l=0 ; l<40 ; l++)
		{
			if (start[l] == '\n' || !start[l])
				break;
		}
		x = (vid.width - l*8) / 2;
		for (j=0 ; j<l ; j++, x+=8)
		{
			Draw_Character (x, y, start[j]);
			if (!remaining--)
				return;
		}

		y += 8;

		while (*start && *start != '\n')
			start++;

		if (!*start)
			break;
		start++;		// skip the \n
	} while (1);
}

extern qbool sb_showscores;
void SCR_CheckDrawCenterString (void)
{
//	scr_copytop = 1;
	if (scr_center_lines > scr_erase_lines)
		scr_erase_lines = scr_center_lines;

	scr_centertime_off -= host_frametime;

	if (scr_centertime_off <= 0 && !cl.intermission)
		return;
	if (key_dest != key_game)
		return;

//	if (sb_showscores && cl_scoreboard_clean.integer)
//		return;


	SCR_DrawCenterString ();
}

/************************************ FOV ************************************/



//Keybinding command
void SCR_SizeUp_f (void)
{
	Cvar_SetFloatByRef (&scene_viewsize, scene_viewsize.floater + 10);
	vid.recalc_refdef = 1;
}

//Keybinding command
void SCR_SizeDown_f (void)
{
	Cvar_SetFloatByRef (&scene_viewsize, scene_viewsize.floater - 10);
	vid.recalc_refdef = 1;
}

/********************************** ELEMENTS **********************************/

void SCR_DrawRam (void)
{
	if (!scr_show_ram.integer)
		return;

//	if (!r_cache_thrash)	// Not used in GL so this is always false and we always return
		return;

	Draw_Pic (scr_vrect.x+32, scr_vrect.y, scr_ram);
}

void SCR_DrawTurtle (void)
{
	static	int	count;

	if (!scr_show_turtle.integer)
		return;

	if (host_frametime < 0.1)
	{
		count = 0;
		return;
	}

	count++;
	if (count < 3)
		return;

	Draw_Pic (scr_vrect.x, scr_vrect.y, scr_turtle);
}

void SCR_DrawNet (void)
{
	if (realtime - cl.last_received_message < 0.3)
		return;

	if (cls.demoplayback)
		return;

	Draw_Pic (scr_vrect.x+64, scr_vrect.y, scr_net);
}

//extern int scr_showstuff_row;




void SCR_DrawSpeed (void)
{
	int		x, y;
	char		str[8];
	vec3_t		vel;
	float		speed;
	float		vspeed, speedunits;
	static	float	maxspeed = 0, display_speed = -1;
	static	double	lastrealtime = 0;

	if (!scr_show_speed.integer)
		return;

	if (lastrealtime > realtime)
	{
		lastrealtime = 0;
		display_speed = -1;
		maxspeed = 0;
	}

	VectorCopy (cl.velocity, vel);
	vspeed = vel[2];
	//vel[2] = 0;
	speed = VectorLength (vel);
#if 1
	{
		void GLDRAW_Speedometer (float x, float y, float radius, float mspeed);

		GLDRAW_Speedometer (vid.width  - 35, vid.height -35, 30, speed);

	}

	return;
#endif
	if (speed > maxspeed)
		maxspeed = speed;

	if (display_speed >= 0)
	{
		sprintf (str, "%3d", (int)display_speed);

		x = vid.width/2 - 80;

		if (scene_viewsize.floater >= 120)
			y = vid.height - 16;

		if (scene_viewsize.floater < 120)
			y = vid.height - 8*5;

		if (scene_viewsize.floater < 110)
			y = vid.height - 8*8;

		if (cl.intermission)
			y = vid.height - 16;

		Draw_Fill (x, y-1, 160, 1, 10);
		Draw_Fill (x, y+9, 160, 1, 10);
		Draw_Fill (x+32, y-2, 1, 13, 10);
		Draw_Fill (x+64, y-2, 1, 13, 10);
		Draw_Fill (x+96, y-2, 1, 13, 10);
		Draw_Fill (x+128, y-2, 1, 13, 10);

		Draw_Fill (x, y, 160, 9, 52);

		speedunits = display_speed;
		if (display_speed <= 500)
			Draw_Fill (x, y, (int)(display_speed/3.125), 9, 100);
		else {
			while (speedunits > 500)
				speedunits -= 500;
			Draw_Fill (x, y, (int)(speedunits/3.125), 9, 68);
		}
		Draw_String (x + 36 - strlen(str) * 8, y, str);
	}

	if (realtime - lastrealtime >= 0.1)
	{
		lastrealtime = realtime;
		display_speed = maxspeed;
		maxspeed = 0;
	}
}

#if 0 // Baker: I don't like the idea of the clock on the screen
void SCR_DrawLocalTime (void)
{
	time_t	ltime;
	char	str[80], *format;


	if (!scr_clock.integer)
		return;

	format = ((int)scr_clock.integer == 2) ? "%H:%M:%S" :
		 ((int)scr_clock.integer == 3) ? "%a %b %I:%M:%S %p" :
		 ((int)scr_clock.integer == 4) ? "%a %b %H:%M:%S" : "%I:%M:%S %p";

	time (&ltime);
	strftime (str, sizeof(str)-1, format, localtime(&ltime));

	if (scr_clock_y.integer < 0)
		Draw_String (8*scr_clock_x.integer, vid.height - sb_lines + 8*scr_clock_y.integer, str);
	else
		Draw_String (8*scr_clock_x.integer, 8*scr_clock_y.integer, str);
}
#endif

float	drawstats_limit;

/*
===============
SCR_DrawStats
===============
*/
void SCR_DrawStats (void)
{
	int		mins, secs, tens;
	extern	mpic_t	*sb_colon, *sb_nums[2][11];

	if (!scr_show_stats.integer || ((scr_show_stats.integer == 3 || scr_show_stats.integer == 4) && drawstats_limit < cl.time))
		return;

	mins = cl.ctime / 60;
	secs = cl.ctime - 60 * mins;
	tens = (int)(cl.ctime * 10) % 10;

	if (!scr_show_stats_small.integer)
	{
		Sbar_IntermissionNumber (vid.width - 140, 0, mins, 2, 0);

		Draw_TransPic (vid.width - 92, 0, sb_colon);
		Draw_TransPic (vid.width - 80, 0, sb_nums[0][secs/10]);
		Draw_TransPic (vid.width - 58, 0, sb_nums[0][secs%10]);

		Draw_TransPic (vid.width - 36, 0, sb_colon);
		Draw_TransPic (vid.width - 24, 0, sb_nums[0][tens]);

		if (scr_show_stats.integer == 2 || scr_show_stats.integer == 4)
		{
			Sbar_IntermissionNumber (vid.width - 48, 24, cl.stats[STAT_SECRETS], 2, 0);
			Sbar_IntermissionNumber (vid.width - 72, 48, cl.stats[STAT_MONSTERS], 3, 0);
		}
	}
	else
	{
		Draw_String (vid.width - 56, 0, va("%2i:%02i:%i", mins, secs, tens));
		if (scr_show_stats.integer == 2 || scr_show_stats.integer == 4)
		{
			Draw_String (vid.width - 16, 8, va("%2i", cl.stats[STAT_SECRETS]));
			Draw_String (vid.width - 24, 16, va("%3i", cl.stats[STAT_MONSTERS]));
		}
	}
}



/*
===============
SCR_DrawPlaybackStats
===============
*/
void SCR_DrawPlaybackStats (void)
{
#if 0
	int		yofs;
	static	float	pbstats_time = 0;

	if (!cls.demoplayback)
		return;

	if (realtime < pbstats_time - 2.0)
	{
		pbstats_time = 0;
		return;
	}

	if (pbstats_changed)
	{
		pbstats_time = realtime + 2.0;
		pbstats_changed = false;
	}
	else if (realtime > pbstats_time)
	{
		return;
	}

	if (scr_show_stats.integer == 1 || scr_show_stats.integer == 3)
		yofs = !scr_show_stats_small.integer ? 48 : 32;
	else if (scr_show_stats.integer == 2 || scr_show_stats.integer == 4)
		yofs = !scr_show_stats_small.integer ? 96 : 48;
	else
		yofs = 24;

	Draw_String (vid.width - 136, yofs, va("demo speed: %.1lfx", demospeed.floater));
	Draw_String (vid.width - 136, yofs + 8, va("playback mode: %s", demorewind.integer ? "rewind" : "forward"));
#endif
}

/* Not using this
void SCR_DrawDemoClock (void)
{
	int x, y;
	char str[80];

	if (!cls.demoplayback || !scr_democlock.integer)
		return;

	if (scr_democlock.integer == 2)
		StringLCopy (str, SecondsToHourString((int) (cls.demotime)), sizeof(str));
	else
		StringLCopy (str, SecondsToHourString((int) (cls.demotime - demostarttime)), sizeof(str));

	x = ELEMENT_X_COORD(scr_democlock);
	y = ELEMENT_Y_COORD(scr_democlock);
	Draw_String (x, y, str);
}
*/


void SCR_DrawPause (void)
{
	mpic_t	*pic;

	if (!scr_show_pause.integer)		// ability to turn off to make screeshots
		return;

	if (!cl.paused)
		return;

	pic = Draw_CachePic ("gfx/pause.lmp");
	Draw_Pic ((vid.width - pic->width) / 2, (vid.height - 48 - pic->height) / 2, pic);
}

void SCR_DrawLoading (void)
{
	mpic_t	*pic;

	if (!scr_drawloading)
		return;

	pic = Draw_CachePic ("gfx/loading.lmp");
	Draw_Pic ((vid.width - pic->width) / 2, (vid.height - 48 - pic->height) / 2, pic);
}



/********************************** CONSOLE **********************************/

void SCR_SetUpToDrawConsole (int *my_console_forced_up)
{
//	int my_console_forced_up;

	//johnfitz -- let's hack away the problem of slow console when host_timescale is <0
	Con_CheckResize ();

	if (scr_drawloading)
		return;		// never a console with loading plaque

// decide on the height of the console
	*my_console_forced_up = !cl.worldmodel || cls.signon != SIGNONS;

	if (*my_console_forced_up)
	{
		scr_conlines = vid.height;		// full screen
		scr_con_current = scr_conlines;
	}
	else if (key_dest == key_console)
	{
		scr_conlines = vid.height * scr_con_size.floater;
		scr_conlines = CLAMP (30, scr_conlines, vid.height);
	}
	else
	{
		scr_conlines = 0;			// none visible
	}

	if (scr_conlines < scr_con_current)
	{
		scr_con_current -= scr_con_speed.floater * host_frametime * vid.height / 320; //FIXME REALTIME SHEESH
		scr_con_current = max(scr_con_current, scr_conlines);
	}
	else if (scr_conlines > scr_con_current)
	{
		scr_con_current += scr_con_speed.floater * host_frametime * vid.height / 320; //FIXME REALTIME SHEESH
		scr_con_current = min(scr_con_current, scr_conlines);
	}

	if (clearconsole++ < vid.numpages)
	{
//#ifndef GLQUAKE
//		scr_copytop = 1;
//		Draw_TileClear (0,(int)scr_con_current,vid.width, vid.height - (int)scr_con_current);
//#endif
		Sbar_Changed ();
	}
	else if (clearnotify++ < vid.numpages)
	{
//#ifndef GLQUAKE
//		scr_copytop = 1;
//		Draw_TileClear (0,0,vid.width, con_notifylines);
//#endif
	}
	else
		con_notifylines = 0;
}


/*
===============
SCR_DrawCoords - Baker: display coords on-screen
===============
*/
void SCR_DrawCoords (void)
{
//	if (developer.integer < 2 bullshit)
//		return;
//
//	Draw_String (16, 16, va("Position xyz = %i %i %i",
//	(int)cl_entities[cl.player_point_of_view_entity].origin[0],
//	(int)cl_entities[cl.player_point_of_view_entity].origin[1],
//	(int)cl_entities[cl.player_point_of_view_entity].origin[2]));

}
void SCR_DrawConsole (void)
{
	if (scr_con_current)
	{
		Con_DrawConsole (scr_con_current, true);
		clearconsole = 0;
	}
	else
	{
		if (key_dest == key_game || key_dest == key_message)
			Con_DrawNotify ();	// only draw notify in game
	}
}

/********************************* TILE CLEAR *********************************/

void SCR_TileClear (void)
{
	// Baker: We have a problem that if the viewsize is less than 100 the overlay HUD needs to be cleared if hwgamma isn't being used
	//        Because the overlay HUD doesn't clear itself.
	extern qbool gl_clear_for_frame;
	int sbar_lines = (sb_lines>0 && engine.HWGamma && !engine.HWGamma->hwgamma_enabled && scr_hud_style.integer && !gl_clear_for_frame) ? 0 : sb_lines;

	// left, right
	if (r_refdef.vrect.x > 0) Draw_TileClear (0, 0, r_refdef.vrect.x, vid.height - sbar_lines);
	if (r_refdef.vrect.x > 0) Draw_TileClear (r_refdef.vrect.x + r_refdef.vrect.width, 0, vid.width - (r_refdef.vrect.x + r_refdef.vrect.width), vid.height - sbar_lines);

	// top then bottom
	if (r_refdef.vrect.y > 0) Draw_TileClear (r_refdef.vrect.x, 0, r_refdef.vrect.x + r_refdef.vrect.width, r_refdef.vrect.y);
	if (r_refdef.vrect.y > 0) Draw_TileClear (r_refdef.vrect.x, r_refdef.vrect.y + r_refdef.vrect.height, r_refdef.vrect.width, vid.height - sbar_lines - (r_refdef.vrect.height + r_refdef.vrect.y));

}




// Update the screen
// Update the screen
// Update the screen
// Update the screen

const char		*scr_notifystring;
qbool	scr_drawdialog;


void Text_Get_Columns_And_Rows (const char *myText, int *return_columns, int *return_rows)
{
	int i, maxcolumn = 0, numrows = 1, curcolumn = 0;

	for (i=0 ; i < strlen(myText) ; i++)
	{
		if (myText[i] == '\b')
			continue;	// Bronzing toggle ... we ignore this

		if (myText[i] == '\n')
		{
			numrows ++;
			curcolumn = 0;
			continue;
		}

		curcolumn ++;
		if (curcolumn > maxcolumn)
			(maxcolumn = curcolumn);
	}

	*return_columns = maxcolumn;
	*return_rows = numrows;

}

void Text_Print_At_XY (const char *MyText, int x_pixel_anchor, int y_pixel_anchor)
{
	int i;
	int x_coord = x_pixel_anchor;
	int y_coord = y_pixel_anchor;
	qbool bronze_on = false;

	for (i=0 ; i < strlen(MyText) ; i++)
	{
		qbool bronze_this_char=false;

		if (MyText[i] == '\b')
		{
			bronze_on = ! bronze_on;
			continue;
		}

		if (MyText[i] == '\n')
		{
			y_coord += 8;
			x_coord = x_pixel_anchor;
			continue;
		}

		if (bronze_on)
			if (MyText[i] > 32 && MyText[i] < 128) // Only bronze 33 to 127
				bronze_this_char = true;

		if (bronze_this_char)
			Draw_Character (x_coord, y_coord, MyText[i] | 128);
		else
			Draw_Character (x_coord, y_coord, MyText[i]);

		x_coord +=8;
	}

}

void Draw_FillRGBA (float x, float y, float w, float h, float red, float green, float blue, float alpha);
void SCR_DrawNotifyString (void)
{
//	char	*start;
	int		numcols = 0, numrows = 0;
	int		text_pixel_x;
	int		text_pixel_y;
	RECT	shadebox;

	// Find out the size of the text
	Text_Get_Columns_And_Rows (scr_notifystring, &numcols, &numrows);

	// Calculate RECT
	text_pixel_x	= (vid.width - numcols * 8) / 2;
	text_pixel_y	= vid.height * 0.35;
	shadebox.right	= (numcols + 8) * 8 + 4;
	shadebox.bottom	= (numrows + 8) * 8 + 4;
	shadebox.left   = text_pixel_x - 32 + 2;
	shadebox.top    = text_pixel_y - 32 + 2;

	Draw_FillRGBA	(shadebox.left, shadebox.top, shadebox.right, shadebox.bottom, 1,1,1,.5);
	Draw_FillRGBA	(shadebox.left+2, shadebox.top+2, shadebox.right-4, shadebox.bottom-4, 0,0,0,1);

	Text_Print_At_XY (scr_notifystring, text_pixel_x, text_pixel_y);
}

// Baker: Modified
// ESC always returns 0
// Otherwise returns strstr - string + 1
// So that's 1 for the first option
//           2 for the second option, etc.
int SCR_ModalMessage (const char *text, const float timeout, const char *altkeys)
{
	double			time1, time2; //johnfitz -- timeout
	const char		default_keys[3] = "ny";
	const char		*accepted_keys;
	int				exit_type = -1;
	char			last_char[2] = " ";

	if (altkeys)
		accepted_keys = altkeys;
	else
		accepted_keys = default_keys;

	if (cls.state == ca_dedicated)
		return true;

	scr_notifystring = text;

// draw a fresh screen
//	scr_fullupdate = 0;
	scr_drawdialog = true;
	SCR_UpdateScreen ();


	S_ClearBuffer ();		// so dma doesn't loop current sound

	time1 = Sys_DoubleTime () + timeout; //johnfitz -- timeout
	time2 = 0.0f; //johnfitz -- timeout

	do
	{

		key_count = -1;		// wait for a key down and up
		Sys_SendKeyEvents ();

		if (timeout) time2 = Sys_DoubleTime (); //johnfitz -- zero timeout means wait forever.

		last_char[0] = key_lastpress;

		if (strstr(accepted_keys, last_char))
		{
			exit_type = ((int)strstr(accepted_keys, last_char) - (int)accepted_keys + 1);
			exit_type = exit_type;
		}
		else if (key_lastpress == K_ESCAPE)
			exit_type = 0; // Abort
		else if (time2 >= time1)
			exit_type = 0; // Abort

		SCR_UpdateScreen ();

	} while (exit_type < 0 );

	scr_drawdialog = false;
	return exit_type;
}


/******************************** SCREENSHOTS ********************************/



qbool SCR_ScreenShot (char *absolute_screenshot_name)
{
	qbool		ok = false;
	int			buffersize = glwidth * glheight * RGB__BYTES_PER_PIXEL_IS_3;
	byte		*buffer;
	char		*ext;
//	char		prepared_absolute_name[MAX_OSPATH];

	ext = StringTemp_FileExtension (absolute_screenshot_name);
	ImageWork_Start ("screenshot", "screenshot");
	buffer = ImageWork_malloc (buffersize, "Screenshot");
	eglReadPixels (glx, gly, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, buffer);
#ifndef MACOSX
	Gamma_ApplyHWGammaToImage (buffer, buffersize, RGB__BYTES_PER_PIXEL_IS_3);
#endif
#pragma message ("OSX should do this")

	Con_DevPrintf (DEV_GAMEDIR, "Filename passed is %s\n", absolute_screenshot_name);
	#if SUPPORTS_LIBJPEG
	if (COM_StringMatchCaseless (ext, "jpg"))
		ok = Image_WriteJPEG (absolute_screenshot_name, 85 /* jpeg_compression_level.floater */, buffer + buffersize - 3 * glwidth, -glwidth, glheight);
	else
    #endif
    #if SUPPORTS_LIBPNG
	if (COM_StringMatchCaseless (ext, "png"))
		ok = Image_WritePNG (absolute_screenshot_name, 1 /* png_compression_level.floater */, buffer + buffersize - 3 * glwidth, -glwidth, glheight);
	else
	#endif
		ok = Image_WriteTGA (absolute_screenshot_name, buffer, glwidth, glheight, RGB__BYTES_PER_PIXEL_IS_3 * 8, false);

	ImageWork_free (buffer);

	ImageWork_Finish ();
	return ok;
}


#ifndef MACOSX // Move entire function to system or something sheesh ...
#include <windows.h>

void SCR_ScreenShot2_f (void)
{
	int myBPP = RGBA_BYTES_PER_PIXEL_IS_4;
	int buffersize = (glwidth * glheight * myBPP);
//	int i;
	byte *bmbits;
	HBITMAP hBitmap;

	ImageWork_Start ("Screenshot", "clipboard");
	bmbits = ImageWork_malloc (buffersize, "Screenshot");  // Special.  Won't need to be freed.

	eglReadPixels (glx, gly, glwidth, glheight, myBPP == RGB__BYTES_PER_PIXEL_IS_3 ? GL_RGB : GL_BGRA_EXT, GL_UNSIGNED_BYTE, bmbits);
    Gamma_ApplyHWGammaToImage (bmbits, buffersize, myBPP);

	// Ok ... we are upside down for what Windows wants
	FlipBuffer (bmbits, glwidth, glheight, myBPP);

	hBitmap= CreateBitmap (glwidth, glheight, 1, 8 * myBPP, bmbits);


	OpenClipboard (NULL);

	if (!EmptyClipboard())
	{
      CloseClipboard();
      return;
	}

	if ((SetClipboardData (CF_BITMAP, hBitmap)) == NULL)
	 Sys_Error ("SetClipboardData failed");

	CloseClipboard ();

	// Baker: This memory is transferred to Windows ownership now ...
	// Do not try to free it
	ImageWork_free (bmbits);
	Con_Printf("Screenshot to clipboard\n");

	ImageWork_Finish ();
}
#endif



/*
==================
SCR_ScreenShot_f
==================
*/
// This function is responsible for naming the screenshot and
// ensuring that the filename is available.
void SCR_ScreenShot_f (void)
{
	int		i, success;
	char	name[MAX_OSPATH];
	char	test[MAX_OSPATH];
	char	mapname[MAX_QPATH];
	char	timestr[80];
	char	ext[4];

	qbool format_determined = false;


	if (Cmd_Argc() > 2)
	{
		Con_Printf ("Usage: %s [filename]", Cmd_Argv(0));
		return;
	}

	if (Cmd_Argc() == 2)	// Name explicitly specified
	{
		snprintf (name, sizeof(name), "%s/%s", FOLDER_SHOTS, Cmd_Argv(1));
		goto takeshot;
	}

	// Auto name ... name not specified

	// Determine extension
#ifndef MACOSX
	if (COM_StringMatchCaseless (screenshot_format.string, "clipboard"))
	{
		SCR_ScreenShot2_f ();
		return;
	}
#endif
#pragma message ("Deal with clipboard screenshot in some way for OSX")
	// Determine extension

#if SUPPORTS_LIBPNG		// If we support PNG, check for that
	if (!format_determined && COM_StringMatchCaseless (screenshot_format.string, "png"))	{ StringLCopy (ext, "png"); format_determined = true;}
#endif
	if (!format_determined && COM_StringMatchCaseless (screenshot_format.string, "tga"))	{ StringLCopy (ext, "tga"); format_determined = true;}

#if SUPPORTS_LIBJPEG
	if (!format_determined && (COM_StringMatchCaseless (screenshot_format.string, "jpg") || COM_StringMatchCaseless (screenshot_format.string, "jpeg")))	{ StringLCopy (ext, "jpg"); format_determined = true;}
#endif

	if (!format_determined)	// Default it to jpg if available or tga if not
#if SUPPORTS_LIBJPEG
		StringLCopy (ext, "jpg");
#else
		StringLCopy (ext, "tga");
#endif

	// Determine naming convention

	if (con_forcedup) // No game going on
	{
		snprintf (test, sizeof(test), "%s", GENERIC_SCREENSHOT_NAME);
		goto no_game_occuring;
	}

	{ // Make time string
		int minutes = cl.time / 60;
		int seconds = cl.time - 60*minutes;
		int tens = seconds / 10;
		int units = seconds - 10*tens;
		snprintf (timestr, sizeof(timestr), "%1im%i%is", minutes, tens, units);
	}

	COM_Copy_StripExtension (cl.worldmodel->name + 5, mapname);

	if (cl.gametype == GAME_DEATHMATCH && cl.stats[STAT_TOTALMONSTERS] == 0 /*RQuake uses DM scoreboard */)
		snprintf (test, sizeof(test), "%s_frags_%02i_time_%s", mapname, cl.stats[STAT_FRAGS], timestr);
	else  // Single player naming convention
		if (sv.active) // Single player
			if (isGamedired) // "id1" game ... don't print gamedir
				snprintf (test, sizeof(test), "map_%s_skill_%s", mapname, SkillTextForCurrentSkill(false));
			else
				snprintf (test, sizeof(test), "game_%s_map_%s_skill_%s", com_gamedirshort, mapname, SkillTextForCurrentSkill(false));
		else // Networked
			snprintf (test, sizeof(test), "%s_networked_monsters_%i_of_%i", mapname, cl.stats[STAT_MONSTERS], cl.stats[STAT_TOTALMONSTERS]);

no_game_occuring:

	// file exists --- find one that doesn't exist by appending a number to the end

	// Check for file already existing.
	for (i=0 ; i<10000 ; i++)
	{
		int file_exists = 0;

		snprintf(name, sizeof(name), "%s/%s%_%04i.%s", FOLDER_SHOTS, test, i, ext);

		if ((file_exists = Sys_FileTime(name)) == -1)
			break;	// file doesn't exist
	}

	if (i == 10000)
	{
		Con_Printf ("SCR_ScreenShot_f: ERROR: Cannot create more than 10000 screenshots\n");
		return;
	}

takeshot:

	success = SCR_ScreenShot (name);
	Con_Printf ("%s '%s'\n", success ? "Wrote" : "Couldn't write", StringTemp_SkipPath(name));
}


// Baker 3.85:  This should really be located elsewhere, but duplicating it in both gl_screen.c and screen.c is silly.
//              Quakeworld has the equivalent in cl_cmd.c

#if 0
void CL_Fov_f (void)
{
	if (scene_fov_x.floater == 90.0 && default_fov.floater)
	{
		if (default_fov.floater == 90)
			return; // Baker 3.99k: Don't do a message saying default FOV has been set to 90 if it is 90!

		Cvar_SetFloatByRef (&scene_fov_x, default_fov.floater);
		Con_Printf("fov set to default_fov %s\n", default_fov.string);
	}
}
#endif
#if 0
static void CL_Default_fov_f (void)
{

	if (default_fov.floater == 0)
		return; // Baker: this is totally permissible and happens with Reset to defaults.

	if (default_fov.floater < 10.0 || default_fov.floater > 140.0)
	{
		Cvar_SetFloatByRef (&default_fov, 0.0f);
		Con_Printf("Default fov %s is out-of-range; set to 0\n", default_fov.string);
	}

}
#endif

// End Baker


static float	savedsensitivity;
static float	savedfov;

#pragma message ("Quality assurance: Rework zooming in a way that does not need to touch FOV.")
#pragma message ("Quality assurance: Changing zoom sensitivity is obsolete now.")
/*
================
CL_SaveFOV

Saves the FOV
================
*/
static void CL_SaveFOV_f (void)
{
	savedfov = scene_fov_x.floater;
}

/*
================
CL_RestoreFOV

Restores FOV to saved level
================
*/
static void CL_RestoreFOV_f (void)
{
	if (!savedfov)
	{
		Con_Printf("RestoreFOV: No saved FOV to restore\n");
		return;
	}

	Cvar_SetFloatByRef(&scene_fov_x, savedfov);
}
#pragma message ("Quality assurance: Perhaps implement Qrack zooming transition to knock out this function? And make a zoom_fov and zoom_fov_transition_seconds")
#pragma message ("Quality assurance: Wall collision crosshair option")

#if 0
/*
================
CL_SaveSensivity

Saves the Sensitivity
================
*/
static void CL_SaveSensitivity_f (void)
{
	savedsensitivity = in_sensitivity.floater;
}

/*
================
CL_RestoreSensitivity

Restores Sensitivity to saved level
================
*/
static void CL_RestoreSensitivity_f (void)
{
	if (!savedsensitivity)
	{
		Con_Printf("RestoreSensitivity: No saved SENSITIVITY to restore\n");
		return;
	}

	Cvar_SetFloatByRef(&sensitivity, savedsensitivity);
}
#endif


/*
=============
CL_Tracepos_f -- johnfitz

display impact point of trace along VPN
=============
*/

static void CL_Tracepos_f (void)
{
	vec3_t	v, w;
	extern void CL_TraceLine (vec3_t start, vec3_t end, vec3_t impact);

	VectorScale(vpn, 8192.0, v);
	CL_TraceLine(r_refdef.vieworg, v, w);

	if (VectorLength(w) == 0)
		Con_Printf ("Tracepos: trace didn't hit anything\n");
	else
		Con_Printf ("Tracepos: (%i %i %i)\n", (int)w[0], (int)w[1], (int)w[2]);
}

/*
=============
CL_Viewpos_f -- johnfitz

display client's position and angles
=============
*/
static void CL_Viewpos_f (void)
{
#if 0
	//camera position
	Con_Printf ("Viewpos: (%i %i %i) %i %i %i\n",
		(int)r_refdef.vieworg[0],
		(int)r_refdef.vieworg[1],
		(int)r_refdef.vieworg[2],
		(int)r_refdef.viewangles[PITCH],
		(int)r_refdef.viewangles[YAW],
		(int)r_refdef.viewangles[ROLL]);
#else
	//player position
	Con_Printf ("You are at xyz = %i %i %i   angles: %i %i %i\n",
		(int)cl_entities[cl.player_point_of_view_entity].origin[0],
		(int)cl_entities[cl.player_point_of_view_entity].origin[1],
		(int)cl_entities[cl.player_point_of_view_entity].origin[2],
		(int)cl.viewangles[PITCH],
		(int)cl.viewangles[YAW],
		(int)cl.viewangles[ROLL]);
#endif
}


/************************************ INIT ************************************/

void SCR_Init (void)
{
	Cvar_Registration_Client_Screen ();


//	Cvar_SetCurrentGroup(CVAR_GROUP_VIEW);
// register our commands
	Cmd_AddCommand ("screenshot", SCR_ScreenShot_f);
	Cmd_AddCommand ("sizeup", SCR_SizeUp_f);
	Cmd_AddCommand ("sizedown", SCR_SizeDown_f);

	Cmd_AddCommand ("tracepos", CL_Tracepos_f); //johnfitz
	Cmd_AddCommand ("viewpos", CL_Viewpos_f); //Baker 3.76 - Using FitzQuake's viewpos instead of my own

	Cmd_AddCommand ("savefov", CL_SaveFOV_f);
//	Cmd_AddCommand ("savesensitivity", CL_SaveSensitivity_f);
	Cmd_AddCommand ("restorefov", CL_RestoreFOV_f);
//	Cmd_AddCommand ("restoresensitivity", CL_RestoreSensitivity_f);


	scr_ram = Draw_PicFromWad ("ram");
	scr_net = Draw_PicFromWad ("net");
	scr_turtle = Draw_PicFromWad ("turtle");

#if SUPPORTS_AVI_CAPTURE
	Movie_Init ();
#endif

	scr_initialized = true;
}

// Baker 3.97: new scheme supercedes these
extern qbool vid_consize_ignore_callback;
/*
==================
VID_Consize_f -- Baker -- called when vid_consize changes
==================
*/
extern qpic_t *conback_sizer;
//qbool vid_smoothfont = false;
extern qbool smoothfont_init;

static int freepass = -2;
void VID_Consize_f(void)
{
	float startwidth;
	float startheight;
	float desiredwidth;
	int contype = freepass; //scr_con_scale.integer;
	int exception = 0;
#ifdef MACOSX
	extern unsigned int gGLDisplayWidth, gGLDisplayHeight;
	Con_DevPrintf (DEV_OPENGL, "VID_Consize_f hit\n");
#endif
	if (contype < -1) // We haven't been passed a value, so use consize
		contype = scr_con_scale.integer;

#pragma message ("Fix me urgent")

#ifdef MACOSX
	startwidth = vid.width = gGLDisplayWidth;
	startheight = vid.height = gGLDisplayHeight;
#else
	startwidth = vid.width = video_modes[vid_default].width;
	startheight = vid.height = video_modes[vid_default].height;
#endif

//	Con_Printf("Entering ...\n");
//	Con_Printf("vid.width is %d and vid.height is %d\n", vid.width, vid.height);
//	Con_Printf("vid.conwidth is %d and vid.conheight is %d\n", vid.conwidth, vid.conheight);

	// Baker 3.97
	// We need to appropriately set vid.width, vid.height, vid.smoothfont (?)

//	vid_smoothfont = false; // Assume it is unnecessary

	if (contype == -1)
	{
		// Automatic consize to avoid microscopic text
		if (vid.width>=1024)
			contype = 1;
		else
			contype = 0;
	}

	switch (contype)
	{

		case 0: // consize is width

			desiredwidth = vid.width;
			break;

		case 1: // consize is 50% width (if possible)

			// if resolution is < 640, must use the resolution itself.
			if (vid.width < 640)
			{
				exception = 1; // Notify later about console resolution unavailable
				desiredwidth = vid.width;
				break;
			}

			desiredwidth = (int)(vid.width/2);
			break;

		case 3:
			desiredwidth = 320;
			break;

		default:
			// If vid.width is under 640, must use 320?
			if (vid.width < 640)
			{
				exception = 2; // Notify later about console resolution unavailable
				desiredwidth = vid.width;
				break;
			}
			desiredwidth = 640;
			break;
	}

	vid.conwidth = CLAMP (320, desiredwidth, vid.width);
	vid.conwidth &= 0xFFFFFFF8;                      // But this must be a multiple of 8
	vid.conheight = vid.conwidth * vid.height / vid.width;  // Now set height using proper aspect ratio
	vid.conheight &= 0xFFFFFFF8;					  // This too must be a multiple of 8

	// Baker: this fixes when consize <200 auto-sizing
	if (vid.conheight< 200)
	{
		vid.conheight = CLAMP (200, vid.conheight, vid.height);
		Con_DevPrintf (DEV_VIDEO, "Console min adjusted: %dx%d\n", vid.conwidth, vid.conheight);
	}
	// Baker /end fix, but aspectratio is not right

	conback_sizer->width = vid.width = vid.conwidth; // = vid.width;
	conback_sizer->height = vid.height = vid.conheight; // = vid.height;

	//  Determine if smooth font is needed

/*Bakered:
	if ((int)(startwidth / vid.conwidth) == ((startwidth + 0.0f) / (vid.conwidth + 0.0f)) )
	{
		SmoothFontSet (false);
	}
	else
	{
		SmoothFontSet (true);
	}*/

	// Print messages AFTER console resizing to ensure they print right
	if (exception)
	{
		if (exception == 1)
			Con_DevPrintf(DEV_VIDEO, "VID_Consize_f: 50%% console size unavailable, using 100%% for this resolution.\n");
		else
			Con_DevPrintf(DEV_VIDEO, "VID_Consize_f: 640 console size unavailable, using 100%% for this resolution.\n");
	}


	freepass = -2; // Set back to "normal"
	vid.recalc_refdef = 1;

}

qbool OnChange_vid_consize (cvar_t *var, const char *string)
{
	freepass = atoi(string);
	VID_Consize_f();
	return false;
}
