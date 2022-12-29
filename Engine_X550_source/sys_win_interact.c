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
// sys_win_interact.c -- Win32 system interface code

#include "quakedef.h"
#include "winquake.h"
#include <windows.h>

#ifdef _DEBUG

void Sys_OpenQuakeFolder_f(void)
{
	HINSTANCE			ret;
/*	qbool			switch_to_windowed = false;

#ifdef GLQUAKE
	if ((switch_to_windowed = VID_CanSwitchedToWindowed()))
		VID_Windowed();
#endif
*/

	ret = ShellExecute(0, "Open", com_basedir, NULL, NULL, SW_NORMAL);

	if (ret==0)
		Con_Printf("Opening Quake folder failed\n");
	else
		Con_Printf("Quake folder opened in Explorer\n");

}
#endif

void Sys_HomePage_f(void)
{
	HINSTANCE			ret;
/*
	qbool			switch_to_windowed = false;

	if ((switch_to_windowed = VID_CanSwitchedToWindowed()))
		VID_Windowed();
*/
//	char	outstring[CON_TEXTSIZE]="";

//	snprintf(outstring, size(outstring), "%s", ENGINE_HOMEPAGE_URL);

	ret = ShellExecute(0, NULL, ENGINE_HOMEPAGE_URL, NULL, NULL, SW_NORMAL);

	if (ret==0)
		Con_Printf("Opening home page failed\n");
	else
		Con_Printf("%s home page opened in default browser\n", ENGINE_NAME);

}




int Sys_SetPriority(int priority)
{
    DWORD p;

	switch (priority)
	{
		case 0:	p = IDLE_PRIORITY_CLASS; break;
		case 1:	p = NORMAL_PRIORITY_CLASS; break;
		case 2:	p = HIGH_PRIORITY_CLASS; break;
		case 3:	p = REALTIME_PRIORITY_CLASS; break;
		default: return 0;
	}

	return SetPriorityClass(GetCurrentProcess(), p);
}

void Sys_Priority_f (void)
{
	static int current_priority;

	if (Cmd_Argc() == 1)
	{
		Con_Printf ("Usage: %s <priority> : set process priority\n", Cmd_Argv(0));
		Con_Printf ("						0 normal, 1 high, -1 low\n\n");
		Con_Printf ("Current priority is %i", current_priority);
		return;
	}

	// Set Priority

	do
	{

		int requested_priority = atoi(Cmd_Argv(1));
		int api_priority;


		if      (requested_priority >=  1)			api_priority = 2;	// High
		else if (requested_priority <= -1)			api_priority = 0;	// Low
		else										api_priority = 1;	// Normal

		if (!Sys_SetPriority(api_priority))
		{
			Con_Printf("Changing process priority failed\n");
			return; // change declined
		}

		Con_Printf("Process priority set to %s\n", (api_priority == 1) ? "normal" : ((api_priority == 2) ? "high" : "low"));

		current_priority = requested_priority;

	} while (0);
}

// Baker: This really doesn't go here ... not windows specific at all
void Sys_Sleep_f (void)
{
	if (Cmd_Argc() == 1)
	{
		Con_Printf ("Usage: %s <milliseconds> : let system sleep and yield cpu\n", Cmd_Argv(0));
		return;
	}

	Con_Printf ("Sleeping %i milliseconds ...\n", atoi(Cmd_Argv(1)));
	Sleep (atoi(Cmd_Argv(1)));
}

// Simple system compatibility function wrappers

#include <process.h>
#include <direct.h>

int  Sys_getpid (void)				{ return _getpid();	}   // Windows, _getpid; other operating systems "getpid".
void Sys_mkdir	(const char *Path)	{ _mkdir (Path);	};	// Guess MSVC6 isn't ISO C compliant on this
void Sys_Sleep	(void)				{ Sleep (1);		};

void Sys_AppendBinaryExtension (char *path_to_modify)
{
//	if (!strstr(path, ".exe") && !strstr(path, ".EXE"))
//		strlcat (path, ".exe", sizeof(path));
//  Baker: ^^^ case sensitive, probably does the trick 99.9% but let's get it right

	COMD_ForceExtension  (path_to_modify, ".exe");
}

void Sys_CarriageReturnFix (char *buffer_to_modify)
{
}
// Only Macs need to do this ... maybe

