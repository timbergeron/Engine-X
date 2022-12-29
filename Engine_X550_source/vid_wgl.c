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
// vid_wgl.c -- NT GL vid component

#include "quakedef.h"
#include "winquake.h"
#include "resource.h"
//#include <commctrl.h>
#include "winquake_video_modes.h"

// === end includes



//#define VID_ROW_SIZE		3
//#define WARP_WIDTH		320
//#define WARP_HEIGHT		200
//#define BASEWIDTH		320
//#define BASEHEIGHT		200


// === end defines



//qbool	DDActive;



static qbool	vid_initialized = false;
//static qbool	windowed;

//qbool leavecurrentmode;
qbool vid_canalttab = false;
static int		windowed_mouse;
//extern qbool	in_mouse_acquired;	// from in_win.c
HICON	hIcon;

// vid_modes.c
// vide_modes.c
qbool VID_SetMyVideoMode (int modenum, HINSTANCE MyHINSTANCE);
void OS_WINDOWS_RegisterClassFrame (HINSTANCE hInstance);


//DWORD		WindowStyle, ExWindowStyle;

HWND		mainwindow; //, dibwindow;




#if SUPPORTS_GLVIDEO_MODESWITCH
qbool	user_video_mode_forced = false; // Baker 3.93: D3D version loses surface with this, investigate later
#else
qbool	user_video_mode_forced = true;
#endif

//int			desktop_bpp;  // query this @ startup and when entering the video menu
qbool	vid_fullscreen_only = false; // Baker 3.99h: This is to allow partial video mode switching if -bpp != desktop_bpp, only available if -window isn't used

//static int	windowed_default;
//unsigned char	vid_curpal[256*3];
qbool	fullsbardraw = false;

// Baker: begin hwgamma support


HDC		maindc;



HWND WINAPI InitializeWindow (HINSTANCE hInstance, int nCmdShow);



void VID_Menu_Init (void); //johnfitz
void VID_Menu_f (void); //johnfitz
void VID_MenuDraw (void);
void VID_MenuKey (int key);

LONG WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AppActivate (BOOL fActive, BOOL minimize);
char *VID_GetModeDescription (int mode);
void Key_ClearAllStates (void);
void VID_UpdateWindowStatus (void);




static qbool update_vsync = false;




SWAPINTERVALFUNCPTR wglSwapIntervalEXT = NULL;
















qbool OnChange_vid_vsync (cvar_t *var, const char *string)
{
	update_vsync = true;
	return false;
}

/*
================
VID_UpdateWindowStatus
================
*/
int		MouseRegionCenter_x, MouseRegionCenter_y;
int		window_x, window_y, window_width, window_height;

// REGIONS:  MouseWindowLockRegion .... this is windows coordinate system ................  241, 93 to (241 + clientwidth), (93 + clientheight)
//           ClientDrawingRect ........ within the frame of reference of the client area..  0,0 to clientwidth, clientheight

RECT	MouseWindowLockRegion;
RECT	ClientDrawingRect;


void VID_UpdateWindowStatus (void)
{

	MouseWindowLockRegion.left = window_x;
	MouseWindowLockRegion.top = window_y;
	MouseWindowLockRegion.right = window_x + window_width;
	MouseWindowLockRegion.bottom = window_y + window_height;
	MouseRegionCenter_x = (MouseWindowLockRegion.left + MouseWindowLockRegion.right) / 2;
	MouseRegionCenter_y = (MouseWindowLockRegion.top + MouseWindowLockRegion.bottom) / 2;

	IN_Mouse_UpdateClipCursor ();
}




extern int vid_modenum;
extern int vid_default;


static void VID_PaintTiles (void)
{   HDC	thdc = GetDC (mainwindow);
	int canvas_width = ClientDrawingRect.right, canvas_height = ClientDrawingRect.bottom;
	int k;

	for (k = 8; k >= 1; k --)
	if (canvas_width / (8 * k) == (float)(canvas_width / (8.0f * (float)k)))	// Is whole number?
		break;


	{	int i, j, checker_x_size = k*8, checker_y_size = k * 8 /*48*/;
		int checker_count_cols = canvas_width  / checker_x_size;
		int checker_count_rows = canvas_height / checker_y_size;

		for (i = 0; i<checker_count_cols; i++)
			for (j = 0; j<checker_count_rows+1; j++)
			{
				int x1 = i * checker_x_size;  // 128 = left start, 32 = tile size
				int y1 = j * checker_y_size; // 128 = top start, 32 = tile size
				int x2 = (i + 1) * checker_x_size;
				int y2 = (j + 1) * checker_y_size;
				int	c = ((int)(i+j) & 1) ? BLACKNESS : WHITENESS;;

				PatBlt (thdc, (int) x1, (int) y1, (int) x2, (int) y2 , c); //  modenum == 0? BLACKNESS : WHITENESS); // local and set
			}
	}
	ReleaseDC (mainwindow, thdc); thdc = 0;
}

/*
================
VID_SetMode
================
*/
extern void IN_StartupMouse (void);
// Baker: This codebase's OpenGL paths might be "trying a little too hard" by doing all of this
//        on a reset.  Not the code in here, the context stuff.  A display change is just a
//        a display change right?
//
//        Direct3D doesn't do all of this on a reset.

qbool in_setmode = false;

#if RENDERER_DIRECT3D_AVAILABLE
int vid_d3d_minwindow_x;
int vid_d3d_minwindow_y;

int	d3d_vsync_state;
#endif

#if RENDERER_DIRECT3D_AVAILABLE && 0
qbool vid_d3d_dirty = false;
double vid_d3d_dirty_time;

qbool d3d_zorder_foobared = false;
#endif

