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
// com_parse.c -- parsing stuffs

#include "quakedef.h"


char	com_token[1024];
int	com_argc;
char	**com_argv;

#define CMDLINE_LENGTH	256
char	com_cmdline[CMDLINE_LENGTH];


/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char *COM_Parse (char *data)
{
	int	c, len;

	len = 0;
	com_token[0] = 0;

	if (!data)
		return NULL;

// skip whitespace
skipwhite:
	while ((c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;		// end of file;
		data++;
	}

// skip // comments
	if (c == '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}


// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c == '\"' || !c)
			{
				com_token[len] = 0;
				return data;
			}
			com_token[len] = c;
			len++;
		}
	}

// parse single characters
	if (c == '{' || c == '}'|| c == ')'|| c == '(' || c == '\'' || c == ':')
	{
		com_token[len] = c;
		len++;
		com_token[len] = 0;
		return data+1;
	}

// parse a regular word
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;
		// joe, from ProQuake: removed ':' so that ip:port works
		if (c == '{' || c == '}'|| c == ')'|| c == '(' || c == '\'' /*|| c==':'*/) 	// JPG 3.20 - so that ip:port works
			break;
	} while (c > 32);

	com_token[len] = 0;
	return data;
}


/*
================
COM_CheckParm

Returns the position (1 to argc-1) in the program's argument list
where the given parameter apears, or 0 if not present
================
*/
int COM_CheckParm (const char *parm)
{
	int	i;

	for (i=1 ; i<com_argc ; i++)
	{
		if (!com_argv[i])
			continue;		// NEXTSTEP sometimes clears appkit vars.
		if (COM_StringMatch (parm,com_argv[i]))
			return i;
	}

	return 0;
}

int COM_Commandline_GetInteger (const char *param)
{
	const int	i		= COM_CheckParm(param);
	int			retval	= 0;

	if ((i + 1) < com_argc)	 // If there is a next param, that'll be the return value
		retval = atoi (com_argv[i+1]);

	return retval;
}



int COM_Argc (void)
{
	return com_argc;
}

char *COM_Argv (int arg)
{
	if (arg < 0 || arg >= com_argc)
		return "";
	return com_argv[arg];
}

#define NUM_SAFE_ARGVS  7

static	char	*largv[MAX_NUM_ARGVS + NUM_SAFE_ARGVS + 1];
static	char	*argvdummy = " ";

static char *safeargvs[NUM_SAFE_ARGVS] = {
	"-stdvid",
	"-nolan",
	"-nosound",
	"-nocdaudio",
	"-joystick",
	"-nomouse",
	"-dibonly"
};


/*
================
COM_InitArgv
================
*/
void COM_InitArgv (int argc, char **argv)
{
	qbool	safe;
	int		i, j, n;

// reconstitute the command line for the cmdline externally visible cvar
	n = 0;

	for (j=0 ; j<MAX_NUM_ARGVS && j<argc ; j++)
	{
		i = 0;

		while ((n < (CMDLINE_LENGTH - 1)) && argv[j][i])
		{
			com_cmdline[n++] = argv[j][i++];
		}

		if (n < (CMDLINE_LENGTH - 1))
			com_cmdline[n++] = ' ';
		else
			break;
	}

	com_cmdline[n] = 0;

	safe = false;

	for (com_argc = 0 ; com_argc < MAX_NUM_ARGVS && com_argc < argc ; com_argc++)
	{
		largv[com_argc] = argv[com_argc];
		if (COM_StringMatch ("-safe", argv[com_argc]))
			safe = true;
	}

	if (safe)
	{
	// force all the safe-mode switches. Note that we reserved extra space in
	// case we need to add these, so we don't need an overflow check
		for (i=0 ; i<NUM_SAFE_ARGVS ; i++)
		{
			largv[com_argc] = safeargvs[i];
			com_argc++;
		}
	}

	largv[com_argc] = argvdummy;
	com_argv = largv;

	if (COM_CheckParm("-rogue"))
		rogue = 1;

	if (COM_CheckParm("-hipnotic") || COM_CheckParm ("-quoth"))
		hipnotic = 1;

#ifdef SUPPORTS_NEHAHRA
	if (COM_CheckParm("-nehahra"))
		nehahra = 1;
#endif

	if (hipnotic && rogue)
		Sys_Error ("You can't run both mission packs at the same time");
}

char *StringTemp_ObtainValueFromClientWorldSpawn (const char *find_keyname)
{
	static char		valuestring[4096];
	char			current_key[128];
	char			*data;

	// Read some data ...
	if (!(data = COM_Parse(data = cl.worldmodel->entities)) || com_token[0] != '{')	// Opening brace is start of worldspawn
		return NULL; // error

	while (1)
	{

		// Read some data ...
		if (!(data = COM_Parse(data)) || /* end of worldspawn --> */ com_token[0] == '}')	// Closing brace is end of worldspawn
			return NULL; // End of worldspawn

		// Copy data over, skipping '_' in a keyname
		StringLCopy (current_key, com_token + (com_token[0] == '_' ? 1: 0));

		COMD_RemoveTrailingSpaces (current_key);

		if (!(data = COM_Parse(data)))
			return NULL; // error

		if (COM_StringMatchCaseless (find_keyname, current_key))
		{
			StringLCopy (valuestring, com_token);
			return valuestring;
		}

	}

	return NULL;
}
