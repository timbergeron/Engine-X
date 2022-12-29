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
// vid_modes_menu.c -- NT GL vid component


#include "quakedef.h"
#include "winquake.h"
#include <windows.h>
#include "winquake_video_modes.h"


//==========================================================================
//
//  NEW VIDEO MENU -- johnfitz
//
//==========================================================================

extern	void M_Menu_Options_f (void);
extern	void M_Print (int cx, int cy, char *str);
extern	void M_PrintWhite (int cx, int cy, char *str);
extern	void M_DrawCharacter (int cx, int line, int num);
extern	void M_DrawTransPic (int x, int y, mpic_t *pic);
extern	void M_DrawPic (int x, int y, mpic_t *pic);
extern void M_DrawCheckbox (int x, int y, int on);

extern qbool	m_entersound;


//extern int vid_bpp;
//extern int desktop_bpp;



#define VIDEO_OPTIONS_ITEMS 10 //6
int		video_cursor_table[] = {48, 56, 64, 72, 88, 96, 128, 136, 144, 152};
int		video_options_cursor = 0;

typedef struct {int width,height;} vid_menu_mode;

int vid_menu_rwidth;
int vid_menu_rheight;

//TODO: replace these fixed-length arrays with hunk_allocated buffers

vid_menu_mode vid_menu_modes[MAX_MODE_LIST];
int vid_menu_nummodes=0;

int vid_menu_bpps[4];
int vid_menu_numbpps=0;

int vid_menu_rates[20];
int vid_menu_numrates=0;

/*
================
VID_Menu_Init
================
*/
void VID_Menu_Init (void)
{
	int i,j,h,w;

	for (i=1;i<num_video_modes;i++) //start i at mode 1 because 0 is windowed mode
	{
		w = video_modes[i].width;
		h = video_modes[i].height;

		for (j=0;j<vid_menu_nummodes;j++)
{
			if (vid_menu_modes[j].width == w &&
				vid_menu_modes[j].height == h)
				break;
		}

		if (j==vid_menu_nummodes)
		{
			vid_menu_modes[j].width = w;
			vid_menu_modes[j].height = h;
			vid_menu_nummodes++;
		}
	}
}

/*
================
VID_Menu_RebuildBppList

regenerates bpp list based on current vid_width and vid_height
================
*/
void VID_Menu_RebuildBppList (void)
{
	int i,j,b;

	vid_menu_numbpps=0;

	for (i=1;i<num_video_modes;i++) //start i at mode 1 because 0 is windowed mode
	{
		//bpp list is limited to bpps available with current width/height
		if (video_modes[i].width != vid_width.integer ||
			video_modes[i].height != vid_height.integer)
			continue;

		b = video_modes[i].bpp;

		for (j=0;j<vid_menu_numbpps;j++)
		{
			if (vid_menu_bpps[j] == b)
				break;
		}

		if (j==vid_menu_numbpps)
		{
			vid_menu_bpps[j] = b;
			vid_menu_numbpps++;
		}
	}

	//if there are no valid fullscreen bpps for this width/height, just pick one
	if (vid_menu_numbpps == 0)
	{
		Cvar_SetFloatByRef (&vid_bpp, (float)video_modes[0].bpp);
		return;
	}

	//if vid_bpp is not in the new list, change vid_bpp
	for (i=0;i<vid_menu_numbpps;i++)
		if (vid_menu_bpps[i] == (int)(vid_bpp.integer))
			break;

	if (i==vid_menu_numbpps)
		Cvar_SetFloatByRef (&vid_bpp, (float)vid_menu_bpps[0]);
}

