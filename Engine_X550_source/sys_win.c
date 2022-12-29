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
// sys_win.c -- Win32 system interface code

#include "quakedef.h"
#include "winquake.h"
#include "resource.h"
#include "conproc.h"
#include <limits.h>
#include <errno.h>

#include <direct.h>			// _mkdir
//#include <conio.h>		// _putch

// JPG 3.30 - need these for synchronization
#include <fcntl.h>


#define CONSOLE_ERROR_TIMEOUT	60.0	// # of seconds to wait on Sys_Error running
										// dedicated before exiting


qbool		isDedicated;

void Sys_InitDoubleTime (void);

/*
===============================================================================
SYSTEM IO
===============================================================================
*/


extern HANDLE houtput;
void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[1024];
	char		text2[1024];
	char		*text3 = "Press Enter to exit\n";
	char		*text4 = "***********************************\n";
	char		*text5 = "\n";
	DWORD		dummy;
	double		starttime;
	static int	in_sys_error0 = 0;
	static int	in_sys_error1 = 0;
	static int	in_sys_error2 = 0;
	static int	in_sys_error3 = 0;

	if (!in_sys_error3)
	{
		in_sys_error3 = 1;
	}

	va_start (argptr, error);
	vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	if (isDedicated)
	{
		extern qbool sc_return_on_enter;

		va_start (argptr, error);
		vsnprintf (text, sizeof(text), error, argptr);
		va_end (argptr);

		snprintf (text2, sizeof(text2), "ERROR: %s\n", text);
		WriteFile (houtput, text5, strlen(text5), &dummy, NULL);
		WriteFile (houtput, text4, strlen(text4), &dummy, NULL);
		WriteFile (houtput, text2, strlen(text2), &dummy, NULL);
		WriteFile (houtput, text3, strlen(text3), &dummy, NULL);
		WriteFile (houtput, text4, strlen(text4), &dummy, NULL);

		starttime = Sys_DoubleTime ();
		sc_return_on_enter = true;	// so Enter will get us out of here

		while (!Sys_ConsoleInput () && ((Sys_DoubleTime () - starttime) < CONSOLE_ERROR_TIMEOUT))
		{
		}
	}
	else
	{
	// switch to windowed so the message box is visible, unless we already
	// tried that and failed
		if (!in_sys_error0)
		{
			in_sys_error0 = 1;
#if RENDERER_DIRECT3D_AVAILABLE
	// Baker: To avoid painful Window with focus behind Direct 3D Window problem
			if (!isDedicated && engine.Renderer && engine.Renderer->graphics_api == RENDERER_DIRECT3D)
				VID_Shutdown ();
#endif
			VID_SetDefaultMode ();

			MessageBox (NULL, text, TEXT("Quake Error"), MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
		}
		else
		{
			MessageBox (NULL, text, TEXT("Double Quake Error"), MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
		}
	}

	if (!in_sys_error1)
	{
		in_sys_error1 = 1;
		Host_Shutdown ();
	}

	// shut down QHOST hooks if necessary
	if (!in_sys_error2)
	{
		in_sys_error2 = 1;
		DeinitConProc ();
	}

	exit (1);
}

void Sys_Printf (char *fmt, ...)
{
	va_list	argptr;
	char	text[2048]; 	// JPG - changed this from 1024 to 2048
	DWORD	dummy;

	if (!isDedicated)
		return;

	va_start (argptr, fmt);
	vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

#if 1	// Baker:	I'm kinda tempted to move this below the rcon because if we dequake, we dequake the rcon too.
		//          Which seems odd to be affected by a cvar related to console logging.
		//			Still, the idea of sending stupid characters over rcon ... well, why would you do that?

	// JPG 1.05 - translate to plain text
	if (scr_con_chatlog_dequake.integer)
		COMD_DeQuake (text);

	WriteFile (houtput, text, strlen(text), &dummy, NULL);

#endif
		// JPG 3.00 - rcon (64 doesn't mean anything special, but we need some extra space because NET_MAXMESSAGE == RCON_BUFF_SIZE)
	if (rcon_active  && (rcon_message.cursize < rcon_message.maxsize - strlen(text) - 64))
	{
		rcon_message.cursize--;
		MSG_WriteString (&rcon_message, text);
	}

}








