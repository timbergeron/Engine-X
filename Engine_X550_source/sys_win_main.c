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
// sys_win_main.c -- Win32 main program entry point


#include "quakedef.h"
#include "conproc.h"
#include "winquake.h"



/*
==============================================================================
 WINDOWS CRAP
==============================================================================
*/

static HANDLE	tevent;


void SleepUntilInput (int time)
{

	MsgWaitForMultipleObjects (1, &tevent, FALSE, time, QS_ALLINPUT);
}


/*
==================
WinMain
==================
*/
HINSTANCE	global_hInstance;
//int		global_nCmdShow;
char	*argv[MAX_NUM_ARGVS];
static	char	*empty_string = "";
//HWND	hwnd_dialog;

LONG WINAPI EngineCrashHandler (LPEXCEPTION_POINTERS toast)
{
	static qbool recursive_entry_protection=false;

	if (!recursive_entry_protection)
	{
#if RENDERER_DIRECT3D_AVAILABLE
	// Baker: To avoid painful Window with focus behind Direct 3D Window problem
	if (!isDedicated && engine.Renderer && engine.Renderer->graphics_api == RENDERER_DIRECT3D)
		VID_Shutdown ();
#endif
		MessageBox (NULL, ENGINE_NAME " has crashed.\n" "Please visit " ENGINE_HOMEPAGE_URL " and report this crash.", "An error has occurred", MB_OK | MB_ICONSTOP);

		recursive_entry_protection=true;
	}

	// restore default timer
	//   timeEndPeriod (sys_time_period);

   // down she goes
   return EXCEPTION_EXECUTE_HANDLER;
}


//extern HANDLE	heventParent;
//extern HANDLE	heventChild;


qbool	ActiveApp; /*, Minimized*/; // Minimized now owned by host

