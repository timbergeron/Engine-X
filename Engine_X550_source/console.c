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
// console.c
// Baker: Validated 6-27-2011.  Boring functionary changes.


//#ifndef _MSC_VER
//#include <unistd.h>
//#endif

//#include <fcntl.h>
//#include <errno.h>
#include "quakedef.h"

//#ifdef _WIN32 // Headers
//#include <io.h>
//#include <windows.h>
//#endif

#include <time.h> // JPG - needed for console log



float		con_cursorspeed = 6;	// joe: increased to make it blink faster

// JPG - upped CONSOLE_MAX_TEXTSIZE from 16384 to 65536
#define		CONSOLE_MAX_TEXTSIZE	65536

qbool 	con_forcedup;		// because no entities to refresh

typedef struct
{
	int 		columns_available;
	int			rows_available;				// total lines in console scrollback
	int			text_rows_populated;		// number of non-blank text lines, used for backscrolling, added by joe

	int			message_cursor_row;			// where next message will be printed
	int			message_cursor_column;		// offset in current line for next print

	int			maximum_backscroll;
	char		*console_text; // = 0;

} qconsole_t;

int		rows_into_backscroll;		// lines up from bottom to display

FILE		*qconsole_log;

qbool	cl_inconsole = false;//R00k
qbool	con_initialized = false;


qconsole_t qconsole;



char		con_lastcenterstring[1024]; //johnfitz

#define	CONSOLE_MAX_NOTIFY_LINES 16
float		notify_messages_realtimes[CONSOLE_MAX_NOTIFY_LINES];	// realtime time the line was generated
						// for transparent notify lines

int		console_visible_pixel_lines;
int		con_notifylines;		// scan lines to clear for notify lines


extern	char	key_lines[32][MAXCMDLINE];
extern	int	edit_line;
extern	int	key_linepos;
extern	int	key_insert;





/*
================
Con_Quakebar -- johnfitz -- returns a bar of the desired length, but never wider than the console

includes a newline, unless len >= columns_available.
================
*/
char *Con_Quakebar (int len)
{
	static char bar[42];
	int i;

	len = min(len, sizeof(bar) - 2);
	len = min(len, qconsole.columns_available);

	bar[0] = '\35';
	for (i = 1; i < len - 1; i++)
		bar[i] = '\36';
	bar[len-1] = '\37';

	if (len < qconsole.columns_available)
	{
		bar[len] = '\n';
		bar[len+1] = 0;
	}
	else
		bar[len] = 0;

	return bar;
}

/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f (void)
{
	key_lines[edit_line][1] = 0;	// clear any typing
	key_linepos = 1;

	if (key_dest == key_console)
	{
		if (cls.state == ca_connected)
		{
			key_dest = key_game;
			cl_inconsole = false;//R00k
		}
		else
		{
			M_Menu_Main_f ();
			cl_inconsole = true;//R00k
		}
	}
	else
	{
		key_dest = key_console;
		cl_inconsole = true;//R00k
		// Baker: keep this commented out
		//rows_into_backscroll = 0; // JPG - don't want to enter console with backscroll
	}

	Con_DevPrintf (DEV_PROTOCOL, "Toggleconsole is Ending loading plaque\n");
	SCR_EndLoadingPlaque ();
	memset (notify_messages_realtimes, 0, sizeof(notify_messages_realtimes));
}

/*
================
Con_Clear_f
================
*/
void Con_Clear_f (void)
{
	if (qconsole.console_text)
		memset (qconsole.console_text, ' ', CONSOLE_MAX_TEXTSIZE);
}

/*
====================
Con_Showtime_f // Baker 3.60 - "time" command to display current date and time in console

time
====================
*/
void Con_Showtime_f (void)
{
	time_t	ltime;
	char	str[80];

	time (&ltime);
	strftime (str, sizeof(str)-1, "%B %d, %Y - %I:%M:%S %p", localtime(&ltime));

	Con_Printf ("%s \n", str);


//	Sys_CopyToClipboard(va("Operating System: %s\r\nThis Thing: %\r\n", session_cmdline.string));


}