// ALL CHANGES EXCEPT WINDOW RESIZING FLOW THROUGH HERE
qbool full_screen_to_windowed;
double full_screen_to_windowed_time;
int VID_SetMode (const int modenum, qbool isReset)
{
	int		original_mode; // Baker: to fall back on, even though we don't actually do that
	int		temp;
	extern	qbool in_mouse_acquired;
	qbool	reset_only = false;		// Baker: Direct3D doesn't do everything on a reset
	qbool	mouse_was_captured = in_mouse_acquired; // First we need to know if we need to recapture the mouse
	int		newmode_is_windowed = -1;

	if (!in_setmode)
		Con_DevPrintf (DEV_VIDEO, "In_setmode on (%i)\n", (in_setmode = true)); // Prevent recursive resizing troubles
	

// Sanity checks
	if (modenum < 0 || modenum >= num_video_modes)
		Sys_Error ("Bad video mode");

	if (isReset && vid_modenum > 0 /*fullscreen*/ && modenum == 0 /* we are going windowed */)
	{
		full_screen_to_windowed = true;
		full_screen_to_windowed_time = Sys_DoubleTime ();	// timestamp so not honoring this in silly situations
	}
	else
		full_screen_to_windowed = false;

	if (vid_modenum == 0 && engine.Renderer->graphics_api == RENDERER_DIRECT3D)
		VID_UpdateDesktopProperties (); // Any switch when already in windowed mode is a good time to snag desktop info when using D3D wrapper (because fullscreen isn't 100% reliable)

#if 0 // def _DEBUG
	Con_Printf ("Mode %i requested has width and height of %i x %i\n", modenum, video_modes[modenum].width, video_modes[modenum].height);
	if (modenum == 0)
		in_setmode = in_setmode;
#endif




#if RENDERER_DIRECT3D_AVAILABLE
	if (engine.Renderer->graphics_api == RENDERER_DIRECT3D)
	{
		vid_d3d_minwindow_x = vid_d3d_minwindow_y = 0; // Once we have renderer switch, we will want
#if RENDERER_DIRECT3D_AVAILABLE && 0
		vid_d3d_dirty = true;
		vid_d3d_dirty_time = 0;
		Con_DevPrintf (DEV_VIDEO, "D3D Dirty state and size locks reset\n");
#endif
	}
#endif
	// Baker: this is evil because we are assuing Quake's scheme
	// of windowed mode = 0.
#if RENDERER_DIRECT3D_AVAILABLE		// Baker: I need this even if I don't do D3D_ResetMode to define sizing locks for D3D
	if (modenum > 0)
		newmode_is_windowed = 0;
	else if (video_modes[modenum].width == vid.DesktopWidth && video_modes[modenum].height == vid.DesktopHeight)
		newmode_is_windowed = 2; // Fullscreen windowed mode
	else
		newmode_is_windowed = 1; // Normal windowed mode
#endif


// Clear the input and keyboard states
	Key_ClearAllStates ();

// so Con_Printfs don't mess us up by forcing vid and snd updates
	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;

	// Stop Sounds
	S_BlockSound ();
#pragma message ("Quality assurance: 2 things.  First the sound isn't stopping on ALT-ENTER.  Second let's stop sound in reconnect")

	S_ClearBuffer ();
	MP3Audio_Pause ();

// Save video mode
	if (vid_modenum == NO_MODE)
		original_mode = 0 /* windowed default mode */;
	else
		original_mode = vid_modenum;

	// Release mouse .. we restart it later

	IN_Mouse_Unacquire ();

	// Set the new video mode, errors happen in that function

#if RENDERER_DIRECT3D_AVAILABLE && RENDERER_OPENGL_AVAILABLE
	if (engine.Renderer->graphics_api == RENDERER_OPENGL)
		reset_only = false;  // OpenGL pathway: Do whole 9 yards
	else
		reset_only = isReset;
#endif

#if RENDERER_DIRECT3D_AVAILABLE && !RENDERER_OPENGL_AVAILABLE
	reset_only = isReset;
#endif

#if !RENDERER_DIRECT3D_AVAILABLE && RENDERER_OPENGL_AVAILABLE
	reset_only = false;
#endif




	if (reset_only == false)
	{

		// Baker: there are differences between this and D3D_ResetMode!!!
		VID_SetMyVideoMode(modenum, global_hInstance); //, false /* isFullScreen */);
	}
	else
	{
#if RENDERER_DIRECT3D_AVAILABLE
		// Baker: wrap the reset mode with all the stuff we need

		// Begin D3D_ResetMode missing stuff
		//		lastmodestate = modestate;

		if (modenum !=0 )	// Baker: apparently this is very important placement.
			modestate = MODE_FULLSCREEN;


		D3D_ResetMode (video_modes[modenum].width, video_modes[modenum].height, video_modes[modenum].bpp, &window_x, &window_y, newmode_is_windowed);

		if (modenum == 0) // Baker: apparently this is very important placement.
			modestate = MODE_WINDOWED;

//		vid.numpages = 2;

//		mainwindow = dibwindow;		// Baker: we are never changing this!

		SendMessage (mainwindow, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
		SendMessage (mainwindow, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIcon);

		// now fill in all the ugly globals that Quake stores the same data in over and over again
		// (this will be different for different engines)

		/*vid.width  =*/ window_width  = ClientDrawingRect.right  = video_modes[modenum].width;
		/*vid.height =*/ window_height = ClientDrawingRect.bottom = video_modes[modenum].height;

		VID_Consize_f (); // Should occur immediately after vid.width variable is changed
		Con_DevPrintf (DEV_OPENGL, "D3D alternate to SetMyVideoMode ... vid_default is %i and video_mode[x] where x = %i", vid_default, modenum);

		if (vid_default != modenum)		// Baker: Why did I do this and when can this actually happen?
			Sys_Error ("Mode mismatch for VID_Consize");
#endif
	}

	{ // mouseclip globals
		window_width  = video_modes[modenum].width;		// Assigned
		window_height = video_modes[modenum].height;	// Assigned

#if RENDERER_DIRECT3D_AVAILABLE
// Baker: prevent oddities, so restrict resizing a bit for Direct3D wrapper
		if (engine.Renderer->graphics_api == RENDERER_DIRECT3D && newmode_is_windowed == 1)
		{

			vid_d3d_minwindow_x = video_modes[modenum].width;
			vid_d3d_minwindow_y = video_modes[modenum].height;
			Con_DevPrintf (DEV_VIDEO, "D3D Window resize restrictions updated\n");

		}
#endif
	}

// Shouldn't setmyvidemode do this
// But what is the point of vid_default
// Sheesh this video code is spaghetti
	vid_modenum = modenum;

	// Restore sound
	S_UnblockSound ();
	MP3Audio_Resume ();

	// Restore screen updates to prior state
	scr_disabled_for_loading = temp;

	if (reset_only == false)
	{
		// now we try to make sure we get the focus on the mode switch, because sometimes in some systems we don't.
		// We grab the foreground, then finish setting up, pump all our messages, and sleep for a little while
		// to let messages finish bouncing around the system, then we put ourselves at the top of the z order,
		// then grab the foreground again. Who knows if it helps, but it probably doesn't hurt

		SetForegroundWindow (mainwindow);

		// Baker: apparently vid_default cannot always be trusted
		// so we are copying this out

		{
			MSG		msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}
		}

//		{   int i, increments = 8;
//			HDC	thdc;
//			thdc = GetDC (mainwindow);
//			for (i= 0; i<increments; i++)
//			{
//				PatBlt (thdc, (int)((float)(i)  /(float)(increments)*(float)ClientDrawingRect.right), 0,
//							  (int)((float)(i+1)/(float)(increments)*(float)ClientDrawingRect.right), ClientDrawingRect.bottom,
//							  modenum == 0? BLACKNESS : WHITENESS); // local and set
//				Sleep (100/ increments);
//			}
//			ReleaseDC (mainwindow, thdc); thdc = 0;
//		}


		Sleep (100);

		// First point that we can draw "fullscreen" ... meaning the taskbar isn't pissing us off

		if (isReset == false && !(engine.Renderer->graphics_api == RENDERER_DIRECT3D && vid_modenum > 0)) // Bad visuals for D3D and fullscreen) // Bad visuals for D3D and fullscreen because D3D clears to black on setmode ... so we don't get nice transitition
			VID_PaintTiles ();



		// Z-order window to top (HWND_TOP), but do not move or resize SWP_NOMOVE | SWP_NOSIZE
		SetWindowPos (mainwindow, HWND_TOP, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOCOPYBITS);

		SetForegroundWindow (mainwindow);

		Con_SafePrintf ("Video mode %s initialized (%s).\n", VID_GetModeDescription(vid_modenum), vid_modenum == 0 ? "windowed" : "fullscreen");
	}


	vid.recalc_refdef = 1;

	//R00k mouse died on mode change
	IN_Mouse_Restart ();

	if (mouse_was_captured)
		IN_Mouse_Acquire ();
	else
		IN_Mouse_Unacquire ();	// Because IN_Mouse_Restart automatically acquires it

	// Update the mouse clip region
	VID_UpdateWindowStatus ();	// Baker: if we make a resize function we can kill this, besides shouldn't we be in sync
								// Then we update it prior to re-acquiring the mouse!  So mouse_acquire will take care of this

	in_setmode = false; Con_DevPrintf (DEV_VIDEO, "In_setmode off\n");
	return true;
}