void Sys_Crash_f (void)
{
	void (*Crash_Me_Now) (void);

	if (!developer.integer) if (!developer.integer) { Con_Printf("Only available in developer mode\n"); return;}

	Crash_Me_Now = NULL;
	Crash_Me_Now ();	// CRASH!!  To test crash protection.

}

void Sys_Error_f (void)
{
	if (!developer.integer) { Con_Printf("Only available in developer mode\n"); return;}

	Sys_Error ("Intentional fake system error");

}



#ifdef _DEBUG

void Sys_ShowMouseCursor_f (void)
{
	ShowCursor (TRUE);
}

void Sys_HideMouseCursor_f (void)
{
	ShowCursor (FALSE);
}

void Sys_RegionNull_f (void)
{
	ClipCursor (NULL);
}
extern RECT	MouseWindowLockRegion;
void Sys_RegionUpdate_f (void)
{
	ClipCursor (&MouseWindowLockRegion);
}


void Sys_RegionPrint_f (void)
{
	ClipCursor (&MouseWindowLockRegion);
	Con_Printf("%i %i %i %i\n", MouseWindowLockRegion.left, MouseWindowLockRegion.top, MouseWindowLockRegion.right, MouseWindowLockRegion.bottom);
}

//   char buffer[_MAX_PATH];
//
//   /* Get the current working directory: */
//   if( _getcwd( buffer, _MAX_PATH ) == NULL )
//      perror( "_getcwd error" );
//   else
//      printf( "%s\n", buffer );

void Sys_cwd_f (void)
{
	char	mycwd [1024];

	if (!GetCurrentDirectory(sizeof(mycwd), mycwd))
		Sys_Error ("Couldn't determine current directory");

	// Strip trailing slash
//	if (mycwd[strlen(mycwd)-1] == '/')
//		mycwd[strlen(mycwd)-1] = 0;

	COMD_StripTrailing_UnixSlash (mycwd);

	Con_Printf("Current working dir: %s\n", mycwd);
}


#endif
// NEVER USE ID1?
//{
//	{"quake",		OWNER_QUAKE,					""},
//	{"config",		OWNER_GAMEDIR,					"config.cfg"
//	{"log",			OWNER_ENGINE_IF_NOT_DEDICATED,	"qconsole.log"  // Except if dedicated server!
//	{"shots",		OWNER_ENGINE_IF_NOT_DEDICATED,	"shots"
//	{"videos",		OWNER_ENGINE_IF_NOT_DEDICATED,	"videos"
//	{"history",		OWNER_ENGINE_IF_NOT_DEDICATED,	"engine_history.txt"
//	{"iplog",		OWNER_ENGINE_IF_NOT_DEDICATED,	"iplog.dat"
//	{"skyboxes",	OWNER_GAMEDIR,					"gfx/env"
//	{"charsets",	OWNER_GAMEDIR,					"gfx/conchars.tga"
//	{"crosshairs",	OWNER_GAMEDIR,					"gfx/scr_crosshair.tga"
//	{"config"		OWNER_GAMEDIR,
//	{"music"		OWNER_GAMEDIR
//	{"maps"			OWNER_GAMEDIR		// Engine X is a "closed system" ... don't id1 this
//	{"demos"		OWNER_GAMEDIR		// And since the enginex path is searched, it could be confusing if id1 data
										// and enginex data didn't jive


qbool Explorer_OpenFolder_HighlightFile (const char *AbsoluteFileName)
{
	char folder_to_open[MAX_OSPATH];
	char file_highlight[MAX_OSPATH];
	char command_line  [1024];

	if (Sys_FileTime(AbsoluteFileName) == -1)
	{
		Con_DevPrintf (DEV_GAMEDIR, "File \"%s\" does not exist to show\n", AbsoluteFileName);
		Con_Printf ("File does not exist to show\n");
		return false;
	}

	// Copy it
	StringLCopy (file_highlight, AbsoluteFileName);

	// Windows format the slashes
	COMD_SlashesBack_Like_Windows (file_highlight);

	// Get the path
	COM_Copy_GetPath (file_highlight, folder_to_open);

	snprintf (command_line, sizeof(command_line), "/select,%s", file_highlight);

	// Zero is failure, non-zero is success
	Con_DevPrintf (DEV_GAMEDIR, "Folder highlight: explorer.exe with \"%s\"\n", command_line);

	return (ShellExecute(0, "Open", "explorer.exe", command_line, NULL, SW_SHOWDEFAULT) != 0);

}


