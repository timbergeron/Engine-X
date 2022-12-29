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
// sys_win_activate.c -- Win32 system interface code


#include "quakedef.h"
#include "winquake.h"
#include "winquake_video_modes.h"

/*
===================================================================

MAIN WINDOW

===================================================================
*/

qbool vid_wassuspended = false;

extern qbool vid_canalttab;

// vid_modes.c
void VIDMODES_SetLastMode (void);
void VIDMODES_AltTabFix (void);


qbool regained_focus = false;
float regained_focus_time = 0;


void AppActivate (BOOL fActive, BOOL minimize)
{
/****************************************************************************
*
* Function:     AppActivate
* Parameters:   fActive - True if app is activating
*
* Description:  If the application is activating, then swap the system
*               into SYSPAL_NOSTATIC mode so that our palettes will display
*               correctly.
*
****************************************************************************/
	static BOOL	sound_active;   /// Um?  How

	ActiveApp = fActive;
	Minimized = minimize;	 // Note, minimizing loses focus

// enable/disable sound on focus gain/loss
	if (!ActiveApp && sound_active)
	{
		S_BlockSound ();
		S_ClearBuffer ();
#if SUPPORTS_MP3_TRACKS
		// Need to pause CD music here if is playing
		if (engine.sound && engine.sound->sound_started)	// Dumb ... this has nothing to do with if an MP3 is playing
			MP3Audio_Pause ();
#endif
		sound_active = false;
	}
	else if (ActiveApp && !sound_active)
	{
		// Engine gaining focus ... turn on the sounds

		S_UnblockSound ();
#if SUPPORTS_MP3_TRACKS
		// Need to unpause CD music here if was playing
		if (engine.sound && engine.sound->sound_started)	// Dumb ... this has nothing to do with if an MP3 is playing
			MP3Audio_Resume ();
#endif
		sound_active = true;
	}

	if (fActive)
	{
		// Baker: Better to get desktop properties when receiving active state than when losing it
		if (vid_canalttab && vid_wassuspended)
		{
			VID_UpdateDesktopProperties ();
		}

		regained_focus = true;
		regained_focus_time = realtime;

		if (modestate == MODE_FULLSCREEN)
		{

			IN_Mouse_Acquire ();


			if (vid_canalttab && vid_wassuspended)
			{
				vid_wassuspended = false;


				VIDMODES_SetLastMode ();

				ShowWindow (mainwindow, SW_SHOWNORMAL);

				// Fix for alt-tab bug in NVidia drivers
				VIDMODES_AltTabFix ();

				// scr_fullupdate = 0;
				Sbar_Changed ();
			}
		}
		else if (modestate == MODE_WINDOWED && Minimized)
		{
			ShowWindow (mainwindow, SW_RESTORE);
		}
		 else if ((modestate == MODE_WINDOWED) && _windowed_mouse.integer && key_dest == key_game)
		{
			IN_Mouse_Acquire ();
		}

		IN_Keyboard_Acquire ();

		if (engine.HWGamma && engine.HWGamma->hwgamma_enabled)
//			if (vid_canalttab /*&& !Minimized */ /*&& currentgammaramp */)
				Gamma_SetUserHWGamma (); // VID_SetDeviceGammaRamp (currentgammaramp);

	}

	if (!fActive)
	{
		if (modestate == MODE_FULLSCREEN)
		{

			IN_Mouse_Unacquire ();

			if (vid_canalttab) // Baker: dual monitor situation doesn't need to do this!  Or does it?
			{
				eChangeDisplaySettings (NULL, 0);

				vid_wassuspended = true;
			}
		}
		else if ((modestate == MODE_WINDOWED) && _windowed_mouse.integer)
		{
			IN_Mouse_Unacquire ();

		}

		IN_Keyboard_Unacquire ();

		if (engine.HWGamma && engine.HWGamma->hwgamma_enabled) // Baker: let's see what happens
			Gamma_RestoreHWGamma ();

	}
}