// This is taking window width, not client width.  Remember there are borders.  We have to calculate client area.
int VID_ResizeWindow (void)
{
//	extern	qbool in_mouse_acquired;
//	qbool	mouse_was_captured = in_mouse_acquired; // First we need to know if we need to recapture the mouse
	extern  qbool in_video_restart;
	extern  double in_video_restart_time;

	RECT clientrect;
	int newWidth, newHeight;

	Con_DevPrintf (DEV_VIDEO, "VID_ResizeWindow: Tried to resize window\n");
	
	if (in_setmode /*|| in_video_restart*/)
		return false; // Prevent this from being called within mode changes

//	if (in_video_restart == false && (Sys_DoubleTime() < (in_video_restart_time + 2)))
//	{
//		// No ...2 second delay
//		return false;
//	}

	if (vid_default != 0)
		return false;		// Not a windowed mode ... ignore

	Con_DevPrintf (DEV_VIDEO, "VID_ResizeWindow: Ok ... we ARE resizing it\n");


#if RENDERER_DIRECT3D_AVAILABLE && 0
	if (engine.Renderer->graphics_api == RENDERER_DIRECT3D)
	{
		vid_d3d_dirty = true;
		vid_d3d_dirty_time = realtime;
		Con_DevPrintf (DEV_VIDEO, "D3D dirty\n");
	}
#endif




	GetClientRect (mainwindow, &clientrect);

	newWidth  =  clientrect.right - clientrect.left;
	newHeight =  clientrect.bottom - clientrect.top;

	if ( newWidth < 1 || newHeight < 1) return false;  // check the client rect dimensions to make sure it's valid
	if ( newWidth > vid.DesktopWidth || newHeight > vid.DesktopHeight) return false;	// Keep it real.  Reject stupid values.
	if ( newWidth == video_modes[0].width &&  newHeight == video_modes[0].height) return false;  // Reject if no change

#if RENDERER_DIRECT3D_AVAILABLE
	if (engine.Renderer->graphics_api == RENDERER_DIRECT3D)
	{
		extern void D3D_ResetWindow (RECT new_clientrec, const int newWidth, const int newHeight, const int WhateverBpp);
		//D3D_ResizeWindow ();
		D3D_ResetWindow (clientrect, newWidth, newHeight, video_modes[0].bpp);
	}
#endif


	// Yes, there might be someone with high dexterity
	// that can manipulate the mouse and keyboard
	// simultaneously.  Just to be safe ...
	// Clear the input and keyboard states
	//	Key_ClearAllStates ();

	// These need to set to "update the window state" (mouse cursor region)
	// window_x and window_y should be unaffected
	// window_width and window_height changed

	video_modes[0].width  = clientrect.right - clientrect.left;
    video_modes[0].height = clientrect.bottom - clientrect.top;

	vid.width  = ClientDrawingRect.right  = video_modes[0].width;
	vid.height = ClientDrawingRect.bottom = video_modes[0].height;

	Con_DevPrintf (DEV_OPENGL, "vid_default is %i and video_mode[x] where x = 0", vid_default);
	if (vid_default != 0)
		Sys_Error ("Mode mismatch for VID_Consize");
	VID_Consize_f (); // Should occur immediately after vid.width variable is changed

	{ // mouseclip globals
		window_width  = video_modes[0].width;		// Assigned
		window_height = video_modes[0].height;	// Assigned
	}

//	//R00k mouse died on mode change
//	IN_Mouse_Restart ();
//
//	if (mouse_was_captured)
//		IN_Mouse_Acquire ();
//	else
//		IN_Mouse_Unacquire ();	// Because IN_Mouse_Restart automatically acquires it

	// Update mouse clip region
	VID_UpdateWindowStatus ();

	{ // Render a frame to update the video immediately
		vid.recalc_refdef = true;	// Recalc
		SCR_UpdateScreen ();	// Refresh it
	}

	return true;
}




/*
===============
VID_Vsync_f -- johnfitz
===============
*/
void VID_Vsync_f (void)
{
	update_vsync = true;
	//return false;
}

/*
===================
VID_Restart -- johnfitz -- change video modes on the fly
===================
*/




BOOL bSetupPixelFormat(HDC hDC);

void GL_SetupState(void);


#if RENDERER_DIRECT3D_AVAILABLE && 0
qbool vid_restart_no_check_same_mode = false;
#endif

// This will need reworked a tad.

