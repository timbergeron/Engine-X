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
// common.c -- misc functions used in client and server
// Baker: Validated 6-27-2011.  Boring functionary changes.

#include "quakedef.h"
#include <assert.h>  // strltrim strrtrim


//cvar_t  session_registered = {"registered", "0"};
//cvar_t  session_cmdline = {"cmdline", "0", CVAR_SERVER};

//qbool	msg_suppress_1 = 0;

void COM_InitFilesystem (void);

// if a packfile directory differs from this, it is assumed to be hacked
#define PAK0_COUNT              339
#define PAK0_CRC                32981

int	rogue = 0, hipnotic = 0, nehahra = 0;

/*

All of Quake's data access is through a hierchal file system, but the contents
of the file system can be transparently merged from several sources.

The "base directory" is the path to the directory holding the quake.exe and all
game directories. The sys_* files pass this to host_init in quakeparms_t->basedir.
This can be overridden with the "-basedir" command line parm to allow code
debugging in a different directory. The base directory is only used during
filesystem initialization.

The "game directory" is the first tree on the search path and directory that all
generated files (savegames, screenshots, demos, config files) will be saved to.
This can be overridden with the "-game" command line parameter.
The game directory can never be changed while quake is executing.
This is a precacution against having a malicious server instruct clients to
write files over areas they shouldn't.

The "cache directory" is only used during development to save network bandwidth,
especially over ISDN / T1 lines.  If there is a cache directory specified, when
a file is found by the normal search path, it will be mirrored into the cache
directory, then opened there.


FIXME:
The file "parms.txt" will be read out of the game directory and appended to the
current command line arguments to allow different games to initialize startup
parms differently. This could be used to add a "-sspeed 22050" for the high
quality sound edition. Because they are added at the end, they will not override
an explicit setting on the original command line.

*/



/*
================
COM_Init
================
*/
void COM_Path_f (void);	// Baker 2011-09-16 none of these are in header files. ;-)
void COM_Printdirs_f (void);
void COM_Gamedir_f (void);
void COM_CheckRegistered (void);

void COM_Init (char *basedir)
{
	COM_EndianessCheck ();		// Determine big endian verus little endian

	Cvar_Registration_Host_Common ();

	Cmd_AddCommand ("path", COM_Path_f);
	Cmd_AddCommand ("gamedir", COM_Gamedir_f);
	Cmd_AddCommand ("compaths", COM_Printdirs_f);

	COM_InitFilesystem ();
	COM_CheckRegistered ();
}


/*
=======================
COMD_DeQuake

JPG 1.05 - initialize the dequake array
Baker: Self initializes
======================
*/

void COMD_DeQuake (char *text_to_modify)
{
	static char		dequake[256];	// JPG 1.05
	static qbool	dequake_initalized=false;
	unsigned char	*myChar;
	int				i;	

	if (!dequake_initalized)	// 32 to 128 is itself, Convert everything over 128 to be 0-127 range.  Exceptions noted ...
	{
		//Tab				// Newline			// Carriage return	// Line feed becomes a space
		dequake[9] = 9;		dequake[10] = 10;	dequake[13] = 13;	dequake[12] = ' ';	
		dequake[1] =		dequake[5] =		dequake[14] =		dequake[15] =		dequake[28] = '.';
		dequake[16] = '[';	dequake[17] = ']';	dequake[29] = '<';	dequake[30] = '-';	dequake[31] = '>';
		// Iterations ...
		for (i =   1 ;	i <  12 ;	i++)		dequake[i] = '#';
		for (i =  18 ;	i <  28 ;	i++)		dequake[i] = '0' + i - 18;
		for (i =  32 ;	i < 128 ;	i++)		dequake[i] = i;														
		for (i = 128 ;	i < 255 ;	i++)		dequake[i] = dequake[i & 127];										
		// Final touches
		dequake[128] = '(';	dequake[129] = '=';	dequake[130] = ')';	dequake[131] = '*';	dequake[141] = '>';	
		dequake_initalized = true;
	}

	for (myChar = text_to_modify ; *myChar ; myChar++)
		*myChar = dequake[*myChar];
}

void COMD_DeQuake_Name (char *text_to_modify) // Turns carriage returns and linefeeds into spaces
{
	unsigned char	*myChar;
	COMD_DeQuake (text_to_modify);
	
	for (myChar = text_to_modify ; *myChar ; myChar++)
		if (*myChar == 10 || *myChar == 13)	
			*myChar = ' ';	// Special case: strip wannabe linefeeds from name
}


