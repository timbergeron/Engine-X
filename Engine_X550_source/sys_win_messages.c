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
// sys_win_messages.c -- Win32 message handler code

#include "quakedef.h"
#include "winquake.h"
#include "winquake_video_modes.h"

void Sys_SendKeyEvents (void)
{
	MSG	msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
	// we always update if there are any events, even if we're paused
		scr_skipupdate = 0;

		if (!GetMessage(&msg, NULL, 0, 0))
			Sys_Quit ();

		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}



LONG MP3Audio_MessageHandler (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int IN_MapKey (int key);
extern int 	key_special_dest;
void VID_UpdateWindowStatus (void);
void AppActivate (BOOL fActive, BOOL minimize);
int VID_ResizeWindow ();



int 		extmousex, extmousey; // Baker: for tracking Windowed mouse coordinates


#ifndef WM_MOUSEWHEEL
#define	WM_MOUSEWHEEL	0x020A
#endif

#define MK_XBUTTON1         0x0020
#define MK_XBUTTON2         0x0040

/* main window procedure */

#if RENDERER_DIRECT3D_AVAILABLE
extern int vid_d3d_minwindow_x;
extern int vid_d3d_minwindow_y;
#endif
extern qbool in_setmode;




LONG WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LONG		lRet = 1;
	int	fActive, fMinimized, temp;
	char	state[256];
	char	asciichar[4];
	int		vkey;
	int		charlength;
	qbool 	down = false;

//	if (developer.integer > 1)
//		Con_Printf ("Windows message: %i\n", (int)uMsg);

	switch (uMsg)
	{
	case WM_GETMINMAXINFO:
		{
			RECT windowrect;
			RECT clientrect;
			MINMAXINFO *mmi = (MINMAXINFO *) lParam;

			GetWindowRect (hWnd, &windowrect);
			GetClientRect (hWnd, &clientrect);

#if RENDERER_DIRECT3D_AVAILABLE && 0 // Baker: This causes problems beyond belief for somereason
			if (engine.Renderer->graphics_api == RENDERER_DIRECT3D && !in_setmode && (vid_d3d_minwindow_x || vid_d3d_minwindow_y))
			{ // Baker: prevents resize oddities with Direct3D wrapper in its current state
				mmi->ptMinTrackSize.x = vid_d3d_minwindow_x + ((windowrect.right - windowrect.left) - (clientrect.right - clientrect.left));
				mmi->ptMinTrackSize.y = vid_d3d_minwindow_y + ((windowrect.bottom - windowrect.top) - (clientrect.bottom - clientrect.top));
			}
			else
#endif
			{

				mmi->ptMinTrackSize.x = 320 + ((windowrect.right - windowrect.left) - (clientrect.right - clientrect.left));
				mmi->ptMinTrackSize.y = 200 + ((windowrect.bottom - windowrect.top) - (clientrect.bottom - clientrect.top));
			}
		}

		return 0;


	case WM_SIZE:

		
		


		if (in_setmode)
		{
			short lparam_width  = (int)(((short*)&lParam)[0]);
			short lparam_height = (int)(((short*)&lParam)[1]);

			Con_DevPrintf (DEV_VIDEO, "WM_SIZE message rejected: Setting video mode (wparam %i lparam %i, lparam shorts: %i %i)\n", wParam, lParam, (int)lparam_width, (int)lparam_height );
			return 0;
		}

#if 0
		{
			short lparam_width  = (int)(((short*)&lParam)[0]);
			short lparam_height = (int)(((short*)&lParam)[1]);
			if (Minimized)
			{	
				Con_DevPrintf (DEV_VIDEO, "WM_SIZE message rejected: Application minimized (wparam %i lparam %i, lparam shorts: %i %i)\n", wParam, lParam, (int)(((short*)&lParam)[0]), (int)(((short*)&lParam)[1]) );
///BCUR				VID_ResizeWindow (); // Grows by 144 ?  What is 144?  ProQuake doesn't grow by 144?
				// Even if I eliminate Window resizing it still grows by 144.  
				return 0;
			}
		}
#endif	
		Con_DevPrintf (DEV_VIDEO, "WM_SIZE message accepted (wparam %i lparam %i, lparam shorts: %i %i)\n", wParam, lParam, (int)(((short*)&lParam)[0]), (int)(((short*)&lParam)[1]) );

		VID_ResizeWindow ();
		return 0;					

//		
//		switch (wParam)												// Evaluate Size Action
//		{
///			case SIZE_MINIMIZED:									// Was Window Minimized?
//				return 0;
//		
//			case SIZE_MAXIMIZED:									// Was Window Maximized?
//			case SIZE_RESTORED:
//				
//				// Apparently maximize or anti
//				VID_ResizeWindow (LOWORD (lParam), HIWORD (lParam));
//				return 0;					
//
//			default:
//				break;
//		}
//		// Ignore all the other messages for now
//		return 0;
/*
 * WM_SIZE message wParam values

#define SIZE_RESTORED       0
#define SIZE_MINIMIZED      1
#define SIZE_MAXIMIZED      2
#define SIZE_MAXSHOW        3
#define SIZE_MAXHIDE        4
*/

//		Con_Printf ("Size message with wParam %i\n", wParam);
//
//		if (/*in_setmode &&*/ wParam == SIZE_MAXIMIZED)
//		{
//			Con_Printf ("Maximize request while setting mode\n");
//			lRet = 0;
//			break;
//		}

//		if (wParam == 0)
//		{
//			// Tricky we usually want to honor this
//			// Except if we just switched from fullscreen to windowed in
//			// a restart
//			extern qbool full_screen_to_windowed;
//			extern double full_screen_to_windowed_time;
//			if (full_screen_to_windowed && Sys_DoubleTime() < (full_screen_to_windowed_time + 3))
//			{
//				full_screen_to_windowed=false;	// Ignore once
//				break;
//			}
//
//			break;		// A restore message.  But we have that covered.
//		}

//		switch (wParam)												// Evaluate Size Action
//		{
//			case SIZE_MINIMIZED:									// Was Window Minimized?
//				window->isVisible = FALSE;							// Set isVisible To False
//				return 0;
//		
//			case SIZE_MAXIMIZED:									// Was Window Maximized?
//				window->isVisible = TRUE;							// Set isVisible To True
//				ReshapeGL (LOWORD (lParam), HIWORD (lParam));		// Reshape Window - LoWord=Width, HiWord=Height
//				VID_ResizeWindow (LOWORD (lParam), HIWORD (lParam));
//				return 0;												
//
//			case SIZE_RESTORED:										// Was Window Restored?
//				window->isVisible = TRUE;							// Set isVisible To True
//				ReshapeGL (LOWORD (lParam), HIWORD (lParam));		// Reshape Window - LoWord=Width, HiWord=Height
//				VID_ResizeWindow (LOWORD (lParam), HIWORD (lParam));
//				return 0;											// Return
//		}
		

//		return VID_ResizeWindow (LOWORD (lParam), HIWORD (lParam));
//		break;

	case WM_CREATE:
		break;
	case WM_SYSCHAR:  // keep Alt-Space from happening
		break;




	case WM_KILLFOCUS:
		if (modestate == MODE_FULLSCREEN)
			ShowWindow(mainwindow, SW_SHOWMINNOACTIVE);
		break;

	case WM_MOVE:
		{
			extern int window_x, window_y;
			window_x = (int) LOWORD(lParam);	// Assigned
			window_y = (int) HIWORD(lParam);	// Assigned
			VID_UpdateWindowStatus ();
		}
		break;


	case WM_CLOSE:
		if (!session_confirmquit.integer || MessageBox(mainwindow, "Are you sure you want to quit?", "Confirm Exit", MB_YESNO | MB_SETFOREGROUND | MB_ICONQUESTION) == IDYES)
			Sys_Quit ();

	    return 0;

	case WM_ACTIVATE:
		fActive = LOWORD(wParam);
		fMinimized = (BOOL)HIWORD(wParam);
		Con_DevPrintf (DEV_VIDEO, "WM_ACTIVATE message: Received either an activate or minimize request isMinimizing = %i\n", fMinimized);
		AppActivate (!(fActive == WA_INACTIVE), fMinimized);

		// fix the leftover Alt from any Alt-Tab or the like that switched us away
		Key_ClearAllStates ();
		// MH updates the clipcursor here

		return 1;
		break;
	case WM_GRAPHNOTIFY:
		return MP3Audio_MessageHandler (hWnd, uMsg, wParam, lParam);

	case WM_DESTROY:
		{
//			extern HWND dibwindow;
			if (mainwindow)
			{
				DestroyWindow (mainwindow);  mainwindow = 0;
			}

			PostQuitMessage (0);
		}
		break;

	case WM_MOUSEWHEEL:
		if ((short)HIWORD(wParam) > 0)
		{
			Key_Event(K_MWHEELUP, 0, true);
			Key_Event(K_MWHEELUP, 0, false);
		}
		else
		{
			Key_Event(K_MWHEELDOWN, 0, true);
			Key_Event(K_MWHEELDOWN, 0, false);
		}
		break;


	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		down=true;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		// Baker 3.703 - old way
		// Key_Event (IN_MapKey(lParam), false);
		vkey = IN_MapKey(lParam);
		GetKeyboardState (state);
		// alt/ctrl/shift tend to produce funky ToAscii values,
		// and if it's not a single character we don't know care about it
		charlength = ToAscii (wParam, lParam >> 16, state, (unsigned short *)asciichar, 0);

		if (vkey == K_ALT || vkey == K_CTRL || vkey == K_SHIFT || charlength == 0)
			asciichar[0] = 0;
		else if( charlength == 2 )
		{
			asciichar[0] = asciichar[1];
		}

		Key_Event (vkey, asciichar[0], down);
		break;



// this is complicated because Win32 seems to pack multiple mouse events into
// one update sometimes, so we always check all states and look for events
	case WM_LBUTTONUP:
		// Mouse isn't active + special destination
		// means Quake doesn't control mouse
		if (key_special_dest && !in_mouse_acquired)
		{
			extmousex = Q_rint((float)LOWORD(lParam)*((float)vid.width/(float)glwidth)); //Con_Printf("Mouse click x/y %d/%d\n", extmousex, extmousey);
			extmousey = Q_rint((float)HIWORD(lParam)*((float)vid.height/(float)glheight));
			Key_Event (K_MOUSECLICK_BUTTON1, 0, false);
			break;
		}
	case WM_RBUTTONUP:
		// Mouse isn't active + special destination
		// means Quake doesn't control mouse
		if (key_special_dest && !in_mouse_acquired)
		{
			Key_Event (K_MOUSECLICK_BUTTON2, 0, false);
			break;
		}

// Since we are not trapping button downs for special destination
// like namemaker or customize controls, we need the down event
// captures to be below the above code so it doesn't filter into it
// The code below is safe due to the "& MK_xBUTTON" checks
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
		temp = 0;

		if (wParam & MK_LBUTTON)
			temp |= 1;

		if (wParam & MK_RBUTTON)
			temp |= 2;

		if (wParam & MK_MBUTTON)
		{
			if (key_special_dest && !in_mouse_acquired)
			{
				// Allow button to be bound in customize controls
				// Even if we don't have mouse acquired
				// Really we should be check for -nomouse and stuff
				// Silly to bind a mouse button if -nomouse was specified
				// EXCEPT that we are using this for GUI stuff so
				// live with that little quibble.
				Key_Event (K_MOUSECLICK_BUTTON3, 0, false);
				break; // Get out
			}
			temp |= 4;
		}

		if (wParam & MK_XBUTTON1)
		{
			if (key_special_dest && !in_mouse_acquired)
			{
				Key_Event (K_MOUSECLICK_BUTTON4, 0, false);
				break; // Get out
			}
			temp |= 8;
		}

		if (wParam & MK_XBUTTON2)
		{
			if (key_special_dest && !in_mouse_acquired)
			{
				Key_Event (K_MOUSECLICK_BUTTON5, 0, false);
				break; // Get out
			}
			temp |= 16;
		}

		IN_Mouse_Event (temp);

		break;



	default:
		// pass all unhandled messages to DefWindowProc
		lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
		break;
	}

	// return 1 if handled message, 0 if not
	return lRet;
}