qbool in_video_restart;
double in_video_restart_time = 0;
void VID_Restart_f (void)
{
#if RENDERER_OPENGL_AVAILABLE
	HDC			hdc;
	HGLRC		hrc;
#endif
	int			i;
	qbool	mode_changed = false;
//	vmode_t		oldmode;
	int			oldmode;

	in_video_restart = true;

	if (vid_isLocked)
	{
		Con_Printf("VID_Restart_f: Video is locked\n");

//				in_video_restart = false;
//				return;
				goto fail_vidrestart;
	}

	if (vid_fullscreen_only && !vid_fullscreen.integer)
	{
		Con_Printf("VID_Restart_f: Only fullscreen allowed and windowed mode requested\n");
//				in_video_restart = false;
//				return;
				goto fail_vidrestart;
	}

// Baker: determine if the mode is any different than what we are running
// check cvars against current mode
//
	{
		if (vid_fullscreen.integer)
		{
			if (video_modes[vid_default].displaymode == MODE_WINDOWED /*windowed */)
				mode_changed = true;
			else if (video_modes[vid_default].refreshrate != (int)vid_displayfrequency.integer)
				mode_changed = true;
		}
		else
		{
			Con_DevPrintf (DEV_VIDEO, "Requesting a windowed mode\n");
			if (video_modes[vid_default].displaymode != MODE_WINDOWED)
			{
				Con_DevPrintf (DEV_VIDEO, "Set mode changed to true\n");
				mode_changed = true;
			}

		}

		if (video_modes[vid_default].width != (int)vid_width.integer ||
			video_modes[vid_default].height != (int)vid_height.integer)
			mode_changed = true;
	}

#if RENDERER_DIRECT3D_AVAILABLE && 0
	if (!vid_restart_no_check_same_mode)
#endif
		if (!mode_changed)
		{
			Con_DevPrintf (DEV_VIDEO, "No video mode change\n");
			goto nochange;
		}

//
// decide which mode to set
//
	oldmode = vid_default;

	if (vid_fullscreen.integer)
	{
		// Baker: Locate matching mode
		for (i=1; i<num_video_modes; i++)
		{
			if (video_modes[i].width == (int)vid_width.integer &&
				video_modes[i].height == (int)vid_height.integer &&
				video_modes[i].bpp == (int)vid_bpp.integer &&
				video_modes[i].refreshrate == (int)vid_displayfrequency.integer)
			{
				break;
			}
		}

		// We didn't find one
		if (i == num_video_modes)
		{
			Con_Printf ("%dx%dx%d %dHz is not a valid fullscreen mode\n",
						(int)vid_width.integer,
						(int)vid_height.integer,
						(int)vid_bpp.integer,
						(int)vid_displayfrequency.integer);
//				in_video_restart = false;
//				return;
				goto fail_vidrestart;
		}

// Baker: This variable aint ever even read.  Sheesh.		windowed = false;
		vid_default = i;
	}
	else //not fullscreen
	{

		// Baker: remove this somehow.
		// Refresh desktop info and turn into bool function
		// Like ... can switch to windowed



		{
			//Window_Restrict_Size (320, 200, vid.DesktopWidth, vid.DesktopHeight);
			if (vid_width.integer > vid.DesktopWidth)
			{
				Con_Printf ("Window width can't be greater than desktop width\n");
//				in_video_restart = false;
//				return;
				goto fail_vidrestart;
			}

			if (vid_height.integer > vid.DesktopHeight)
			{
				Con_Printf ("Window width can't be less than desktop height\n");
//				in_video_restart = false;
//				return;
				goto fail_vidrestart;
			}


			if (vid_width.integer < 320)
			{
				Con_Printf ("Window width can't be less than 320\n");
//				in_video_restart = false;
//				return;
				goto fail_vidrestart;
			}

			if (vid_height.integer < 200)
			{
				Con_Printf ("Window height can't be less than 200\n");
//				in_video_restart = false;
//				return;
				goto fail_vidrestart;
			}
		}

		video_modes[0].width = (int)vid_width.integer;
		video_modes[0].height = (int)vid_height.integer;

		Rebuild_Mode_Description (0);

// Baker: This variable aint ever even read.  Sheesh.				windowed = true;
		vid_default = 0;
	}
//
// destroy current window
//
	Con_DevPrintf (DEV_VIDEO, "In_setmode on (%i)\n", (in_setmode = true)); // Prevent recursive resizing troubles

	// Baker: restore gamma after Window is destroyed
	// to avoid Windows desktop looking distorted due
	// during switch
	Gamma_RestoreHWGamma ();

#if RENDERER_OPENGL_AVAILABLE
	if (engine.Renderer->graphics_api == RENDERER_OPENGL)
	{   // OpenGL destroy/release stuffs
		hrc = ewglGetCurrentContext();
		hdc = ewglGetCurrentDC();
		ewglMakeCurrent(NULL, NULL);

		vid_canalttab = false;

		if (hdc && mainwindow)
		{
			ReleaseDC (mainwindow, hdc); hdc = 0;
		}

		if (modestate == MODE_FULLSCREEN)
		{
			eChangeDisplaySettings (NULL, 0);
			VID_UpdateDesktopProperties ();
		}
		if (maindc && mainwindow)
		{
			ReleaseDC (mainwindow, maindc); maindc = NULL;
		}

		if (mainwindow)
		{
			DestroyWindow (mainwindow); mainwindow = 0;
		}
	}
#endif

//
// set new mode
//
	VID_SetMode (vid_default, /* isReset = */ true); // This is a mode reset

#if RENDERER_OPENGL_AVAILABLE
	if (engine.Renderer->graphics_api == RENDERER_OPENGL)
	{	// Recreate stuffs
		maindc = GetDC(mainwindow);

		bSetupPixelFormat(maindc);

		// if bpp changes, recreate render context and reload textures
		if (video_modes[vid_default].bpp != video_modes[oldmode].bpp)
		{
			ewglDeleteContext (hrc);	hrc = 0;
			hrc = ewglCreateContext (maindc);
			if (!ewglMakeCurrent (maindc, hrc))
				Sys_Error ("VID_Restart: ewglMakeCurrent failed");

			//Bakertest: TexMgr_ReloadImages ();
			GL_SetupState ();
		}
		else if (!ewglMakeCurrent (maindc, hrc))

		{
			//Sys_Error ("VID_Restart: ewglMakeCurrent failed");

			char szBuf[80];
			LPVOID lpMsgBuf;
			DWORD dw = GetLastError();
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
			snprintf(szBuf, sizeof(szBuf), "VID_Restart: ewglMakeCurrent failed with error %d: %s", dw, lpMsgBuf);
 			Sys_Error (szBuf);
		}
	}
#endif


	vid_canalttab = true;

	// Baker: Now that we have created the new window, restore it
	Gamma_SetUserHWGamma (); // VID_SetDeviceGammaRamp (currentgammaramp);

	//swapcontrol settings were lost when previous window was destroyed
	VID_Vsync_f ();


//	VID_Consize_f ();

nochange:

//
// keep cvars in line with actual mode
//
	VID_SyncCvarsToMode (vid_default);
fail_vidrestart:
	in_video_restart_time = Sys_DoubleTime ();
	in_video_restart = false;
}