/*
================
Con_Dump_f -- johnfitz -- adapted from quake2 source
================
*/
void Con_Dump_f (void)  // FWRITERESTRICT
{
	int		l, x;
	char	*line;
	FILE	*f;
	char	buffer[1024];
	char	name[MAX_OSPATH];

#if 1
	//johnfitz -- there is a security risk in writing files with an arbitrary filename. so,
	//until stuffcmd is crippled to alleviate this risk, just force the default filename.
	snprintf (name, sizeof(name), "%s/condump.txt", com_gamedirfull);
#else
	if (Cmd_Argc() > 2)
	{
		Con_Printf ("Usage: %s <filename>\n", Cmd_Argv (0));
		return;
	}

	if (Cmd_Argc() > 1)
	{
		if (strstr(Cmd_Argv(1), ".."))
		{
			Con_Printf ("Relative pathnames are not allowed.\n");
			return;
		}
		snprintf (name, sizeof(name), "%s/%s", com_gamedir, Cmd_Argv(1));
		COMD_DefaultExtension (name, ".txt");
	}
	else
		snprintf (name, sizeof(name), "%s/condump.txt", com_gamedir);
#endif

	f = FS_fopen_write (name, "w", FS_CREATE_PATH);
	if (!f)
	{
		Con_Printf ("ERROR: couldn't open file.\n", name);
		return;
	}

	// skip initial empty lines
	for (l = qconsole.message_cursor_row - qconsole.rows_available + 1 ; l <= qconsole.message_cursor_row ; l++)
	{
		line = qconsole.console_text + (l%qconsole.rows_available)*qconsole.columns_available;
		for (x=0 ; x<qconsole.columns_available ; x++)
			if (line[x] != ' ')
				break;
		if (x != qconsole.columns_available)
			break;
	}

	// write the remaining lines
	buffer[qconsole.columns_available] = 0;
	for ( ; l <= qconsole.message_cursor_row ; l++)
	{
		line = qconsole.console_text + (l%qconsole.rows_available)*qconsole.columns_available;
		strncpy (buffer, line, qconsole.columns_available);
		for (x=qconsole.columns_available-1 ; x>=0 ; x--)
		{
			if (buffer[x] == ' ')
				buffer[x] = 0;
			else
				break;
		}
		for (x=0; buffer[x]; x++)
			buffer[x] &= 0x7f;

		fprintf (f, "%s\n", buffer);
	}

	fclose (f);
	Con_Printf ("Dumped console text to %s.\n", name);
}

#if SUPPORTS_CLIPBOARD_COPY
/*
================
Con_Copy_f -- Baker -- adapted from Con_Dump
================
*/
void Con_Copy_f (void)
{
	char	outstring[CONSOLE_MAX_TEXTSIZE]="";
	int		l, x;
	char	*line;
	char	buffer[1024];

	if (Cmd_Argc() > 1) // More than just the command
	{
		if (COM_StringMatchCaseless(Cmd_Argv(1), "ents"))  // If argument "ents" passed and we have an entitystring
		{
			extern char *entitystring;
			if (!sv.active)		{ Con_Printf ("copy ents: Not running a server"); return; }
			if (!entitystring)  { Con_Printf ("copy ents: No entities to copy");   return; }  // How would this happen, btw?

			Sys_CopyToClipboard (entitystring);

			Con_Printf ("Entities copied to the clipboard (%i bytes)\n", strlen(entitystring));
			return;
		}
		else // Invalid args
		{
			Con_Printf ("Usage: %s [ents]\n", Cmd_Argv (0));
			return;
		}

	}

	// skip initial empty lines
	for (l = qconsole.message_cursor_row - qconsole.rows_available + 1 ; l <= qconsole.message_cursor_row ; l++)
	{
		line = qconsole.console_text + (l%qconsole.rows_available)*qconsole.columns_available;
		for (x=0 ; x<qconsole.columns_available ; x++)
			if (line[x] != ' ')
				break;
		if (x != qconsole.columns_available)
			break;
	}

	// write the remaining lines
	buffer[qconsole.columns_available] = 0;
	for ( ; l <= qconsole.message_cursor_row ; l++)
	{
		line = qconsole.console_text + (l%qconsole.rows_available)*qconsole.columns_available;
		strncpy (buffer, line, qconsole.columns_available);
		for (x=qconsole.columns_available-1 ; x>=0 ; x--)
		{
			if (buffer[x] == ' ')
				buffer[x] = 0;
			else
				break;
		}
		for (x=0; buffer[x]; x++)
			buffer[x] &= 0x7f;

		StringLCat (outstring, va("%s\r\n", buffer)); // Gross ... we are adding carriage returns .... // strlcat (outstring, va("%s\r\n", buffer), sizeof(outstring));

	}

	Sys_CopyToClipboard(outstring);
	Con_Printf ("Copied console to clipboard\n");
}
#endif