/*
================
VID_Menu_RebuildRateList

regenerates rate list based on current vid_width, vid_height and vid_bpp
================
*/
void VID_Menu_RebuildRateList (void)
{
	int i,j,r;

	vid_menu_numrates=0;

	for (i=1;i<num_video_modes;i++) //start i at mode 1 because 0 is windowed mode
	{
		//rate list is limited to rates available with current width/height/bpp
		if (video_modes[i].width != vid_width.integer ||
			video_modes[i].height != vid_height.integer ||
			video_modes[i].bpp != vid_bpp.integer)
			continue;

		r = video_modes[i].refreshrate;

		for (j=0;j<vid_menu_numrates;j++)
		{
			if (vid_menu_rates[j] == r)
				break;
		}

		if (j==vid_menu_numrates)
		{
			vid_menu_rates[j] = r;
			vid_menu_numrates++;
		}
	}

	//if there are no valid fullscreen refreshrates for this width/height, just pick one
	if (vid_menu_numrates == 0)
	{
		Cvar_SetFloatByRef (&vid_displayfrequency, (float)video_modes[0].refreshrate);
		return;
	}

	//if vid_displayfrequency is not in the new list, change vid_displayfrequency
	for (i=0;i<vid_menu_numrates;i++)
		if (vid_menu_rates[i] == (int)(vid_displayfrequency.integer))
			break;

	if (i==vid_menu_numrates)
		Cvar_SetStringByRef (&vid_displayfrequency,va("%i",vid_menu_rates[0]));
}

/*
================
VID_Menu_CalcAspectRatio

calculates aspect ratio for current vid_width/vid_height
================
*/
void VID_Menu_CalcAspectRatio (void)
{
	int w,h,f;
	w = vid_width.integer;
	h = vid_height.integer;
	f = 2;
	while (f < w && f < h)
	{
		if ((w/f)*f == w && (h/f)*f == h)
		{
			w/=f;
			h/=f;
			f=2;
		}
		else
			f++;
	}
	vid_menu_rwidth = w;
	vid_menu_rheight = h;
}

/*
================
VID_Menu_ChooseNextMode

chooses next resolution in order, then updates vid_width and
vid_height cvars, then updates bpp and refreshrate lists
================
*/
void VID_Menu_ChooseNextMode (int dir)
{
	int i;

	for (i=0;i<vid_menu_nummodes;i++)
	{
		if (vid_menu_modes[i].width == vid_width.integer &&
			vid_menu_modes[i].height == vid_height.integer)
			break;
	}

	if (i==vid_menu_nummodes) //can't find it in list, so it must be a custom windowed res
	{
		i = 0;
	}
			else
	{
		i+=dir;
		if (i>=vid_menu_nummodes)
			i = 0;
		else if (i<0)
			i = vid_menu_nummodes-1;
	}

	Cvar_SetStringByRef (&vid_width,  va("%i",vid_menu_modes[i].width));
	Cvar_SetStringByRef (&vid_height, va("%i",vid_menu_modes[i].height));
	VID_Menu_RebuildBppList ();
	VID_Menu_RebuildRateList ();
	VID_Menu_CalcAspectRatio ();
}

/*
================
VID_Menu_ChooseNextBpp

chooses next bpp in order, then updates vid_bpp cvar, then updates refreshrate list
================
*/
void VID_Menu_ChooseNextBpp (int dir)
{
	int i;

	for (i=0;i<vid_menu_numbpps;i++)
	{
		if (vid_menu_bpps[i] == vid_bpp.integer)
			break;
	}

	if (i==vid_menu_numbpps) //can't find it in list
	{
		i = 0;
	}
	else
	{
		i+=dir;
		if (i>=vid_menu_numbpps)
			i = 0;
		else if (i<0)
			i = vid_menu_numbpps-1;
	}

	Cvar_SetFloatByRef (&vid_bpp, (float)vid_menu_bpps[i]);
	VID_Menu_RebuildRateList ();
}

/*
================
VID_Menu_ChooseNextRate

chooses next refresh rate in order, then updates vid_displayfrequency cvar
================
*/
void VID_Menu_ChooseNextRate (int dir)
{
	int i;

	for (i=0;i<vid_menu_numrates;i++)
	{
		if (vid_menu_rates[i] == vid_displayfrequency.integer)
			break;
	}

	if (i==vid_menu_numrates) //can't find it in list
	{
		i = 0;
	}
	else
	{
		i+=dir;
		if (i>=vid_menu_numrates)
			i = 0;
		else if (i<0)
			i = vid_menu_numrates-1;
}

	Cvar_SetStringByRef (&vid_displayfrequency, va("%i",vid_menu_rates[i]));
}