void VID_Fullscreen(void)
{
	// Only do if video mode switching is enabled?  Yes

	if (!host_initialized)
		return;

	if (vid_isLocked)
	{
		Con_Printf("VID_Fullscreen: Video mode switching is locked\n");
		return;
	}

	if (vid_default > 0  || vid_fullscreen_only)
	{
		Con_DevPrintf(DEV_VIDEO, "VID_Fullscreen: Already fullscreen\n");
		return;
	}

	// Baker: See if there is a matching width + height + bpp combination
	//        And if not, just select one.

//	VID_SyncCvars (vid_default);	// Stupid to sync twice
	{
//		void VID_SyncCvarsToMode (int myMode);
		int VID_FindFullScreenModeOnCvars (void);

		int t = VID_FindFullScreenModeOnCvars ();
		VID_SyncCvarsToMode (t);
	}


	Cvar_SetFloatByRef(&vid_fullscreen, 1);
	Con_DevPrintf(DEV_VIDEO, "vid_fullscreen set to 1; Requesting restart\n");

	VID_Restart_f();
}


void PreQualifyWindowedModeVersusDesktop (void)
{ // Pre-qualify new windowed mode
	if (vid_fullscreen.integer)
	{
		Con_Printf ("PreQualifyWindowedModeVersusDesktop: Received a fullscreen mode\n");
		return;
	}

	// Make both a multiple of 8
	if ((int)vid_width.integer & 7)
		Cvar_SetFloatByRef (&vid_width, (float)((int)vid_width.integer & 0xfff8));

	if ((int)vid_height.integer & 7)
		Cvar_SetFloatByRef (&vid_height, (float)((int)vid_height.integer & 0xfff8));


	if (vid_width.integer < 320 || vid_height.integer < 200)
	{
		Con_DevPrintf (DEV_VIDEO, "Note: Requested windowed mode too exceeds minimum size of 320 x 200... \n");
		Con_DevPrintf (DEV_VIDEO, "      Requested size: %i x %i\n", (int)vid_width.integer, (int)vid_height.integer);

		Cvar_SetFloatByRef (&vid_width,  640 /*(float)(vid.DesktopWidth  - 64)*/);
		Cvar_SetFloatByRef (&vid_height, 480 /*(float)(vid.DesktopHeight - 64)*/);

		Con_DevPrintf (DEV_VIDEO, "      Adjusted size to %i x %i\n", (int)vid_width.integer, (int)vid_height.integer);
	}

	if (vid_width.integer > (float)(vid.DesktopWidth - 64 /* no reason*/) || vid_height.integer > (float)(vid.DesktopHeight - 64 /* no reason*/))
	{
		Con_DevPrintf (DEV_VIDEO, "Note: Requested windowed mode exceeds \"safe\" desktop size .. \n");
		Con_DevPrintf (DEV_VIDEO, "      Requested size: %i x %i\n", (int)vid_width.integer, (int)vid_height.integer);
		Con_DevPrintf (DEV_VIDEO, "      Desktop   size: %i x %i\n", vid.DesktopWidth, vid.DesktopHeight);

		Cvar_SetFloatByRef (&vid_width,  640 /*(float)(vid.DesktopWidth  - 64)*/);
		Cvar_SetFloatByRef (&vid_height, 480 /*(float)(vid.DesktopHeight - 64)*/);

		Con_DevPrintf (DEV_VIDEO, "      Adjusted size to %i x %i\n", (int)vid_width.integer, (int)vid_height.integer);
	}

	if (vid_bpp.integer > (float)vid.DesktopBPP)
	{
		Con_DevPrintf (DEV_VIDEO, "Note: Requested windowed BPP of %i exceeds Desktop BPP of %i\n", (int)vid_bpp.integer, (float)vid.DesktopBPP);
		Cvar_SetFloatByRef (&vid_bpp, vid.DesktopBPP);
		Con_DevPrintf (DEV_VIDEO, "      Adjusted bpp to %i\n", (int)vid_bpp.integer);
	}
}


void VID_Windowed(void)
{
	// Only do if video mode switching is enabled?  Yes

	if (!host_initialized)
		return;

	if (modestate == MODE_WINDOWED)
	{
		Con_DevPrintf (DEV_VIDEO, "VID_Windowed: Already windowed\n");
		return;
	}

	if (vid_isLocked)
	{
		Con_Printf("VID_Windowed: Video mode switching is locked\n");
		return;
	}

	Cvar_SetFloatByRef(&vid_fullscreen, 0);
	Con_DevPrintf (DEV_VIDEO, "vid_fullscreen set to 0; Requesting restart\n");

	PreQualifyWindowedModeVersusDesktop ();	// Adjusts cvars if windowed mode violates rules

	VID_Restart_f();
}

qbool VID_CanSwitchedToWindowed(void)
{

	if (!host_initialized)
		return 0; // can't

	if (modestate == MODE_WINDOWED)
		return 0; // can't; already are

	if (vid_isLocked)
		return 0; // can't switch modes

	if (vid_fullscreen_only)
		return 0; // can't switch to windowed mode

	return 1; // can and we aren't in it already
}

qbool VID_WindowedSwapAvailable(void)
{
	if (!host_initialized)
		return 0; // can't

	if (vid_isLocked)
		return 0; // can't switch modes

	if (vid_fullscreen_only)
		return 0; // can't switch to/from windowed mode

	return 1; //switchable
}

qbool VID_isFullscreen(void)
{
	if (modestate == MODE_WINDOWED)
		return false;

	return true; // modestate == MODE_FULLSCREEN

}

/*
================
VID_Test -- johnfitz -- like vid_restart, but asks for confirmation after switching modes
================
*/
void VID_Test_f (void)
{
	vmode_t oldmode;
	qbool	mode_changed = false;

	if (vid_isLocked)
		return;
//
// check cvars against current mode
//
	if (vid_fullscreen.integer || vid_fullscreen_only)
	{
		if (video_modes[vid_default].displaymode == MODE_WINDOWED)
			mode_changed = true;
		else if (video_modes[vid_default].bpp != (int)vid_bpp.integer)
			mode_changed = true;
		else if (video_modes[vid_default].refreshrate != (int)vid_displayfrequency.integer)
			mode_changed = true;
	}
	else
		if (video_modes[vid_default].displaymode != MODE_WINDOWED)
			mode_changed = true;

	if (video_modes[vid_default].width != (int)vid_width.integer ||
		video_modes[vid_default].height != (int)vid_height.integer)
		mode_changed = true;

	if (!mode_changed)
		return;
//
// now try the switch
//
	oldmode = video_modes[vid_default];

	VID_Restart_f ();

	//pop up confirmation dialoge
	if (SCR_ModalMessage("\bKeep Video Mode?\b\n\nWould you like to keep this\nvideo mode? (y/n)\n", 15.0f, NULL)<=1)
	{
		//revert cvars and mode
		Cvar_SetStringByRef (&vid_width, va("%i", oldmode.width));
		Cvar_SetStringByRef (&vid_height, va("%i", oldmode.height));
//		Cvar_SetStringByRef (&vid_bpp, va("%i", oldmode.bpp));
		Cvar_SetStringByRef (&vid_displayfrequency, va("%i", oldmode.refreshrate));
		Cvar_SetStringByRef (&vid_fullscreen, ((vid_fullscreen_only) ? "1" : (oldmode.displaymode == MODE_WINDOWED) ? "0" : "1"));
		VID_Restart_f ();
	}
}