/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify (void)
{
	int		i;

	for (i=0 ; i<CONSOLE_MAX_NOTIFY_LINES ; i++)
		notify_messages_realtimes[i] = 0;
}


/*
================
Con_MessageMode_f
================
*/
extern qbool team_message;

void Con_MessageMode_f (void)
{
	key_dest = key_message;
	team_message = false;
}


/*
================
Con_MessageMode2_f
================
*/
void Con_MessageMode2_f (void)
{
	key_dest = key_message;
	team_message = true;
}


/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize (void)
{
	int	i, j, width, oldwidth, oldtotallines, numlines, numchars;
	char	tbuf[CONSOLE_MAX_TEXTSIZE];

	width = (vid.width >> 3 /*divide by 8, the width of a character */) - 2 /*first and last columns are not used*/;

	if (width == qconsole.columns_available) // No change so get out
		return;

	if (width < 1)			// video hasn't been initialized yet
	{
		width = 78;

		qconsole.columns_available = width;
		qconsole.rows_available = CONSOLE_MAX_TEXTSIZE / qconsole.columns_available;
		memset (qconsole.console_text, ' ', CONSOLE_MAX_TEXTSIZE);
	}
	else
	{
		oldwidth = qconsole.columns_available;
		qconsole.columns_available = width;
		oldtotallines = qconsole.rows_available;
		qconsole.rows_available = CONSOLE_MAX_TEXTSIZE / qconsole.columns_available;
		numlines = oldtotallines;

		if (qconsole.rows_available < numlines)
			numlines = qconsole.rows_available;

		numchars = oldwidth;

		if (qconsole.columns_available < numchars)
			numchars = qconsole.columns_available;

		memcpy (tbuf, qconsole.console_text, CONSOLE_MAX_TEXTSIZE);
		memset (qconsole.console_text, ' ', CONSOLE_MAX_TEXTSIZE);

		for (i=0 ; i<numlines ; i++)
		{
			for (j=0 ; j<numchars ; j++)
			{
				qconsole.console_text[(qconsole.rows_available - 1 - i) * qconsole.columns_available + j] = tbuf[((qconsole.message_cursor_row - i + oldtotallines) % oldtotallines) * oldwidth + j];
			}
		}

		Con_ClearNotify ();
	}

	qconsole.maximum_backscroll = qconsole.rows_available - (vid.height >> 3 ) - 1; //con_totallines - (vid.height>>3) - 1
	rows_into_backscroll = 0;
	qconsole.message_cursor_row = qconsole.rows_available - 1;
}


/*
================
Con_Init
================
*/
void Con_Init (void)
{
	extern	char	com_gamedirfull[MAX_OSPATH];
#ifndef _DEBUG	// Always want console log if debug
	if (COM_CheckParm("-condebug"))		// FWRITERESTRICT
#endif
		qconsole_log = FS_fopen_write (FULLPATH_TO_QCONSOLE_LOG, isDedicated ? "a" : "w", FS_CREATE_PATH);

	// Potential problem.  gamedir switching in-engine!

	// Allocate the console memory
	qconsole.console_text = Hunk_AllocName (1, CONSOLE_MAX_TEXTSIZE, "context");
	memset (qconsole.console_text, ' ', CONSOLE_MAX_TEXTSIZE);
	qconsole.columns_available = -1;
	Con_CheckResize ();

// register our commands

	Cvar_Registration_Mixed_Console ();

	Cmd_AddCommand ("toggleconsole", Con_ToggleConsole_f);
	Cmd_AddCommand ("messagemode", Con_MessageMode_f);
	Cmd_AddCommand ("messagemode2", Con_MessageMode2_f);
	Cmd_AddCommand ("clear", Con_Clear_f);
	Cmd_AddCommand ("condump", Con_Dump_f); //johnfitz
	Cmd_AddCommand ("time", Con_Showtime_f); // Baker 3.60 - "time command
#if SUPPORTS_CLIPBOARD_COPY
	Cmd_AddCommand ("copy", Con_Copy_f); // Baker 399.m - copy console to clipboard
#endif
	con_initialized = true;
	Con_Printf ("Console initialized\n");
}

void Con_Shutdown (void)
{
	if (qconsole_log)
		fclose (qconsole_log);
}

