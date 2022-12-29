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
// vid_modes.c -- This file is about the current video mode and setting it.  Not about the other modes.

#include "quakedef.h"
#include "winquake.h"
#include <windows.h>
#include "winquake_video_modes.h"

int		vid_modenum = NO_MODE;			// This is what we HAVE				MODE WE GOT
int		vid_default = MODE_WINDOWED;	// This is what is *requested*		MODE WE WANT


/*
================
CenterWindow
================
*/
static void CenterWindow (HWND hWndCenter, int width, int height)
{
	int     CenterX = max(0, (vid.DesktopWidth  - width ) / 2);
	int		CenterY = max(0, (vid.DesktopHeight - height) / 2);

	SetWindowPos (hWndCenter, NULL, CenterX, CenterY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);
}

static 		DEVMODE	gdevmode;

void VIDMODES_ChangeDisplaySettings (int DisplayWidth, int DisplayHeight, int DisplayBitsPerPixel, int DisplayFrequency)
{
		// We need to change the display settings
//		DEVMODE	gdevmode;

		gdevmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
		gdevmode.dmBitsPerPel =			DisplayBitsPerPixel;
		gdevmode.dmPelsWidth =			DisplayWidth;
		gdevmode.dmPelsHeight =			DisplayHeight;
		gdevmode.dmDisplayFrequency =	DisplayFrequency;
		gdevmode.dmSize = sizeof (gdevmode);

		if (eChangeDisplaySettings(&gdevmode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			Sys_Error ("Couldn't set fullscreen DIB mode");
}

void VIDMODES_SetLastMode (void)
{
	if (eChangeDisplaySettings(&gdevmode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		Sys_Error ("Couldn't set fullscreen DIB mode");
}

void VIDMODES_AltTabFix (void)
{
	MoveWindow (mainwindow, 0, 0, gdevmode.dmPelsWidth, gdevmode.dmPelsHeight, false);
}


int gWindowStandardStyle = WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |  WS_SIZEBOX;
// Baker: This is NOT a constant
int gVileHack_IsFullScreenWindow = false;

// Baker: This IS ONLY called first time?  Confirmed.  This occurs ONCE.  During video init.  And never again.
qbool VID_SetMyVideoMode (int modenum, HINSTANCE MyHINSTANCE)
{
	int		lastmodestate;
	int		expanded_width;				// If I want a 640x480 window, I need extra space for borders + title bar
	int		expanded_height;			// Border and such
	DWORD	StandardWindowStyle			= gWindowStandardStyle;
	DWORD	ExtendedWindowStyle = 0;	// Scrollbar, accept dragdrop files, etc.
	qbool	IsFullScreen = modenum;		// 0 is windowed, everything fullscreen else is non-zero
	qbool	IsFullScreenWindowedMode =	IsFullScreen? 0: (video_modes[modenum].width == GetSystemMetrics(SM_CXSCREEN) && video_modes[modenum].height == GetSystemMetrics(SM_CYSCREEN));
	static qbool first_time = true;

	gVileHack_IsFullScreenWindow = IsFullScreenWindowedMode;

	//	DMSG ("Ok!")

	if (IsFullScreen /*&& !leavecurrentmode*/)
		VIDMODES_ChangeDisplaySettings(video_modes[modenum].width, video_modes[modenum].height, video_modes[modenum].bpp, video_modes[modenum].refreshrate);

	lastmodestate = modestate;

	if (IsFullScreen)	// Baker: apparently this is very important placement.
		modestate = MODE_FULLSCREEN;

	// Direct3D HACK - upgrade to fullscreen windowed mode if close
	if (engine.Renderer->graphics_api == RENDERER_DIRECT3D && !IsFullScreen && !vid_isLocked)
	{
		// Baker: for Direct3D with windowed mode ... we have issues if the window is nearly fullscreen
		// If someone is using commandline or resizing the window, let them personally deal with it
		// But don't let them have this issue via setting a video mode in the menu, etc.
		qbool changed_it = false;

		if (video_modes[0].width  > (vid.DesktopWidth - 64 /* no reason*/) || (video_modes[0].height > (vid.DesktopHeight - 64 /* no reason*/)) )
		{
			video_modes[0].width  = vid.DesktopWidth;
			video_modes[0].height = vid.DesktopHeight;

			changed_it = true;

			Con_Printf ("VID_SetMyVideoMode:  Resized requested window mode size to protect D3D wrapper from extending beyond the window\n");
			IsFullScreenWindowedMode =	(video_modes[modenum].width == vid.DesktopWidth && video_modes[modenum].height == vid.DesktopHeight);

			if (!IsFullScreenWindowedMode)
				Sys_Error ("Set fullscreen but not acknowledge");

			gVileHack_IsFullScreenWindow = IsFullScreenWindowedMode;
		}
	}

	ClientDrawingRect.top = ClientDrawingRect.left = 0; // local and set

	ClientDrawingRect.right = video_modes[modenum].width;  // local and set
	ClientDrawingRect.bottom = video_modes[modenum].height;  // local and set

	if (IsFullScreen || IsFullScreenWindowedMode)
		StandardWindowStyle = WS_POPUP; // No borders and such

	{ // Reuse mainwindow ?  	else // just update characteristics like ... size  SetWindowPos (mainwindow, NULL, 0, 0, width, height, 0);

		HWND	temp_dibwindow;
		RECT	tempRect  = ClientDrawingRect; // local and set
		AdjustWindowRectEx (&tempRect, StandardWindowStyle, FALSE, 0);

		expanded_width = tempRect.right - tempRect.left;	// An expansion
		expanded_height = tempRect.bottom - tempRect.top;

		// Create the DIB window
		temp_dibwindow = CreateWindowEx (
			 ExtendedWindowStyle,
			 TEXT(ENGINE_NAME),
			 va("%s %s %s",ENGINE_NAME, RENDERER_NAME, ENGINE_VERSION),
			 StandardWindowStyle,
			 tempRect.left, tempRect.top,
			 expanded_width,
			 expanded_height,
			 NULL,
			 NULL,
			 MyHINSTANCE,
			 NULL);

		if (!temp_dibwindow)
			Sys_Error ("Couldn't create DIB window");

		mainwindow = temp_dibwindow;

	}

	if (!IsFullScreen) // Center and show the DIB window
		CenterWindow (mainwindow, ClientDrawingRect.right - ClientDrawingRect.left, ClientDrawingRect.bottom - ClientDrawingRect.top); //, false); // local and set
	else
		window_x = window_y = 0;	// Assigned

	// Show the Window
	ShowWindow (mainwindow, SW_SHOWDEFAULT);
	UpdateWindow (mainwindow);

	if (!IsFullScreen) // Baker: apparently this is very important placement.
		modestate = MODE_WINDOWED;

	// Because we have set the background brush for the window to NULL (to avoid flickering when re-sizing the window on the desktop),
	// we clear the window to black when created, otherwise it will be  empty while Quake starts up.

	// Baker: This works out very shitty in Direct3D if fullscreen
#if 0
	if (!(engine.Renderer->graphics_api == RENDERER_DIRECT3D && IsFullScreen))
	{
		HDC	thdc;
		thdc = GetDC (mainwindow);
		PatBlt (thdc, 0, 0, ClientDrawingRect.right, ClientDrawingRect.bottom, IsFullScreen? BLACKNESS : WHITENESS); // local and set
		ReleaseDC (mainwindow, thdc); thdc = 0;
	}
#endif

// ALL the code from this point formward should probably be moved out

	//johnfitz -- stuff
	// Baker: Consize takes care of this
//	vid.width = video_modes[modenum].width;
//	vid.height = video_modes[modenum].height;

	VID_Consize_f (); // Should occur immediately after vid.width variable is changed
	Con_DevPrintf (DEV_OPENGL, "OpenGL SetMyVideoMode ... vid_default is %i and video_mode[x] where x = %i", vid_default, modenum);
//	if (vid_default != modenum)
//		Sys_Error ("Mode mismatch for VID_Consize");

	//johnfitz

// Baker: let's see what happens
//	VID_Consize_f();		// Yay!  We are calling this more than once

//	vid.numpages = 2;

//	mainwindow = dibwindow;		// Already done

	SendMessage (mainwindow, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
	SendMessage (mainwindow, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIcon);

	first_time = false;
	return true;
}


void VID_UpdateDesktopProperties (void)
{
	DEVMODE			devmode;
	if (EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &devmode))
	{
		vid.DesktopWidth =		devmode.dmPelsWidth;
		vid.DesktopHeight =		devmode.dmPelsHeight;
		vid.DesktopBPP =		devmode.dmBitsPerPel;
		vid.DesktopDispFreq =	devmode.dmDisplayFrequency;
	}
}

void VID_DescribeDesktop_f (void)
{
	Con_Printf ("Desktop: %i x % i %i bpp @ freq %i Hz\n", vid.DesktopWidth, vid.DesktopHeight, vid.DesktopBPP, vid.DesktopDispFreq);
}


LONG WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void OS_WINDOWS_RegisterClassFrame (HINSTANCE hInstance)
{
	WNDCLASS		window_class;

	/* Register the frame class */
	window_class.style         = 0;
	window_class.lpfnWndProc   = (WNDPROC)MainWndProc;
	window_class.cbClsExtra    = 0;
	window_class.cbWndExtra    = 0;
	window_class.hInstance     = hInstance;
	window_class.hIcon         = 0;
	window_class.hCursor       = LoadCursor (NULL, IDC_ARROW);
	window_class.hbrBackground = NULL;
	window_class.lpszMenuName  = 0;
    window_class.lpszClassName = ENGINE_NAME; //"WinQuake";

	if (!RegisterClass(&window_class))
		Sys_Error ("Couldn't register window class");
}


/*
================
VID_SyncCvars -- johnfitz -- set vid cvars to match current video mode
================
*/
//extern int vid_default;
//void VID_SyncCvars (void)
//{
//	Cvar_SetStringByRef (&vid_width,				va("%i", video_modes[vid_default].width));
//	Cvar_SetStringByRef (&vid_height,				va("%i", video_modes[vid_default].height));
//	Cvar_SetStringByRef (&vid_bpp,					va("%i", video_modes[vid_default].bpp));
//	Cvar_SetStringByRef (&vid_displayfrequency,		va("%i", video_modes[vid_default].refreshrate));
//	Cvar_SetStringByRef (&vid_fullscreen,           va("%i", vid_default!=0));
//	Cvar_SetStringByRef (&vid_fullscreen, (vid_fullscreen_only) ? "1" : ((windowed) ? "0" : "1"));
//}

//extern int vid_default;
void VID_SyncCvarsToMode (int myMode)
{
	//Sanity check
	if (myMode < 0 || myMode >= num_video_modes)
	{
		Con_Printf ("VID_SyncCvarsToMode invalid mode\n");
		return;
	}

	Cvar_SetStringByRef (&vid_width,				va("%i", video_modes[myMode].width));
	Cvar_SetStringByRef (&vid_height,				va("%i", video_modes[myMode].height));
	Cvar_SetStringByRef (&vid_bpp,					va("%i", video_modes[myMode].bpp));
	Cvar_SetStringByRef (&vid_displayfrequency,		va("%i", video_modes[myMode].refreshrate));
	Cvar_SetStringByRef (&vid_fullscreen,           va("%i", myMode!=0));
//	Cvar_SetStringByRef (&vid_fullscreen, (vid_fullscreen_only) ? "1" : ((windowed) ? "0" : "1"));
}