/*
================
VID_Unlock -- johnfitz
================
*/
void VID_Unlock_f (void)
{
	vid_isLocked = false;

	VID_SyncCvarsToMode (vid_default);

	//sync up cvars in case they were changed during the lock
//	Cvar_SetStringByRef (&vid_width, va("%i", video_modes[vid_default].width));
//	Cvar_SetStringByRef (&vid_height, va("%i", video_modes[vid_default].height));
//	Cvar_SetStringByRef (&vid_displayfrequency, va("%i", video_modes[vid_default].refreshrate));
//	Cvar_SetStringByRef (&vid_fullscreen, (vid_fullscreen_only) ? "1" : ((windowed) ? "0" : "1"));
}



/*
=================
GL_BeginRendering -- sets values of glx, gly, glwidth, glheight
=================
*/
void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
	*x = *y = 0;
	*width = ClientDrawingRect.right - ClientDrawingRect.left;
	*height = ClientDrawingRect.bottom - ClientDrawingRect.top;
}


#if RELEASE_MOUSE_FULLSCREEN
// We will release the mouse if fullscreen under several circumstances
// but specifically NOT if connected to a server that isn't us
// In multiplayer you wouldn't want to release the mouse by going to console
// But we'll say it's ok if you went to the menu
#define MOUSE_RELEASE_GAME_SAFE  (cls.state != ca_connected || sv.active==true || key_dest == key_menu || cls.demoplayback)
//|| cls.demoplayback || key_dest == key_menu || sv.active)
#endif



/*
=================
GL_EndRendering
=================
*/
extern qbool keydestnamemaker;
void GL_EndRendering (void)
{
	Gamma_Apply_Maybe ();

	if (!scr_skipupdate)
	{
		// If we aren't drawing, we aren't playing with the vsync either.
#if RENDERER_OPENGL_AVAILABLE	// Because DIRECT3D doesn't even use this!
		if (wglSwapIntervalEXT && update_vsync && vid_vsync.string[0])
			wglSwapIntervalEXT (vid_vsync.integer ? 1 : 0);
#endif

		update_vsync = false;

#if RENDERER_DIRECT3D_AVAILABLE
		if (engine.Renderer->graphics_api == RENDERER_DIRECT3D)
			FakeSwapBuffers();
		else
#endif
			SwapBuffers (maindc);

	}

	// handle the mouse state when windowed if that's changed
	if (modestate == MODE_WINDOWED)
	{
		if (!_windowed_mouse.integer)
		{
			if (windowed_mouse)
			{
				IN_Mouse_Unacquire ();
				windowed_mouse = false;
			}
		}
		else
		{
			windowed_mouse = true;
			if (!in_mouse_acquired && key_dest == key_game && ActiveApp)
				IN_Mouse_Acquire ();
			else if (in_mouse_acquired && key_dest != key_game)
				IN_Mouse_Unacquire ();
		}
	}
#if RELEASE_MOUSE_FULLSCREEN // Baker release mouse even when fullscreen
	else
	{
		windowed_mouse = true;
		if (!in_mouse_acquired && key_dest == key_game && ActiveApp)
			IN_Mouse_Acquire ();
		else if (in_mouse_acquired && key_dest != key_game)
		{
			if (MOUSE_RELEASE_GAME_SAFE)
			{
				IN_Mouse_Unacquire ();
	//			Con_Printf("Debug: Mouse shown\n", in_mouse_acquired);
			}
		}
	}
#endif


	if (fullsbardraw)
		Sbar_Changed (); // Baker: WTF


	// Baker: a refresh think here

#if RENDERER_DIRECT3D_AVAILABLE && 0
	if (engine.Renderer->graphics_api == RENDERER_DIRECT3D && vid_d3d_dirty && (realtime-vid_d3d_dirty_time)>0.5f)
	{	vid_restart_no_check_same_mode = true;
		VID_SyncCvars (vid_default);
		VID_Restart_f ();
		vid_d3d_dirty = false;
		vid_d3d_dirty_time = 0;
	}
#endif

}

void VID_SetDefaultMode (void)
{
	IN_Mouse_Unacquire ();
}

void VID_Shutdown (void)
{
   	HGLRC	hRC;
   	HDC	hDC;

	if (!vid_initialized) return; // Video not inited

	Gamma_RestoreHWGamma ();

	// Destroy stuff
	vid_canalttab = false;
	hRC = ewglGetCurrentContext ();
	hDC = ewglGetCurrentDC ();
	ewglMakeCurrent (NULL, NULL);

	// Delete context too
	if (hRC)
	{
		ewglDeleteContext (hRC); hRC = 0;
	}

	Gamma_Shutdown (); //johnfitz

	if (hDC && mainwindow)
	{
		ReleaseDC (mainwindow, hDC); hDC = 0;
	}

	if (modestate == MODE_FULLSCREEN)
	{
		eChangeDisplaySettings (NULL, 0);
		VID_UpdateDesktopProperties ();
	}

	if (maindc && mainwindow)
	{
		ReleaseDC (mainwindow, maindc);  maindc = 0;
	}


	AppActivate (false, false);

#if SUPPORTS_DUAL_MONITOR_HACK
	{
		extern int monitors_swapped, num_monitors;
		if (monitors_swapped)
			RestorePrimaryMonitor ();
	}
#endif
}



int bChosePixelFormat(HDC hDC, PIXELFORMATDESCRIPTOR *pfd, PIXELFORMATDESCRIPTOR *retpfd)
{
	int	pixelformat;

	if (!(pixelformat = ChoosePixelFormat(hDC, pfd)))
	{
		MessageBox (NULL, "ChoosePixelFormat failed", "Error", MB_OK);
		return 0;
	}

	if (!(DescribePixelFormat(hDC, pixelformat, sizeof(PIXELFORMATDESCRIPTOR), retpfd)))
	{
		MessageBox(NULL, "DescribePixelFormat failed", "Error", MB_OK);
		return 0;
	}

	return pixelformat;
}