/*
===============
Con_Linefeed
===============
*/
void Con_Linefeed (void)
{
	qconsole.message_cursor_column = 0;
	qconsole.message_cursor_row++;
	if (qconsole.text_rows_populated < qconsole.rows_available)
		qconsole.text_rows_populated++;
	memset (&qconsole.console_text[(qconsole.message_cursor_row%qconsole.rows_available)*qconsole.columns_available], ' ', qconsole.columns_available);

	// JPG - fix backscroll
	if (rows_into_backscroll)
		rows_into_backscroll++;
}

#define DIGIT(x) ((x) >= '0' && (x) <= '9')
extern qbool jq_mm2;//R00k added for cl_mute
/*
================
Con_Print

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the notify window will pop up.
================
*/
void Con_Print (const char *txt)
{
	int		y, c, l, mask; //, t;
	static int	cr;

	static int fixline = 0;

	//rows_into_backscroll = 0;  // JPG - half of a fix for an annoying problem

	// JPG 1.05 - make the "You got" messages temporary
	if (scr_con_filter.integer)
		fixline |= COM_StringMatch (txt, "You got armor\n") ||
				   COM_StringMatch (txt, "You receive ") ||
				   COM_StringMatch (txt, "You got the ") ||
				   COM_StringMatch (txt, "no weapon.\n") ||
				   COM_StringMatch (txt, "not enough ammo.\n");

	if (txt[0] == 1)
	{
		mask = 128;		// go to colored text
		//R00k cl_mute (0 no mute, 1 silent mm1, 2 silent all)

		if (!scr_notify_chatsound.integer)
			S_LocalSound ("misc/talk.wav");

		txt++;

		// JPG 1.05 - timestamp player binds during a match (unless the bind already has a time in it)
		if (!cls.demoplayback && (!cls.netcon || cls.netcon->mod != MOD_PROQUAKE || *txt == '(') &&		// JPG 3.30 - fixed old bug hit by some servers
			!qconsole.message_cursor_column && scr_con_chatlog_timestamp.integer && (cl.minutes || cl.seconds) && cl.seconds < 128)
		{
			int minutes, seconds, match_time, msg_time;
			char buff[16];
			const char *ch;

			if (cl.match_pause_time)
				match_time = ceil(60.0 * cl.minutes + cl.seconds - (cl.match_pause_time - cl.last_match_time));
			else
				match_time = ceil(60.0 * cl.minutes + cl.seconds - (cl.time - cl.last_match_time));
			minutes = match_time / 60;
			seconds = match_time - 60 * minutes;

			msg_time = -10;
			ch = txt + 2;
			if (ch[0] && ch[1] && ch[2])
			{
				while (ch[2])
				{
					if (ch[0] == ':' && DIGIT(ch[1]) && DIGIT(ch[2]) && DIGIT(ch[-1]))
					{
						msg_time = 60 * (ch[-1] - '0') + 10 * (ch[1] - '0') + (ch[2] - '0');
						if (DIGIT(ch[-2]))
							msg_time += 600 * (ch[-2] - '0');
						break;
					}
					ch++;
				}
			}
			if (msg_time < match_time - 2 || msg_time > match_time + 2)
			{
				if (scr_con_chatlog_timestamp.integer == 1)
					snprintf(buff, sizeof(buff), "%d%c%02d", minutes, 'X' + 128, seconds);
				else
					snprintf(buff, sizeof(buff), "%02d", seconds);

				if (cr)
				{
					qconsole.message_cursor_row--;
					cr = false;
				}
				Con_Linefeed ();
				if (qconsole.message_cursor_row >= 0)
					notify_messages_realtimes[qconsole.message_cursor_row % CONSOLE_MAX_NOTIFY_LINES] = realtime;

				y = qconsole.message_cursor_row % qconsole.rows_available;
				for (ch = buff ; *ch ; ch++)
					qconsole.console_text[y*qconsole.columns_available+qconsole.message_cursor_column++] = *ch - 30;
				qconsole.console_text[y*qconsole.columns_available+qconsole.message_cursor_column++] = ' ';
			}
		}
		//proquake end --JPG
	}
	else if (txt[0] == 2)
	{
		mask = 128;		// go to colored text
		txt++;
	}
	else
	{
		mask = 0;
	}

	while ((c = *txt))
	{
	// count word length
		for (l = 0 ; l < qconsole.columns_available ; l++)
			if (txt[l] <= ' ')
				break;

	// word wrap
		if (l != qconsole.columns_available && (qconsole.message_cursor_column + l > qconsole.columns_available))
			qconsole.message_cursor_column = 0;

		txt++;

		if (cr)
		{
			qconsole.message_cursor_row--;
			cr = false;
		}

		if (!qconsole.message_cursor_column)
		{
			Con_Linefeed ();
		// mark time for transparent overlay
			if (qconsole.message_cursor_row >= 0)
				notify_messages_realtimes[qconsole.message_cursor_row % CONSOLE_MAX_NOTIFY_LINES] = realtime;
		}

		switch (c)
		{
		case '\n':
			qconsole.message_cursor_column = 0;
			cr = fixline;	// JPG 1.05 - make the "you got" messages temporary
			fixline = 0;	// JPG
			break;

		case '\r':
			if (scr_con_chatlog_removecr.integer)	// JPG 3.20 - optionally remove '\r'
				c += 128;
			else
			{
			qconsole.message_cursor_column = 0;
			cr = 1;
			break;
			}

		default:	// display character and advance
			y = qconsole.message_cursor_row % qconsole.rows_available;
			qconsole.console_text[y*qconsole.columns_available+qconsole.message_cursor_column] = c | mask;
			qconsole.message_cursor_column++;
			if (qconsole.message_cursor_column >= qconsole.columns_available)
				qconsole.message_cursor_column = 0;
			break;
		}

	}
}