qbool Explorer_OpenFolder (const char *AbsolutePath, qbool bCreate)
{
	char folder_to_open[MAX_OSPATH];

	if (Sys_FileTime(AbsolutePath) == -1 && bCreate)
	{
		// Add a trailing slash so COM_CreatePath will do it right
		snprintf (folder_to_open, sizeof(folder_to_open), "%s/", AbsolutePath);
		COM_CreatePath (folder_to_open);
	}

	if (Sys_FileTime(AbsolutePath) == -1)
	{
		Con_DevPrintf (DEV_GAMEDIR, "Folder \"%s\" does not exist to show\n", AbsolutePath);
		Con_Printf ("Folder does not exist to show\n");

		return false;
	}

	// Copy it
	StringLCopy (folder_to_open, AbsolutePath);

	// Windows format the slashes
	COMD_SlashesBack_Like_Windows (folder_to_open);

	Con_DevPrintf (DEV_GAMEDIR, "Folder open: explorer.exe with \"%s\"\n", folder_to_open);
	return (ShellExecute(0, "Open", "explorer.exe", folder_to_open, NULL, SW_NORMAL) != 0);
}

int COM_Find_strcasecmp (const char *find_this, const char **search_string_array, int fsize)
{
	int i;
	for (i = 0; i < fsize; i++)
		if (COM_StringMatchCaseless (search_string_array[i], find_this))
			break;

	return i;

}


const char *folders[] = {
	"quake",	// 0
	"config",	// 1
	"demos",	// 2
	"log",		// 3
	"maps",		// 4
	"music",	// 5
	"shots",	// 6
	"videos"	// 7
};

const char *current_places[] = {
	"map",		// 0
	"vis",		// 1
	"lit",		// 2
	"ent",		// 3
	"progs",	// 4
	"skybox",	// 5
};

#include "winquake_video_modes.h"


char *COM_StringBuildDelim (const char **in, int fsize, const char *delim)
{
	static char buffer[1024];
	int	i;
	int	num_elements = sizeof(in)/sizeof(in[0]);
	int sizeof_in = sizeof(in);
	int sizeof_in2 = sizeof(*in);
	int sizeof_in3 = sizeof(in[0]);
//	int sizeof_in4 = sizeof(*in[0]);


	buffer[0] = 0;

	for (i=0; i< fsize; i++)
	{
		if (i)
			StringLCat (buffer, delim);		// Concat delim unless first element
		StringLCat (buffer, in[i]);
	}

	return buffer;
}



int	num_folders = sizeof(folders)/sizeof(folders[0]);

void Sys_OpenFolder_f (void)
{

	int	option_selected;
	qbool ret;

	if (Cmd_Argc() < 2) // Only
	{
		Con_Printf ("Usage: %s {%s}\n", Cmd_Argv (0), COM_StringBuildDelim (folders, num_folders, " | "));
		Con_Printf ("       Note: You must be in windowed mode\n");
		return;
	}

	if (video_modes[vid_default].displaymode != MODE_WINDOWED)
	{
		Con_Printf ("%s: You must be in windowed mode to open folder\n", Cmd_Argv (0));
		if (!vid_isLocked && vid_altenter_toggle.integer)	// what about bpp or other stuffs (vid_fullscreen_only)?
			Con_Printf ("Usually ALT-ENTER allows to switch to windowed mode.\n", Cmd_Argv (0));
		return;
	}

	option_selected = COM_Find_strcasecmp (Cmd_Argv(1), folders, num_folders);

	switch (option_selected)
	{

	// Special
	case 1:		ret = Explorer_OpenFolder_HighlightFile (FULLPATH_TO_CONFIG_CFG);	break;
	case 3:		ret = Explorer_OpenFolder_HighlightFile (FULLPATH_TO_QCONSOLE_LOG);	break;

	// Baker: Should we COM_CreatePath these if they don't exist?  Yes.  Add flag to Explorer_OpenFolder
	case 0:		ret = Explorer_OpenFolder (FOLDER_QUAKE,  1);	break;
	case 2:		ret = Explorer_OpenFolder (FOLDER_DEMOS,  1);	break;
	case 4:		ret = Explorer_OpenFolder (FOLDER_MAPS,   1);	break;
	case 5:		ret = Explorer_OpenFolder (FOLDER_MUSIC,  1);	break;
	case 6:		ret = Explorer_OpenFolder (FOLDER_SHOTS,  1);	break;
	case 7:		ret = Explorer_OpenFolder (FOLDER_VIDEOS, 1);	break;

	default:	// No match
				Con_Printf ("Not a recognized location\n");
				return;

	}

	if (!ret)
		Con_Warning ("Opening folder failed\n");
	else
		Con_Printf  ("Folder opened in Explorer\n");

	return;
}