//
// Application Entry Point
//
//double startup_time;
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	quakeparms_t	parms;
	double			time, oldtime, newtime;
	MEMORYSTATUS	lpBuffer;
	static	char	cwd [1024];
	char			exeline[1024];

	global_hInstance = hInstance;

	if (!Sys_GetWindowsVersion())
		Sys_Error ("Quake requires at least Win95 or NT 4.0");


	// Obtain memory information
	lpBuffer.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus (&lpBuffer);

	{ // Get current working directory.
		if (!GetCurrentDirectory(sizeof(cwd), cwd))
			Sys_Error ("Couldn't determine current directory");

		// Strip trailing slash
//		if (cwd[strlen(cwd)-1] == '/')
//			cwd[strlen(cwd)-1] = 0;
		COMD_StripTrailing_UnixSlash (cwd);

	}

	{	// Get executable directory
		char	exedir[1024];
//		char	*e;
		int		i;

		// Get the exe name
		if(!(i = GetModuleFileName(NULL, exedir, sizeof(exedir)-1)))
			Sys_Error("FS_InitFilesystemEx: GetModuleFileName failed");

		// exedir in: Will be something like "c:\quake\glquake.exe" at this point
		exedir[i] = 0; // ensure null terminator.
		StringLCopy (exeline, exedir);

//		for (e = exedir+strlen(exedir)-1; e >= exedir; e--)
//		{
//			if (*e == '/' || *e == '\\')
//			{
//				*e = 0;
//				break;
//			}
//		}

		COM_Copy_GetPath (exedir, exedir); // Terminates after the path

		// exedir out: Will be something like "c:\quake" at this point


		// Baker 3.76 - playing demos via file association; working directory might be some temp file dir if
		// started via file association

		{	// Override working dir if
			char			fpaktest[1024];
			FILE			*fpak0;

			snprintf (fpaktest, sizeof(fpaktest), "%s/id1/pak0.pak", cwd); // Baker 3.76 - Sure this isn't gfx.wad, but let's be realistic here

			if (!(fpak0 = FS_fopen_read(fpaktest, "rb")))
			{
				// Failed to find pak0.pak, use the dir the exe is in
				StringLCopy (cwd, exedir);
			}
			else
			{
				fclose( fpak0 ); // Pak0 found so close it; we have a valid directory
			}

			// Now ... Baker we are going to do this anyway.  This isn't actually illogical.
			//         Although at first glance, it would seem so.

			StringLCopy (cwd, exedir);
#pragma message ("Baker: consider removing this?")
		}	// out:  if our cwd is useless because no gamedata, use exedir.
	}


	parms.basedir = cwd;
	parms.argc = 1;

	{	// Prepare argv[0] ... before ProQuake, it was NULL.  Now is exe full path.
		char			*ch;	// JPG 3.00 - for eliminating quotes from exe name

		argv[0] = GetCommandLine();	// JPG 3.00 - was empty_string

		// Ensure null termination
		lpCmdLine[-1] = 0;			// JPG 3.00 - isolate the exe name, eliminate quotes

		// Move past any double quotes "
		if (argv[0][0] == '\"')
			argv[0]++;
		if (argv[0][0] == '\"')
			argv[0]++;

		// Find trailing double quote remove it
		if (ch = strchr(argv[0], '\"'))
			*ch = 0;

	}

	// Slice and dice the command line into pieces
	while (*lpCmdLine && (parms.argc < MAX_NUM_ARGVS))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			if (*lpCmdLine == '\"')
			{
				lpCmdLine++;

				argv[parms.argc] = lpCmdLine;
				parms.argc++;

				while (*lpCmdLine && *lpCmdLine != '\"') // this include chars less that 32 and greate than 126... is that evil?
					lpCmdLine++;
			}
			else
			{
			argv[parms.argc] = lpCmdLine;
			parms.argc++;

			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;
			}

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}

		}
	}

	parms.argv = argv;

	// Rebuilds the command line back into what gets used for the cmdline cvar.
	COM_InitArgv (parms.argc, parms.argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	isDedicated = (COM_CheckParm ("-dedicated"));

#ifndef _DEBUG
	// Baker: To catch crashes
//	InitCommonControls ();
	if (!isDedicated)
		SetUnhandledExceptionFilter (EngineCrashHandler);
#endif


	{	// Figure out how much memory to take
		// Baker: on a modern machine, it is going to use MAXIMUM_WIN_MEMORY
#define MINIMUM_WIN_MEMORY	0x2000000	// Baker:  32 MB
#define MAXIMUM_WIN_MEMORY	0x8000000	// Baker: 128 MB

		int t;

		// Original Quake:  take the greater of all the available memory or half the total memory,
		// but at least 8 Mb and no more than 16 Mb, unless they explicitly request otherwise
		parms.memsize = lpBuffer.dwAvailPhys;

		if (parms.memsize < MINIMUM_WIN_MEMORY)
			parms.memsize = MINIMUM_WIN_MEMORY;

		if (parms.memsize < (lpBuffer.dwTotalPhys >> 1))
			parms.memsize = lpBuffer.dwTotalPhys >> 1;

		if (parms.memsize > MAXIMUM_WIN_MEMORY)
			parms.memsize = MAXIMUM_WIN_MEMORY;

		// heapsize param:  specified in bytes
		if ((t = COM_CheckParm("-heapsize")) != 0 && t + 1 < com_argc)
			parms.memsize = atoi (com_argv[t+1]) * 1024;

		// mem param:		specified in MBs
		if ((t = COM_CheckParm("-mem")) != 0 && t + 1 < com_argc)
			parms.memsize = atoi (com_argv[t+1]) * 1024 * 1024;

		if (parms.memsize < MINIMUM_WIN_MEMORY)
			Sys_Error ("Only %4.1f megs of memory available, can't execute game", parms.memsize / (float)0x100000);

		// Baker: I do not want this occuring here ...
		// parms.membase = Q_malloc (parms.memsize, "Hunky!");	// Q_malloc will Sys_Error us if malloc fails
	}	// out:  parms.memsize is now determined


	if (!(tevent = CreateEvent(NULL, FALSE, FALSE, NULL)))
		Sys_Error ("Couldn't create event");

#if defined(_WIN32) && defined(GLQUAKE) // Baker: opengl32.dll check
	{
		char file_check[MAX_OSPATH];
		FILE *fp;

		snprintf (file_check, sizeof(file_check), "%s/%s", parms.basedir, "opengl32.dll");
		if ((fp = FS_fopen_read(file_check,"r")))
		{
			fclose(fp);
			// File found.  Open an offer opportunity to remove file.

			MessageBox (NULL, "This file must be deleted, renamed or moved from your Quake folder.\n\nLikely this is the 1990s driver that came with Quake intended for an early video card.\n\nIt causes GLQuake to fail to load on a modern system.\n\nRestart after removing, renaming or moving this file." , "OpenGL32.dll found in Quake folder", MB_OK);
			Explorer_OpenFolder_HighlightFile (file_check);
			Sys_Quit ();

		}
	}

#endif // Windows only


	if (isDedicated)
		AllocateConsoleTerminal ();


	// Initialize timer
	Sys_InitDoubleTime ();

	session_startup_time = oldtime = Sys_DoubleTime ();

// because sound is off until we become active
	S_BlockSound ();

	Sys_Printf ("Host_Init\n");

	Host_Init (&parms);

	/* main window message loop */
	while (1)
	{
		if (isDedicated)
		{
			newtime = Sys_DoubleTime ();
			time = newtime - oldtime;

			while (time < sys_ticrate.floater)
			{
				Sys_Sleep ();
				newtime = Sys_DoubleTime ();
				time = newtime - oldtime;
			}
		}
		else
		{

#define PAUSE_SLEEP			50			// sleep time on pause or minimization
#define NOT_FOCUS_SLEEP		20			// sleep time when not focus

		// yield the CPU for a little while when paused, minimized, or not the focus
			if ((cl.paused && !ActiveApp) || Minimized)
			{
				SleepUntilInput (PAUSE_SLEEP);
				scr_skipupdate = 1;		// no point in bothering to draw
			}
			else if (!ActiveApp)
			{
				SleepUntilInput (NOT_FOCUS_SLEEP);
			}

			newtime = Sys_DoubleTime ();
			time = newtime - oldtime;
		}

		Host_Frame (time);
		oldtime = newtime;

	}

	return TRUE; // return success of application
}


void Sys_Quit (void)
{

	Host_Shutdown ();

	if (tevent)
		CloseHandle (tevent);

	if (isDedicated)
		FreeConsole ();

//	Sys_WinKeyHook_Shutdown ();

// shut down QHOST hooks if necessary
	DeinitConProc ();

	exit (0);
}