// JPG - increased this from 4096 to 16384 and moved it up here
// See http://www.inside3d.com/qip/q1/bugs.htm, NVidia 5.16 drivers can cause crash
#define	MAXPRINTMSG	16384

/*
================
Con_Success
================
*/
void Con_Success (const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	//Con_SafePrintf ("\x02Success: ");
	Con_Printf ("%s", msg);
}


/*
================
Con_Warning -- johnfitz -- prints a warning to the console
================
*/
void Con_Warning (const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	Con_SafePrintf ("\x02Warning: ");
	Con_SafePrintf ("%s", msg);
}

#if 0
/*
==================
Con_Debugf

So Baker can remove testing notes more easily ...
==================
*/
void Con_Debugf (const char *fmt, ...)
{
	va_list		argptr;
	char		msg[1024];
	int			temp;

	va_start (argptr,fmt);
	vsnprintf (msg,sizeof(msg),fmt,argptr);
	va_end (argptr);

	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;
	Con_Printf ("%s", msg);
	scr_disabled_for_loading = temp;
}
#endif


/*
================
Con_Printf

Handles cursor positioning, line wrapping, etc
================
*/
void Con_Printf (const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr, fmt);
	vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	// also echo to debugging console
	Sys_Printf ("%s", msg);

	// log all messages to file
	if (qconsole_log)
	{
		char	text[2048];

		StringLCopy (text, msg);
		// JPG 1.05 - translate to plain text
		if (scr_con_chatlog_dequake.integer)
			COMD_DeQuake (text);

		fprintf (qconsole_log, "%s", text);	// Baker note: This doesn't dequake
		fflush	(qconsole_log);				// JPG has this file open and close every instance.  Maybe to avoid locked files if crash?
	}

	if (!con_initialized)
		return;

	if (cls.state == ca_dedicated)
		return;		// no graphics mode

	// write it to the scrollable buffer
	Con_Print (msg);
}

#if OLDSAFEPRINT
/*
================
Con_Printf

Handles cursor positioning, line wrapping, etc
================
*/
// FIXME: make a buffer size safe vsprintf?
void Con_Printf (const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	static qbool	inupdate;

	va_start (argptr,fmt);
	vsnprintf (msg,sizeof(msg),fmt,argptr);
	va_end (argptr);

// also echo to debugging console
	Sys_Printf ("%s", msg);	// also echo to debugging console

// log all messages to file
	if (con_debuglog)
		Con_DebugLog( /* va("%s/qconsole.log",com_gamedir), */ "%s", msg);  // JPG - got rid of filename

	if (!con_initialized)
		return;

	if (cls.state == ca_dedicated)
		return;		// no graphics mode

// write it to the scrollable buffer
	Con_Print (msg);

// update the screen if the console is displayed
	if (cls.signon != SIGNONS && !scr_disabled_for_loading )
	{
	// protect against infinite loop if something in SCR_UpdateScreen calls
	// Con_Printd
		if (!inupdate)
		{
			inupdate = true;
			SCR_UpdateScreen ();
			inupdate = false;
		}
	}
}


/*
==================
Con_SafePrintf

Okay to call even when the screen can't be updated
==================
*/
void Con_SafePrintf (const char *fmt, ...)
{
	va_list		argptr;
	char		msg[1024];
	int			temp;

	va_start (argptr,fmt);
	vsnprintf (msg,sizeof(msg),fmt,argptr);
	va_end (argptr);

	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;
	Con_Printf ("%s", msg);
	scr_disabled_for_loading = temp;
}
#endif

