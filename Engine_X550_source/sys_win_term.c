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
// sys_win_term.c -- Win32 system console terminal interactions


#include "quakedef.h"
#include "conproc.h"

qbool	sc_return_on_enter = false;
HANDLE	hinput, houtput;

HANDLE	hFile;
HANDLE	heventParent;
HANDLE	heventChild;


char *Sys_ConsoleInput (void)
{
	static char	text[256];
	static int	len;
	INPUT_RECORD	recs[1024];
    int		        ch;
	DWORD numread, numevents, dummy;

	if (!isDedicated)
		return NULL;

	for ( ; ; )
	{
		if (!GetNumberOfConsoleInputEvents(hinput, &numevents))
			Sys_Error ("Error getting # of console events");

		if (numevents <= 0)
			break;

		if (!ReadConsoleInput (hinput, recs, 1, &numread))
			Sys_Error ("Error reading console input");

		if (numread != 1)
			Sys_Error ("Couldn't read console input");

		if (recs[0].EventType == KEY_EVENT)
		{
			if (!recs[0].Event.KeyEvent.bKeyDown)
			{	
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;

				switch (ch)
				{
					case '\r':
						WriteFile(houtput, "\r\n", 2, &dummy, NULL);

						if (len)
						{
							text[len] = 0;
							len = 0;
							return text;
						}
						else if (sc_return_on_enter)
						{
						// special case to allow exiting from the error handler on Enter
							text[0] = '\r';
							len = 0;
							return text;
						}

						break;

					case '\b':
						WriteFile (houtput, "\b \b", 3, &dummy, NULL);
						if (len)
							len--;
						break;

					default:
						if (ch >= (int) (unsigned char) ' ')
						{
							WriteFile( houtput, &ch, 1, &dummy, NULL);
							text[len] = ch;
							len = (len + 1) & 0xff;
						}
						break;
				}
			}
		}
	}

	return NULL;
}


void AllocateConsoleTerminal (void)
{
	int t;

	if (!AllocConsole())
		Sys_Error ("Couldn't create dedicated server console");

	hinput = GetStdHandle (STD_INPUT_HANDLE);
	houtput = GetStdHandle (STD_OUTPUT_HANDLE);

// give QHOST a chance to hook into the console
	if ((t = COM_CheckParm("-HFILE")) > 0)
	{
		if (t < com_argc)
			hFile = (HANDLE)atoi (com_argv[t+1]);
	}

	if ((t = COM_CheckParm("-HPARENT")) > 0)
	{
		if (t < com_argc)
			heventParent = (HANDLE)atoi (com_argv[t+1]);
	}

	if ((t = COM_CheckParm("-HCHILD")) > 0)
	{
		if (t < com_argc)
			heventChild = (HANDLE)atoi (com_argv[t+1]);
	}

	InitConProc (hFile, heventParent, heventChild);
}