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
// Quake is a trademark of Id Software, Inc., (c) 1996 Id Software, Inc. All
// rights reserved.

// Baker code audit 2011-Sep-27:	Static to every variable possible		_
//									Static to every function possible		_	
//									Const to variables when possible		_
//
//									Replace common functions (strcmp, etc.) 
//									with more readable non-performance
//									impairing macros						_
//
//									Replace common functions with slight			// Baker:	Things that don't run every frame or in a huge loop
//									performance hit in non-critical					//			StrlCopy is slightly slower than an unsafe string copy, for instance
//									situations (error messages, loadup)				//			This doesn't matter in most situations but can sure matter
//									such as StringLCopy						_		//			if it is a loop that runs 10000 times during a game frame.
//
//									Limit usage of "return"					_		// Baker:	Early returns in a function are sloppy to the "workflow"; make cleanup harder
//																								Particularly in a complicated function.  As a general rule of thumb, 
//																								It is my belief that a function should ideally have a single door to exit
//																								I mean, everything enters the function in the same place.
//																								The exception is simple condition checks at the beginning of a function
//																								No need to kludge up code with needless do loops.


/*
==============================================================================================================
MH - this file is now nothing more than a Quake-compatible wrapper for my Direct X music playing routines
(qmp3.cpp).

It should be easy to port it to any other engine as a "drop 'n' go" - you'll also need the WM_GRAPHNOTIFY
stuff in gl_vidnt.c and the appropriate headers and libs.
==============================================================================================================
*/

#include "quakedef.h"

#include <windows.h>

mp3_com_t MP3Com;

char InitMP3DShow (void);
void PlayMP3DShow (const int mp3num);
void KillMP3DShow (void);
void StopMP3DShow (void);
void PawsMP3DShow (const int paused);
void VolmMP3DShow (const int Level);
void MesgMP3DShow (const int looping);
void UserMP3DShow (const char *mp3name, const int verbose);

static void MP3_f (void);

// it's a dumb warning anyway...
#pragma warning(disable : 4761)


void MP3Audio_Stop (void)
{
	if (!engine.sound || !engine.sound->sound_started)	return;
	if (!MP3Com.enabled)								return;

	StopMP3DShow ();
}

static void mp3_Stop (void)
{
	if (!engine.sound->sound_started)					return;  // We aren't checking MP3Com.enabled because we aren't checking it to play?
	
	StopMP3DShow ();
}

void MP3Audio_Pause (void)
{
	if (!engine.sound || !engine.mp3)					return;	// // Hasn't been hooked up ... Get out

	if (!engine.sound->sound_started)					return;
	if (!MP3Com.enabled)								return;

	PawsMP3DShow (0);
}

void MP3Audio_Resume (void)
{
	if (!engine.sound || !engine.sound->sound_started)	return;
	if (!MP3Com.enabled)								return;

	PawsMP3DShow (1);
}

void MP3Audio_Update (void)
{
}

void MP3Audio_Play (byte track, qbool looping)
{
	MP3Com.looping = false;

	if (!engine.sound->sound_started)					return;

	// worldspawn tracks are one ahead of the game
	MP3Com.CurrentTrack = track - 1;

	Con_DevPrintf (DEV_SOUND, "Track name is %s\n", va ("%02i", MP3Com.CurrentTrack));

	// exit conditions
	if ( MP3Com.CurrentTrack > 99)						return;
	if ( MP3Com.CurrentTrack < 0)						return;
	if (!MP3Com.enabled)								return;

	// stop any previous instances
	MP3Audio_Stop ();

	PlayMP3DShow (MP3Com.CurrentTrack);

	MP3Com.looping = looping;
}

qbool OnChange_mp3volume (cvar_t *var, const char *string)
{
	Con_DevPrintf (DEV_SOUND, "bgm change desired\n");

	do
	{
		if (!MP3Com.initialized)
			break;  // Change accepted but no action

		Con_DevPrintf (DEV_SOUND, "setting bgm volume\n");
	
		// set the volume
		VolmMP3DShow ((int) (CLAMP (0, atof(string), 1) * 10));
	} while (0);

	return false;
}