// Baker: This is a FAKE
/*
================
Con_SafePrintf

Okay to call even when the screen can't be updated
================
*/
void Con_SafePrintf (const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr, fmt);
	vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	Con_Printf ("%s", msg);
}

/*
================
Con_DevPrintf

A Con_Printf that only shows up if the "developer" cvar is set
================
*/

char *dev_filters[] = {
	"ANY",          // 0
	"INPUT",        // 1
	"KEYBOARD",     // 2
	"MOUSE",        // 3
	"VIDEO",        // 4
	"GRAPHICS",     // 5
	"DEMOS",        // 6
	"IO",           // 7
	"PROTOCOL",     // 8
	"MODEL",        // 9
	"SOUND",        // 10
	"GAMEDIR",      // 11
	"IMAGE",        // 12
	"OPENGL",       // 13
	"SERVER",       // 14
	"SYSTEM",       // 15
	"GAMMA",        // 16
	"CVAR"			// 17
};

int num_dev_filters =  sizeof(dev_filters)/sizeof(dev_filters[0]);

void Con_DevMode_f (void)
{
	int i;

	if (Cmd_Argc() < 2)
	{
		qbool current_filter_valid = ((int)developer_filter.integer > 0 && (int)developer_filter.integer < num_dev_filters);

		Con_Printf ("Usage: %s <filter>\n", Cmd_Argv(0));
		Con_Printf ("Filters developer messages to group\n\n");
//		Con_Printf ("-----------------------------------\n");
		Con_Printf ("Current filter is: \"%s\"\n", current_filter_valid ? dev_filters[(int)developer_filter.integer] : "(invalid)");
//		Con_Printf ("-----------------------------------\n");
		Con_Printf ("\nFilters:\n\n");
		for (i = 0; i<num_dev_filters ; i++)
			Con_PrintColumnItem (dev_filters[i]);
		Con_Printf ("\n\n");
		Con_Printf ("Do \"dev off\" to turn off\n\n");
	}

	if (COM_StringMatchCaseless (Cmd_Argv(1), "off"))
	{
		if (developer.integer)
			Cvar_SetFloatByRef (&developer, 0);

		Con_Printf ("Developer messages %s\n", developer.integer ? "on" : "off" );
		return;
	}

	// Set new filter
	for (i=0 ; i <num_dev_filters; i++)
	{
		if (COM_StringMatchCaseless (Cmd_Argv(1), dev_filters[i]))
		{
			// match
			Cvar_SetFloatByRef (&developer_filter, (float)i);
			Con_Printf ("Message filter set to: %s (%i)\n", dev_filters[(int)developer_filter.integer], (int)developer_filter.integer);
			break;
		}
	}

	if (i == num_dev_filters)
	{
		Con_Printf ("Invalid message filter mode\n");
		return;
	}

	Cvar_SetFloatByRef (&developer, 2);
	Con_Printf ("Developer messages %s\n", developer.integer ? "on" : "off" );
	return;
}

void Con_DevPrintf (int DevModeType, const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	if (!developer.integer)
		return;			// don't confuse non-developers with techie stuff...


	if (developer.integer == 2 && developer_filter.integer)
	{
		qbool passes_filter = false;

		if (DevModeType == (int)developer_filter.integer)	// Mode matches filter
			passes_filter = true;
		else if ((int)developer_filter.integer == DEV_INPUT)
		{

			if (DevModeType == DEV_MOUSE || DevModeType == DEV_KEYBOARD );
				passes_filter = true;
		}

		if (passes_filter == false)
			return;
	}

	if (developer.integer == 2)	// Print this first
	{
		if (DevModeType >= 0 && DevModeType < num_dev_filters)
			Con_Printf ("%s: ", dev_filters[DevModeType]);
		else
			Con_Printf ("<invalid>: ");
	}

	va_start (argptr, fmt);
	vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	Con_Printf ("%s", msg);
}