BOOL bSetupPixelFormat (HDC hDC)
{
	static PIXELFORMATDESCRIPTOR retpfd, pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,								// version number
		PFD_DRAW_TO_WINDOW | 			// support window
		PFD_SUPPORT_OPENGL |			// support OpenGL
		PFD_DOUBLEBUFFER ,				// double buffered
		PFD_TYPE_RGBA,					// RGBA type
		24,								// 24-bit color depth
		0, 0, 0, 0, 0, 0,				// color bits ignored
		0,								// no alpha buffer
		0,								// shift bit ignored
		0,								// no accumulation buffer
		0, 0, 0, 0, 					// accum bits ignored
		32,								// 32-bit z-buffer
		0,								// no stencil buffer
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main layer
		0,								// reserved
		0, 0, 0							// layer masks ignored
	};
	int	pixelformat;

	if ((pixelformat = bChosePixelFormat(hDC, &pfd, &retpfd))== 0)
		return FALSE;


	if (retpfd.cDepthBits < 24)
	{
		pfd.cDepthBits = 24;
		if ((pixelformat = bChosePixelFormat(hDC, &pfd, &retpfd))== 0)
		return FALSE;
	}

	if (eSetPixelFormat(hDC, pixelformat, &retpfd) == FALSE)
	{
		MessageBox (NULL, "SetPixelFormat failed", "Error", MB_OK);
		return FALSE;
	}

//	if (retpfd.cDepthBits < 24)
//		gl_allow_ztrick = false;

	return TRUE;
}

//==========================================================================
//
//  INIT
//
//==========================================================================



void VID_AddBasicWindowedMode (void);		// Add default windowed mode   "0"
void VID_AddBasicFullScreenMode (void);		// Add fullscreen default mode "1'
void VID_AddAllFullScreen_Modes (void);		// The rest



void VID_LockCvars_Due_To_CommandLine (void)
{
//	vid_width
	vid_isLocked = true;
}



void VID_DescribeCurrentMode_f (void);
void VID_DescribeModes_f       (void);