void Sys_DemoAssociate_f (void)
{
	Con_Printf ("Associates demo (.dem and .dz) files with engine for automatic demoplay in Windows Explorer\n\n");

	BuildRegistryEntries ("Quake Demos"           , ".dem", ENGINE_NAME, COM_Argv(0)); // Argv0 is the executable name and path
	BuildRegistryEntries ("Quake Demos"           , ".dz" , ENGINE_NAME, COM_Argv(0)); // Argv0 is the executable name and path

//	BuildRegistryEntries ("Pak Files"             , ".pak", "Pakscape",  FULLPATH_TO_PAKSCAPE); // Argv0 is the executable name and path
//	BuildRegistryEntries ("Sprite Filez"          , ".spr", "fimgz",      FULLPATH_TO_FIMG_EXE); // Argv0 is the executable name and path

}

int File_To_ReadOnly (const char *myFile)
{
	DWORD FileAttributes;
	// engine_x_external.cfg exists in the enginedir folder
	if (Sys_FileTime(myFile) == -1)
	{
		Con_DevPrintf (DEV_GAMEDIR, "Not setting file attributes for '%s' because file does not exist\n", myFile);
		return -1;	// This is a failure of sorts.  Because nothing prevents future write of file.  Maybe we should create it?
	}

	if ((FileAttributes = GetFileAttributes(myFile)) == -1)
	{
		Con_DevPrintf (DEV_GAMEDIR, "Somehow GetFileAttributes for seemingly existing file '%s' failed\n", myFile);
		return 0;
	}

	// Determine if read-only already exists

	if (FileAttributes & FILE_ATTRIBUTE_READONLY)
	{
		Con_DevPrintf (DEV_GAMEDIR, "File '%s' is already read-only\n", myFile);
		return 2;	// This is a success condition.
	}

	FileAttributes |= FILE_ATTRIBUTE_READONLY;	// Add Read-Only flag

	if (SetFileAttributes(myFile, FileAttributes) == -1)
	{
		Con_Printf ("Unable to set file attributes for '%s'\n", myFile); // Weird enough to merit a Con_Printf
		return 0;
	}

	return 1;

}

// Lock config ... SetFileAttributes for config.cfg and enginex_external.cfg to read-only
void Sys_Lock_Config_f (void)
{
	int result1;
	int result2;

	result1 = Sys_FileTime (FULLPATH_TO_CONFIG_CFG);
	result2 = Sys_FileTime (FULLPATH_TO_EXTERNALCFG);

	if (result1 == -1 || result2 == -1)
	{
		if (result1 == -1 && result2 == -1)
			Con_Printf ("Failure: Action aborted because config.cfg and %s do not exist.  Try \"writeconfig\" first.\n", EXTERNALS_CONFIG_BARE);
		else if (result2 == -1)
			Con_Printf ("Failure: Action aborted because %s does not exist.  Try \"writeconfig\" first.\n", EXTERNALS_CONFIG_BARE);
		else // result1 == -1
			Con_Printf ("Failure: Action aborted because config.cfg does not exist.  Try \"writeconfig\" first.\n");
		return;
	}

	// config.cfg exists in the gamedir
	result1 = File_To_ReadOnly(FULLPATH_TO_CONFIG_CFG);

	if (result1 <= 0)  // Don't continue if first failed
	{
		if (result1 == -1)
			Con_Printf ("Failure: Action aborted because config.cfg does not exist.  Try \"writeconfig\" first.\n");
		else // result1 has to be 0
			Con_Printf ("Failure: Unable to set read-only status for config.cfg.\n");

		return;
	}

	result2 = File_To_ReadOnly(FULLPATH_TO_EXTERNALCFG);

	if (result2 <= 0)  // Don't continue if first failed
	{
		if (result2 == -1)
			Con_Printf ("Failure: Action aborted because %s does not exist.  Try \"writeconfig\" first.\n", EXTERNALS_CONFIG_BARE);
		else // result1 has to be 0
			Con_Printf ("Failure: Unable to set read-only status for %s.\n", EXTERNALS_CONFIG_BARE);

		return;
	}

	if (result1 == 2 && result2 == 2)
		Con_Printf ("Configs already locked!\n");
	else
		Con_Printf ("Configs successfully locked\n");

}