/*
================
Con_CenterPrintf -- johnfitz -- pad each line with spaces to make it appear centered
================
*/
void Con_CenterPrintf (int linewidth, const char *fmt, ...)
{
	va_list	argptr;
	char	msg[MAXPRINTMSG]; //the original message
	char	line[MAXPRINTMSG]; //one line from the message
	char	spaces[21]; //buffer for spaces
	char	*src, *dst;
	int		len, s;

	va_start (argptr,fmt);
	vsnprintf (msg, sizeof(msg), fmt,argptr);
	va_end (argptr);

	linewidth = min (linewidth, qconsole.columns_available);
	for (src = msg; *src; )
	{
		dst = line;
		while (*src && *src != '\n')
			*dst++ = *src++;
		*dst = 0;
		if (*src == '\n')
			src++;

		len = strlen(line);
		if (len < linewidth)
		{
			s = (linewidth-len)/2;
			memset (spaces, ' ', s);
			spaces[s] = 0;
			Con_Printf ("%s%s\n", spaces, line);
		}
		else
			Con_Printf ("%s\n", line);
	}
}

/*
==================
Con_LogCenterPrint -- johnfitz -- echo centerprint message to the console
==================
*/
void Con_LogCenterPrint (const char *str)
{
	if (COM_StringMatch (str, con_lastcenterstring))
		return; //ignore duplicates

	if (cl.gametype == GAME_DEATHMATCH && scr_centerprint_log.integer != 2)
		return; //don't log in deathmatch

	strcpy (con_lastcenterstring, str);

	if (scr_centerprint_log.integer)
	{
		Con_Printf (Con_Quakebar(40));
		Con_CenterPrintf (40, "%s\n", str);
		Con_Printf (Con_Quakebar(40));
		Con_ClearNotify ();
	}
}


/*
==============================================================================

DRAWING

==============================================================================
*/

/*
================
Con_DrawInput

The input line scrolls horizontally if typing goes beyond the right edge
================
*/
void Con_DrawInput (void)
{
	int		i;
	char	*text;
	char		temp[MAXCMDLINE];

	if (key_dest != key_console && !con_forcedup)
		return;		// don't draw anything

	text = strcpy (temp, key_lines[edit_line]);

	// fill out remainder with spaces
	for (i = strlen(text) ; i < MAXCMDLINE ; i++)
		text[i] = ' ';

	// add the cursor frame
	if ((int)(realtime * con_cursorspeed) & 1)
		text[key_linepos] = 11 + 84 * key_insert;

	// prestep if horizontally scrolling
	if (key_linepos >= qconsole.columns_available)
		text += 1 + key_linepos - qconsole.columns_available;

	// draw it
	Draw_String (8, console_visible_pixel_lines - 16, text);
}


/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
void Con_DrawNotify (void)
{
	int		x, v, i, maxlines;
	char		*text;
	float		time;
	extern	char	chat_buffer[];

	maxlines = CLAMP (0, scr_notify_lines.integer, CONSOLE_MAX_NOTIFY_LINES);

	v = 0;
	for (i = qconsole.message_cursor_row - maxlines + 1 ; i <= qconsole.message_cursor_row ; i++)
	{
		if (i < 0)
			continue;
		time = notify_messages_realtimes[i%CONSOLE_MAX_NOTIFY_LINES];
		if (time == 0)
			continue;
		time = realtime - time;
		if (time > scr_notify_time.floater)
			continue;
		text = qconsole.console_text + (i % qconsole.rows_available) * qconsole.columns_available;

		clearnotify = 0;
//		scr_copytop = 1;

		for (x = 0 ; x < qconsole.columns_available ; x++)
			Draw_Character ((x+1)<<3, v, text[x]);

		v += 8;
	}

	if (key_dest == key_message)
	{
		clearnotify = 0;
//		scr_copytop = 1;

		// JPG - was x = 0 etc.. recoded with x = 5, i = 0
		i = 0;

		// JPG - added support for team messages
		if (team_message)
		{
			Draw_String (8, v, "(say team):");
			x = 12; // Baker 3.90: 7 increased to 12 for "say_team"
		}
		else
		{
			Draw_String (8, v, "say:");
			x = 5;
		}

		while (chat_buffer[i])
		{
			Draw_Character (x << 3, v, chat_buffer[i]);
			x++;

			// JPG - added this for longer says
			i++;
			if (x > qconsole.columns_available)
			{
				x = team_message ? 12 : 5; // Baker 3.90: 7 increased to 12 "(say)" ---> "(say_team)"
				v += 8;
			}
		}
		Draw_Character (x << 3, v, 10 + ((int)(realtime * con_cursorspeed) & 1));
		v += 8;
	}

	if (v > con_notifylines)
		con_notifylines = v;
}