/*
================
VID_MenuKey
================
*/
void VID_MenuKey (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		VID_SyncCvarsToMode (vid_default); //sync cvars before leaving menu. FIXME: there are other ways to leave menu
		S_LocalSound ("misc/menu1.wav");
		M_Menu_Options_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		video_options_cursor--;
		if (video_options_cursor < 0)
			video_options_cursor = VIDEO_OPTIONS_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		video_options_cursor++;
		if (video_options_cursor >= VIDEO_OPTIONS_ITEMS)
			video_options_cursor = 0;
		break;

	case K_LEFTARROW:
		S_LocalSound ("misc/menu3.wav");
		switch (video_options_cursor)
		{
		case 0:
			VID_Menu_ChooseNextMode (-1);
			break;
		case 1:
			VID_Menu_ChooseNextBpp (-1);
			break;
		case 2:
			VID_Menu_ChooseNextRate (-1);
			break;
		case 3:
			Cbuf_AddText ("toggle vid_fullscreen\n");
			break;
		case 4:
		case 5:
			break;
		case 6:
			Cbuf_AddText ("cycle vid_consize -1 0 1 2");
			break;
		case 7:
			Cbuf_AddText ("toggle vid_vsync");
			break;
		case 8:
			Cbuf_AddText ("cycle pq_maxfps 72 100 150 250");
			break;
		case 9:
			Cbuf_AddText ("toggle show_fps");
			break;



		default:
		break;
	}
		break;

	case K_RIGHTARROW:
		S_LocalSound ("misc/menu3.wav");
		switch (video_options_cursor)
		{
		case 0:
			VID_Menu_ChooseNextMode (1);
			break;
		case 1:
			VID_Menu_ChooseNextBpp (1);
			break;
		case 2:
			VID_Menu_ChooseNextRate (1);
			break;
		case 3:
//			if (vid_bpp.integer == vid.DesktopBPP)
				Cbuf_AddText ("toggle vid_fullscreen\n");
			break;
		case 4:
		case 5:
			break;
		case 6:
			Cbuf_AddText ("cycle vid_consize -1 0 1 2");
			break;
		case 7:
			Cbuf_AddText ("toggle vid_vsync");
			break;
		case 8:
			Cbuf_AddText ("cycle pq_maxfps 72 100 150 250");
			break;
		case 9:
			Cbuf_AddText ("toggle show_fps");
			break;


		default:
			break;
		}
		break;

	case K_ENTER:
		m_entersound = true;
		switch (video_options_cursor)
		{
		case 0:
			VID_Menu_ChooseNextMode (1);
			break;
		case 1:
//			SCR_ModalMessage("Colordepth (bits per pixel) must be set\nusing -bpp 16 or -bpp 32 from the\ncommand line.\n\nPress Y or N to continue.",0.0f, NULL);
			VID_Menu_ChooseNextBpp (1);
			break;
		case 2:
			VID_Menu_ChooseNextRate (1);
			break;
		case 3:
//			if (vid_bpp.integer == vid.DesktopBPP)
				Cbuf_AddText ("toggle vid_fullscreen\n");
//			else
//				SCR_ModalMessage("Changing between fullscreen and\nwindowed mode is not available\nbecause your color depth does\nnot match the desktop.\n\nRemove -bpp from your command line\nto have this available.\n\nPress Y or N to continue.",0.0f);

			break;

		case 4:
			Cbuf_AddText ("video_test\n");
			break;
		case 5:
			Cbuf_AddText ("video_restart\n");
			break;
		case 6:
			Cbuf_AddText ("cycle vid_consize -1 0 1 2");
			break;
		case 7:
			Cbuf_AddText ("toggle vid_vsync");
			break;
		case 8:
			Cbuf_AddText ("cycle pq_maxfps 72 100 150 250");
			break;
		case 9:
			Cbuf_AddText ("toggle show_fps");
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

/*
================
VID_MenuDraw
================
*/
void VID_MenuDraw (void)
{
	int i = 0;
	mpic_t *p;
	char *title;
//	int  aspectratio1;
	int d3d_explain=0;
	qbool no_titlebar_window = false;
	qbool window_buster = (  !vid_fullscreen.integer && ((vid_width.integer > (vid.DesktopWidth - 64)) || (vid_height.integer > (vid.DesktopHeight - 64))));
	// If window_buster is exactly desktop settings, then it is ok
	if (window_buster && vid_width.integer == vid.DesktopWidth && vid_height.integer == vid.DesktopHeight)
	{
		window_buster = false;
		no_titlebar_window = true;
	}
	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp"));

	//p = Draw_CachePic ("gfx/vidmodes.lmp");
	p = Draw_CachePic ("gfx/p_option.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	// title
	title = "Set Video Mode";
	M_PrintWhite ((320-8*strlen(title))/2, 32, title);
	title = "Video Options";
	M_PrintWhite ((320-8*strlen(title))/2, 112, title);

	if (developer.integer)
		M_PrintWhite(8,8, va("%i", video_options_cursor));

	// options
	M_Print (16, video_cursor_table[i], "         Video mode");
	M_Print (184, video_cursor_table[i], va("%ix%i (%i:%i)", (int)vid_width.integer, (int)vid_height.integer, vid_menu_rwidth, vid_menu_rheight));
	i++;

	M_Print (16, video_cursor_table[i], "        Color depth");
	M_Print (184, video_cursor_table[i], va("%i", (int)vid_bpp.integer));
	i++;

	M_Print (16, video_cursor_table[i], "       Refresh rate");
#if RENDERER_DIRECT3D_AVAILABLE
	if (engine.Renderer->graphics_api == RENDERER_DIRECT3D)
		M_Print (184, video_cursor_table[i], va("[%i Hz locked]", vid.DesktopDispFreq));
	else
#endif
		M_Print (184, video_cursor_table[i], va("%i Hz", (int)vid_displayfrequency.integer));
	i++;

	M_Print (16, video_cursor_table[i], "         Fullscreen");

	if (no_titlebar_window)
		M_Print (184, video_cursor_table[i], "off (borderless)");
	else
		M_DrawCheckbox (184, video_cursor_table[i], (int)vid_fullscreen.integer);
/*
	if (vid_bpp.integer == vid.DesktopBPP)
		M_Print (184, video_cursor_table[i], va("%s", vid_fullscreen.integer ? "on" : "off"));
		DrawCheckbox (184, video_cursor_table[i], (int)vid_fullscreen.integer);
	else
		M_Print (184, video_cursor_table[i], va("%s [locked]", (int)vid_fullscreen.integer ? "on" : "off"));
*/
	i++;

	M_Print (16, video_cursor_table[i], "       Test changes");
	if (vid_isLocked)
		M_Print (184, video_cursor_table[i], "[cmdline locked]");
	else if (window_buster)
		M_Print (184, video_cursor_table[i], "[mode > desktop]");
	i++;

	M_Print (16, video_cursor_table[i], "      Apply changes");
	if (vid_isLocked)
		M_Print (184, video_cursor_table[i], "[cmdline locked]");
	else if (window_buster)
		M_Print (184, video_cursor_table[i], "[mode > desktop]");
	i++;

	M_Print (16, video_cursor_table[i], "      Console width");
	switch ((int)scr_con_scale.integer)
	{
	case -1:
		M_Print (184, video_cursor_table[i], "auto");
		break;
	case 0:
		M_Print (184, video_cursor_table[i], "100%");
		break;
	case 1:
		M_Print (184, video_cursor_table[i], "50%");
		break;
	case 2:
		M_Print (184, video_cursor_table[i], "640 width");
		break;
	default:
		M_Print (184, video_cursor_table[i], "320 width");
		break;
	}
	i++;

	M_Print (16, video_cursor_table[i], "      Vertical sync");
	{
		qbool	wants_vsync = (vid_vsync.integer != 0);
		qbool	isavailable_vsync = false;
		char	description[30];
		extern qbool vid_gl_vsync_ext_exists;

#if RENDERER_DIRECT3D_AVAILABLE
		extern int d3d_vsync_state;

		if (engine.Renderer->graphics_api == RENDERER_DIRECT3D)
		{
				 if (wants_vsync == false && d3d_vsync_state==false) snprintf (description, sizeof(description), "%s", "off");
			else if (wants_vsync == true  && d3d_vsync_state==true ) snprintf (description, sizeof(description), "%s", "on ");
			else if (wants_vsync == true  && video_modes[vid_default].displaymode== MODE_WINDOWED)
			{	d3d_explain = 1;
				snprintf (description, sizeof(description), "Wants: %s %s", wants_vsync ? "on":"off", "(*)");
			}
			else if (video_modes[vid_default].displaymode== MODE_FULLSCREEN)
			{
				d3d_explain = 2;
				snprintf (description, sizeof(description), "Wants: %s %s", wants_vsync ? "on":"off", "(*)");
			}
			else
				Sys_Error ("Trouble in Denmark\n");
		}
		else
#endif
		{
				 if (wants_vsync == 0)
					 snprintf (description, sizeof(description), "%s", "off");
			else if (wants_vsync == 1 && vid_gl_vsync_ext_exists)
					 snprintf (description, sizeof(description), "%s", "on");
			else if (wants_vsync == 1 && !vid_gl_vsync_ext_exists)
				snprintf (description, sizeof(description), "Wants: %s %s", wants_vsync ? "on":"off", "[not avail]");
			else
				Sys_Error ("Trouble in Denmark\n");
		}
		M_Print (184, video_cursor_table[i], description);

	}
	i++;

	M_Print (16, video_cursor_table[i], "     Max frames/sec");
	M_Print (184, video_cursor_table[i], va("%1.0f",host_maxfps.floater));

	i++;

	M_Print (16, video_cursor_table[i], "     Show framerate");
	M_Print (184, video_cursor_table[i], scr_show_fps.integer ? "on" : "off");

#if RENDERER_DIRECT3D_AVAILABLE
	if (engine.Renderer->graphics_api == RENDERER_DIRECT3D && d3d_explain == 1 && video_options_cursor == 7)
	{
		M_PrintWhite (7* 8, video_cursor_table[i] + 2 * 8, "Vsync for Direct3D 8.1 Wrapper");
		M_PrintWhite (7* 8, video_cursor_table[i] + 3 * 8, "only available fullscreen");
	}
	else if (engine.Renderer->graphics_api == RENDERER_DIRECT3D &&  d3d_explain == 2 && video_options_cursor == 7)
	{
		M_PrintWhite (7* 8, video_cursor_table[i] + 2 * 8, "Vsync change will take effect");
		M_PrintWhite (7* 8, video_cursor_table[i] + 3 * 8, "on next fullscreen mode change");
	}
	else if (engine.Renderer->graphics_api == RENDERER_DIRECT3D &&  video_options_cursor == 2)
	{
		M_PrintWhite (7* 8, video_cursor_table[i] + 2 * 8, "Default desktop refresh rate");
		M_PrintWhite (7* 8, video_cursor_table[i] + 3 * 8, "used by Direct3D 8.1 wrapper");
	}
	else
#endif
	if (video_options_cursor == 8)
	{
		M_PrintWhite (7* 8, video_cursor_table[i] + 2 * 8, "For local games fps affects");
		M_PrintWhite (7* 8, video_cursor_table[i] + 3 * 8, "physics, 72 = Quake standard");
		M_PrintWhite (7* 8, video_cursor_table[i] + 4 * 8, "(local = single player, ..)");
	}
	else if (!vid_fullscreen.integer)
		M_PrintWhite (16, video_cursor_table[i] + 2 * 8, va("       Desktop size   %ix%i", vid.DesktopWidth, vid.DesktopHeight));

	i++;



	// cursor
	M_DrawCharacter (176 /*68*/, video_cursor_table[video_options_cursor], 12+((int)(realtime*4)&1));

	// notes          "345678901234567890123456789012345678"
//	M_Print (16, 172, "Windowed modes always use the desk- ");
//	M_Print (16, 180, "top color depth, and can never be   ");
//	M_Print (16, 188, "larger than the desktop resolution. ");
}

/*
================
VID_Menu_f
================
*/
void VID_Menu_f (void)
{
	key_dest = key_menu;
	m_state = m_videomodes;
	m_entersound = true;

	//set all the cvars to match the current mode when entering the menu
	VID_SyncCvarsToMode (vid_default);

	//set up bpp and rate lists based on current cvars
	VID_Menu_RebuildBppList ();
	VID_Menu_RebuildRateList ();

	//aspect ratio
	VID_Menu_CalcAspectRatio ();
}