int File_Remove_ReadOnly (const char *myFile)
{
	DWORD FileAttributes;

	// engine_x_external.cfg exists in the enginedir folder
	if (Sys_FileTime(myFile) == -1)
	{
		Con_DevPrintf (DEV_GAMEDIR, "Not setting file attributes for '%s' because file does not exist\n", myFile);
		return 1;	// This is a success.  Because the file doesn't exist, it can be written.
	}

	if ((FileAttributes = GetFileAttributes(myFile)) == -1)
	{
		Con_Printf ("Somehow GetFileAttributes for seemingly existing file '%s' failed\n", myFile);
		return 0;
	}

	// Determine if read-only already exists

	if (!(FileAttributes & FILE_ATTRIBUTE_READONLY))
	{
		Con_DevPrintf (DEV_GAMEDIR, "File '%s' is already writable\n", myFile);
		return 2;	// This is a success condition.
	}

	FileAttributes &= ~FILE_ATTRIBUTE_READONLY;	// Remove read-only flag

	if (SetFileAttributes(myFile, FileAttributes) == -1)
	{
		Con_Printf ("Unable to set file attributes for '%s'\n", myFile); // Weird enough to merit a Con_Printf
		return 0;
	}

	return 1;

}





// Lock config ... SetFileAttributes for config.cfg and enginex_external.cfg to read-only
void Sys_Unlock_Config_f (void)
{
	int result1;
	int result2;

	// config.cfg exists in the gamedir
	result1 = File_Remove_ReadOnly(FULLPATH_TO_CONFIG_CFG);

	if (result1 == 0)  // Don't continue if first failed
	{
		Con_Printf ("Failure: Unable to set read-only status for config.cfg.\n");
		return;
	}

	result2 = File_Remove_ReadOnly(FULLPATH_TO_EXTERNALCFG);

	if (result2 == 0)  // Don't continue if first failed
	{
		Con_Printf ("Failure: Unable to set read-only status for %s.\n", EXTERNALS_CONFIG_BARE);
		return;
	}

	if (result1 == 2 && result2 == 2)
		Con_Printf ("Configs already unlocked!\n");
	else
		Con_Printf ("Configs successfully unlocked\n");

}

void Sys_InfoInit(void)
{
	// Dedicated server never goes here
	Cmd_AddCommand ("homepage",   Sys_HomePage_f);

	Cmd_AddCommand ("folder", Sys_OpenFolder_f);	// config | shots | quake | demos | videos | maps

	Cmd_AddCommand ("sysinfo", Sys_InfoPrint_f);
	Cmd_AddCommand ("sleep", Sys_Sleep_f);
	Cmd_AddCommand ("crash", Sys_Crash_f);
	Cmd_AddCommand ("sysfakeerror", Sys_Error_f);
#ifdef _DEBUG
	Cmd_AddCommand ("openquakefolder", Sys_OpenQuakeFolder_f);
	Cmd_AddCommand ("showcursor",   Sys_ShowMouseCursor_f);
	Cmd_AddCommand ("hidecursor",   Sys_HideMouseCursor_f);
	Cmd_AddCommand ("regionnull",   Sys_RegionNull_f);
	Cmd_AddCommand ("regionupdate", Sys_RegionUpdate_f);
	Cmd_AddCommand ("regionprint",  Sys_RegionPrint_f);

	Cmd_AddCommand ("cwd",  Sys_cwd_f);

#endif

	Cmd_AddCommand ("lock_config",   Sys_Lock_Config_f);
	Cmd_AddCommand ("unlock_config", Sys_Unlock_Config_f);
	Cmd_AddCommand ("sys_priority", Sys_Priority_f);


//	Cvar_RegisterVariable (&sys_disablewinkeys, NULL); //, OnChange_sys_disableWinKeys);

//	Cvar_Registration_Client_System ();

	Cmd_AddCommand ("assocdemos",  Sys_DemoAssociate_f);
}