/*
================
Con_DrawConsole

Draws the console with the solid background
The typing input line at the bottom should only be drawn if typing is allowed
================
*/
void Con_DrawConsole (int lines, qbool drawinput)
{
	int		i, j, x, y, rows;
	char		*text;

	if (lines <= 0)
		return;

// draw the background
	Draw_ConsoleBackground (lines);

// draw the text
	console_visible_pixel_lines = lines;

	rows = (lines - 16)>>3;		// rows of text to draw
	y = lines - 16 - (rows<<3);	// may start slightly negative

	for (i = qconsole.message_cursor_row - rows + 1 ; i <= qconsole.message_cursor_row ; i++, y += 8)
	{
		j = i - rows_into_backscroll;
		if (j < 0)
			j = 0;

		if (rows_into_backscroll && i == qconsole.message_cursor_row)
		{
			for (x = 0 ; x < qconsole.columns_available ; x += 4)
				Draw_Character ((x+1)<<3, y, '^');
			continue;
		}

		text = qconsole.console_text + (j % qconsole.rows_available)*qconsole.columns_available;

		for (x = 0 ; x < qconsole.columns_available ; x++)
			Draw_Character ((x+1)<<3, y, text[x]);
	}

// draw the input prompt, user text, and cursor if desired
	if (drawinput)
		Con_DrawInput ();
}


/*
==================
Con_NotifyBox
==================
*/
void Con_NotifyBox (char *text)
{
	double		t1, t2;

// during startup for sound / cd warnings
	Con_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n");

	Con_Printf (text);

	Con_Printf ("Press a key.\n");
	Con_Printf("\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n");

	key_count = -2;		// wait for a key down and up
	key_dest = key_console;

	do {
		t1 = Sys_DoubleTime ();
		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();
		t2 = Sys_DoubleTime ();
		realtime += t2-t1;		// make the cursor blink
	} while (key_count < 0);

	Con_Printf ("\n");
	key_dest = key_game;
	realtime = 0;				// put the cursor back to invisible
}



/*
==================
Con_PrintColumnItem
==================
*/
#define	COLUMNWIDTH	20
#define	MINCOLUMNWIDTH	18	// the last column may be slightly smaller

//extern	int	qconsole.message_cursor_column;

void Con_PrintColumnItem (const char *txt)
{
//	extern	int	qconsole.columns_available;
	int		nextcolx = 0;

	if (qconsole.message_cursor_column)
		nextcolx = (int)((qconsole.message_cursor_column + COLUMNWIDTH) / COLUMNWIDTH) * COLUMNWIDTH;

	// Advance to next line if no room
	if (nextcolx > qconsole.columns_available - MINCOLUMNWIDTH || (qconsole.message_cursor_column && nextcolx + strlen(txt) >= qconsole.columns_available))
		Con_Printf ("\n");

	// Add a space if qconsole.message_cursor_column isn't 0 (Baker: I don't think qconsole.message_cursor_column can be zero anyway, console doesn't use column 0 ...
	// But this speculation isn't confirmed
	if (qconsole.message_cursor_column)
		Con_Printf (" ");
	while (qconsole.message_cursor_column % COLUMNWIDTH) // Space it up until we are at a valid column position
		Con_Printf (" ");
	Con_Printf ("%s", txt);
}

// Like above, except limits string to mincolumnwidth
void Con_PrintColumnItem_MaxWidth (const char *txt)
{
	char buffer[MINCOLUMNWIDTH];

	StringLCopy (buffer, txt);

	if (strlen(txt)>MINCOLUMNWIDTH)
	{
		// Shave buffer
		buffer[MINCOLUMNWIDTH-1] =   0;
		buffer[MINCOLUMNWIDTH-2] = '.';
		buffer[MINCOLUMNWIDTH-3] = '.';
		buffer[MINCOLUMNWIDTH-4] = '.';
		buffer[MINCOLUMNWIDTH-5] = ' ';
	}

	Con_PrintColumnItem (buffer);
}

int Con_GetMaximumBackscroll (void)
{
	// Baker: There isn't a "fix" to this (there might be a workaround).  You can't have the lines of backscroll be "variable".
	// And the console size can vary based on fullscreen, halfscreen, resize of video mode, change of conwidth.
	// Even vertically resizing the console.  A little extra wraparound won't kill us.
	// The alternative is not drawing console before the beginning, which looks dumb.  Or wasting a ton of time
	// Redesigning this and it isn't worth it.  Especially not right now.
	return (qconsole.rows_available);// - (vid.height >> 3 ) - 1);
}

int Con_GetCursorColumn (void)
{
	return qconsole.message_cursor_column;
}