LONG MP3Audio_MessageHandler (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// pass to direct show stuff
	MesgMP3DShow ((int) MP3Com.looping);

	return 0;
}



static void MP3_f (void)
{
	const char	*command = Cmd_Argv (1);
	char		MP3FileName[MAX_OSPATH];

	do
	{

		if (!MP3Com.initialized)
		{
			Con_Printf ("MP3 playback not initialized\n");
			break;
		}

		if (Cmd_Argc() < 2)
		{
			Con_Printf ("Usage: %s {play|stop|on|off|pause|resume} [filename]\n", Cmd_Argv (0));
			Con_Printf ("       Note: music files should be in gamedir/music\n");
			break;
		}

		if (COM_StringMatchCaseless (command, "stop")   )  { MP3Audio_Stop ();			break; }
		if (COM_StringMatchCaseless (command, "resume") )  { MP3Audio_Resume();			break; }
		if (COM_StringMatchCaseless (command, "pause")  )  { MP3Audio_Pause();          break; }


		if (COM_StringMatchCaseless (command, "on"))
		{
			MP3Com.enabled = true;
			Con_Printf ("MP3 playback enabled\n");
			break;
		}

		if (COM_StringMatchCaseless (command, "off"))
		{
	//		if (MP3Com.playing)
			MP3Audio_Stop();
			MP3Com.enabled = false;
			Con_Printf ("MP3 playback disabled\n");
			break;
		}

		if (COM_StringMatchCaseless (command, "play") && Cmd_Argc() >=3)	// Argument is play and there is a 3rd parameter
		{
			// build the MP3 file name
			snprintf (MP3FileName, sizeof(MP3FileName), "%s", Cmd_Argv (2));

			// stop anything that's currently playing

			if (engine.sound->sound_started)
				StopMP3DShow ();
			
			UserMP3DShow (MP3FileName, 1);

			break;

		}

		// Ok ... no valid commands so print how to use it
		
		Con_Printf ("Usage: %s {play|stop} [filename]\n", Cmd_Argv (0));
		Con_Printf ("       Note: music files should be in gamedir/music\n");

	} while (0);

	return;

}


qbool OnChange_mp3_enabled(cvar_t *var, const char *string)
{
	qbool desired = atof(string);

	if (!MP3Com.initialized)
		return true;		// Rejected change ... it is simply disabled

	if (!desired)
		MP3Audio_Stop();	// Shutdown anything playing

	MP3Com.enabled = desired;
	return false;			// Accepted change
}


int MP3Audio_IsEnabled(void)
{
	// Return -1 for hard NO, return 0 for No, return 1 for enabled

	if (!MP3Com.initialized) // Baker: if it isn't MP3Com.initialized, it can't be enabled
		return -1;

	if (!MP3Com.enabled)
		return 0;

	return 1; // Must be MP3Com.enabled then

}



void MP3Audio_Shutdown (void)
{
	if (!MP3Com.enabled) return;

	KillMP3DShow ();
}



mp3_com_t *MP3_Init (void)
{
// Baker: this isn't necessary.  Dedicated server doesn't init this anyway.
//
//	if (cls.state == ca_dedicated)
//	{
//		MP3Com.enabled = false;
//		return -1;
//	}
	Cvar_Registration_Client_Audio_MP3 ();

	if (COM_CheckParm("-nocdaudio"))  // Don't want
	{
		MP3Com.enabled = false;
		goto init_abort;
	}

	if (!InitMP3DShow ())  // Failure and not initialized
	{
		MP3Com.enabled = false;
		goto init_abort;
	}

	MP3Com.initialized = true;

	MP3Com.looping = false;
	MP3Com.paused = false;

	MP3Com.enabled = true;

// Baker: this should be ok here
	if(!Cvar_GetExternal (&mp3_enabled))
		Cvar_KickOnChange (&mp3_enabled);	// Cvar needs to execute onchange event to take effect

	if (!Cvar_GetExternal (&mp3_volume))
		Cvar_KickOnChange (&mp3_volume);	// Cvar needs to execute onchange event to take effect


init_abort:

	Cmd_AddCommand ("mp3", MP3_f);	// We want this available to the client

	return &MP3Com;
}


