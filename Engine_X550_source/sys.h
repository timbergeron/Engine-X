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
// sys.h -- non-portable functions

#ifndef __SYS_H__
#define __SYS_H__


// sys_win.c - base system functions
void Sys_Error (const char *error, ...);	// an error will cause the entire program to exit
void Sys_Printf (char *fmt, ...);			// send text to the console

// sys_win_main.c - main program loop
void Sys_Quit (void);

// OS neutralizing simple functions  ...  move to own file?
void Sys_mkdir					(const char *Path);
int  Sys_getpid					(void);
void Sys_AppendBinaryExtension	(char *path_string_to_modify);	// On non-Windows, this will be an empty function obviously
void Sys_Sleep					(void);							// called to yield for a little bit so as not to hog cpu when paused or debugging
void Sys_CarriageReturnFix		(char *buffer_to_modify);		// Mac OS X may have trouble reading carriage returns.


// sys_win_filesystem.c - file IO
int Sys_FileTime (const char *Path);		// return -1 if file is not present





// sys_win_clipboard.c
char *Sys_GetClipboardData (void);
void Sys_CopyToClipboard(char *);

// sys_win_clock.c - clock
void Sys_InitDoubleTime (void);
double Sys_DoubleTime (void);

// sys_win_messages.c - system messages (keyboard, window system, etc.)
void Sys_SendKeyEvents (void);	// Perform Key_Event () callbacks until the input que is empty

// sys_win_term.c - terminal
char *Sys_ConsoleInput (void);


// sys_win_interact.c - Exploiting operating system features on top of what is required
void Sys_InfoInit(void);
void Sys_WinKeyHook_Shutdown (void);

// sys_win_registry.c
#if SUPPORTS_DEMO_AUTOPLAY
void BuildRegistryEntries (const char *media_description, const char *media_file_extension, const char *application_description, const char *executable_fullpath);
#endif

#ifdef SUPPORTS_OPEN_FOLDER

qbool Explorer_OpenFolder_HighlightFile (const char *AbsoluteFileName);
#endif

// sys_win_version.c
qbool Sys_GetWindowsVersion(void);

// Commands
void Sys_InfoPrint_f(void) ;

#endif // __SYS_H__