/*
===================
VID_Init
===================
*/
qbool first_time_engine_used = false;
void VID_Init (void)
{
	// Get desktop properties, warn if someone is using stupid settings that foobar -window possibility
	// Like 16 colors
	VID_UpdateDesktopProperties ();

	if (vid.DesktopBPP <= 8)
	{
		MessageBox(NULL, "Warning: Only fullscreen available", "Your desktop color depth does not support RGB color.  Only fullscreen video modes will be available", MB_OK);
		// Baker: should do something here to restrict this, but ... well ... wtf.
	}

	Cvar_Registration_Client_Video ();

	// Externals are preferences that are not related to the gamedir at all
	// We store them in a separate file in the enginedir and they can be
	// read early.
	//
	// Things like video modes or turning mp3 tracks on, etc.
	// Be very leery and conservative of these.  A player might want
	// different mouse sensitivity for, say, Kurok or some other mod.

	// Things like screen resolution, use of directinput or gamma should
	// be global preferences.

	// Also quick locking cvars.  I don't think any of them should be
	// locked.  Instead implement command line stuff correctly?

	Cvar_GetExternal (&vid_fullscreen);			//honor_externals = true;
	Cvar_GetExternal (&vid_width);				// honor_externals = true;
	Cvar_GetExternal (&vid_height);				// honor_externals = true;
	Cvar_GetExternal (&vid_bpp);				// honor_externals = true;
	Cvar_GetExternal (&vid_displayfrequency);	// honor_externals = true;

#if RENDERER_DIRECT3D_AVAILABLE	// Check for vsync before defining the window the first time (we have to know first)
	if (engine.Renderer->graphics_api == RENDERER_DIRECT3D)
		Cvar_GetExternal (&vid_vsync);				//        setting up a window the first time
	
#endif


	if (!Cvar_GetExternal (&session_quickstart))			// Registered earlier
	{
		// Assumed to be first time use if we cannot read this cvar
		first_time_engine_used = true;
		Con_DevPrintf (DEV_GAMEDIR, "First time engine startup detected\n");
	}
	// Baker: First ... this is a client cvar.  Second this is way too early, we haven't even read quickstart yet.
//	Cvar_KickOnChange (&session_quickstart);	// Cvar needs to execute onchange event to take effect



	//Cvar_SetDefaultFloatByRef (&vid_bpp, vid.DesktopBPP);

//	Cvar_SetFlagByRef (&vid_bpp, CVAR_ROM);		// Should be CVAR_INIT ideally.  Fix CVAR_INIT message.

	

	Cvar_GetExternal (&vid_brightness_method);

#if SUPPORTS_GLVIDEO_MODESWITCH // D3D DOES NOT SUPPORT THIS
	Cmd_AddCommand ("video_unlock", VID_Unlock_f); //johnfitz
	Cmd_AddCommand ("video_restart", VID_Restart_f); //johnfitz
	Cmd_AddCommand ("video_test", VID_Test_f); //johnfitz
#endif // D3D dies on video mode change

	Cmd_AddCommand ("gl_info", GL_PrintExtensions_f);
	Cmd_AddCommand ("vid_describecurrentmode", VID_DescribeCurrentMode_f);
	Cmd_AddCommand ("vid_describemodes", VID_DescribeModes_f);
	Cmd_AddCommand ("vid_desktopsettings", VID_DescribeDesktop_f);
	{
		void VID_Intro_Run (void);
		Cmd_AddCommand ("run_intro", VID_Intro_Run);
	}

#if SUPPORTS_DUAL_MONITOR_HACK
	if (COM_CheckParm("-monitor2"))
	{
		extern int num_monitors;
		CollectMonitorInformation ();
		if (num_monitors == 2)  // This is written in a 2-monitor way, don't have a third monitor to test triple monitor
			SwapPrimaryMonitor (); // Set monitors_swapped to 1
	}
#endif

	hIcon = LoadIcon (global_hInstance, MAKEINTRESOURCE (IDI_ICON2));
#if _DEBUG
	if (!hIcon)
		MessageBox (NULL, "Failed to load icon", NULL, MB_OK);

#endif
	OS_WINDOWS_RegisterClassFrame (global_hInstance);


// Baker: Vista style popup boxes.  Who cares.
//	InitCommonControls();

	// User command line params are read in these functions
	VID_AddBasicWindowedMode ();		// Add default windowed mode   "0"
	VID_AddBasicFullScreenMode ();

	// Baker: Can't do this yet!  We must process user mode overrides first.
	// VID_AddAllFullScreen_Modes ();

	// Allow command line parameters to override mode settings
	// In future ... lock down those cvars cold with CVAR_ROM.

	// Baker: by doing this now, the list can be right
	// We didn't know if mode 1 was customized before now.
	VID_AddAllFullScreen_Modes ();

	if (COM_CheckParm("-fullwindow") || COM_CheckParm("-window"))
	{	// Override windowed mode settings

		user_video_mode_forced = true;

		if (COM_CheckParm("-fullwindow"))
		{
			video_modes[0].width =		vid.DesktopWidth;
			video_modes[0].height =		vid.DesktopHeight;
		}
		else // COM_CheckParm("-window")
		{
			int i = 0;
			if ((i = COM_CheckParm("-width")) && i + 1 < com_argc)
				video_modes[0].width = atoi(com_argv[i+1]);
			if ((i = COM_CheckParm("-height")) && i + 1 < com_argc)
				video_modes[0].height= atoi(com_argv[i+1]);
			// Baker: I don't normally support bpp for windowed
			// but we'll need it to test texture manager
			if ((i = COM_CheckParm("-bpp")) && i + 1 < com_argc)
				video_modes[0].bpp = atoi(com_argv[i+1]);
		}

		// Unwilling to go under 320x200 Or exceed desktop width
		// Limits to size.  Nah ... let user play or get errors

		// video_modes[0].width  =  CLAMP (320, video_modes[0].width,  vid.DesktopWidth);
		// video_modes[0].height =  CLAMP (200, video_modes[0].height, vid.DesktopHeight);
		// bpp limit to desktop or 16 or 32 could go here
		// but instead, let it error

		Rebuild_Mode_Description (0);
		vid_default = MODE_WINDOWED;
	}
	else
	if (COM_CheckParm("-mode"))
	{
		int i = COM_CheckParm("-mode");
		user_video_mode_forced = true;
		vid_default = atoi(com_argv[i+1]);
	}
	else
	if (COM_CheckParm ("-current") || COM_CheckParm ("-width") || COM_CheckParm("-height") || COM_CheckParm("-bpp") || COM_CheckParm("-freq"))
	{	// Override fullscreen default mode settings
		// Baker: Override if specified
		user_video_mode_forced = true;

		if (COM_CheckParm("-current"))
		{
			Cvar_SetFloatByRef(&vid_width,  (float)vid.DesktopWidth);
			Cvar_SetFloatByRef(&vid_height, (float)vid.DesktopHeight);
		}
		else
		{
			int i=0;
			Cvar_SetFloatByRef(&vid_fullscreen,  1);

			if ((i = COM_CheckParm("-width")) && i + 1 < com_argc)		Cvar_SetFloatByRef(&vid_width,  atof(com_argv[i+1]));
			if ((i = COM_CheckParm("-height")) && i + 1 < com_argc)	Cvar_SetFloatByRef(&vid_height, atof(com_argv[i+1]));
			if ((i = COM_CheckParm("-bpp")) && i + 1 < com_argc)		Cvar_SetFloatByRef(&vid_bpp,    atof(com_argv[i+1]));
			if ((i = COM_CheckParm("-freq")) && i + 1 < com_argc)		Cvar_SetFloatByRef(&vid_displayfrequency, atof(com_argv[i+1]));
	}

		// Cycle though the modes and find the match
		if (!(vid_default=VID_FindFullScreenModeOnCvars()))
// Baker: This can never happen.  Above function will set to 1 if can't find anything
			Sys_Error ("Specified video mode not available\n");

		}
	else
//	if (!user_video_mode_forced)
		{
		 // Determine default mode if we haven't so far
			if (vid_fullscreen.integer)
			{	// Modify mode 1	... but find it
				// We will accept these settings only if they match an existing mode
				int t = VID_FindFullScreenModeOnCvars ();
				vid_default = t;
			}
			else
			{	// Modify mode 0
				//
				PreQualifyWindowedModeVersusDesktop ();	// Adjusts cvars if windowed mode violates rules

				video_modes[0].width = (int)vid_width.integer;
				video_modes[0].height = (int)vid_height.integer;
				video_modes[0].bpp = (int)vid_bpp.integer;
				video_modes[0].refreshrate = vid.DesktopDispFreq; // Well ... ;

// Baker: This variable aint ever even read.  Sheesh.						windowed = true;
				vid_default = MODE_WINDOWED;

			}
		}

	vid_initialized = true;

//	vid.maxwarpwidth = WARP_WIDTH;
//	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));


	{ // Windowing exceptions
		extern int gWindowStandardStyle;

		if (user_video_mode_forced)
			gWindowStandardStyle &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX);		// Remove WS_SIZEBOX | WS_MAXIMIZEBOX as an option since settings are locked

		// Baker: August 24, 2011 ... this renderings all black and white striped and shitty on Vista due to
		// some sort of screen overlap with the taskbar?  Either way, I'd love to enable it but I cannot  
		if (engine.Renderer->graphics_api == RENDERER_DIRECT3D)
			gWindowStandardStyle &= ~WS_MAXIMIZEBOX;					// Maximize box in Direct3D isn't working well for me
//			gWindowStandardStyle &= ~ WS_SIZEBOX;		// Remove WS_SIZEBOX | WS_MAXIMIZEBOX as an option since settings are locked
//			gWindowStandardStyle &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX);		// Remove WS_SIZEBOX | WS_MAXIMIZEBOX as an option since settings are locked

	}


	VID_SetMode (vid_default, /* isReset = */ false); // This is initialization

	{ // Setup of wgl
		HGLRC	baseRC; //johnfitz -- moved here from global scope, since it was only used in this

		maindc = GetDC (mainwindow);
		if (!bSetupPixelFormat(maindc))
			Sys_Error ("bSetupPixelFormat failed");

		engine.HWGamma = Gamma_Init ();

		baseRC = ewglCreateContext( maindc );
		if (!baseRC)
			Sys_Error ("Could not initialize GL (ewglCreateContext failed).\n\nMake sure you are in 65535 color mode, and try running -window.");
		if (!ewglMakeCurrent(maindc, baseRC))
			Sys_Error ("VID_Init: ewglMakeCurrent failed");

		GL_Init ();

#if INTRO_ANIMATION
		if (!session_quickstart.integer)
		{
			void VID_Intro_Run (void);

//			Baker to self: Give it up.  It won't draw.
//			if (engine.Renderer->graphics_api == RENDERER_DIRECT3D && vid_modenum>0) // Bad visuals for D3D and fullscreen) // Bad visuals for D3D and fullscreen because D3D clears to black on setmode ... so we don't get nice transitition
//				VID_PaintTiles ();

//			ShowWindow (mainwindow, SW_SHOWNORMAL);
			VID_Intro_Run ();

		}
#endif

		CheckVsyncControlExtensions ();	// Baker: Doing this late to avoid vsync during intro for OpenGL; Direct3D ignores this function essentially

	}



#if SUPPORTS_GLVIDEO_MODESWITCH // Right?
	vid_menucmdfn = VID_Menu_f; //johnfitz
	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;
#endif

//	vid_realmode = vid_modenum;
	strcpy (badmode.modedesc, "Bad mode");
	vid_canalttab = true;

	if (COM_CheckParm("-fullsbar"))
		fullsbardraw = true;

	VID_Menu_Init(); //johnfitz

	if (user_video_mode_forced)
		VID_LockCvars_Due_To_CommandLine ();

	if (user_video_mode_forced && vid_default > 0 && video_modes[vid_default].bpp != vid.DesktopBPP)
		vid_fullscreen_only = true; // Can't switch
}



