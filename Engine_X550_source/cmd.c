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
// cmd.c -- Quake script command processing module
// Baker: Validated 6-27-2011.  Nothing that isn't functionary.

#include "quakedef.h"



void Cmd_ForwardToServer_f (void);


#ifdef _WIN32
#include "winquake.h"
#endif
// joe: ReadDir()'s stuff
#ifndef _WIN32 // Headers
#include <glob.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

static cmd_alias_t	*cmd_alias;

static int		trashtest, *trashspot;

static qbool	cmd_wait;

//=============================================================================

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "impulse 5 ; +attack ; wait ; -attack ; impulse 2"
============
*/
static void Cmd_Wait_f (void)
{
	cmd_wait = true;
}

/*
=============================================================================

				COMMAND BUFFER

=============================================================================
*/

static sizebuf_t	cmd_text;

/*
============
Cbuf_Init
============
*/
void Cbuf_Init (void)
{
	SZ_Alloc (&cmd_text, 8192);		// space for commands and script files
//	SZ_Alloc (&cmd_network_text,	8192);		// space for untrusted commands and script files (server, demo, etc.)



}


/*
============
Cbuf_AddText

Adds command text at the end of the buffer
============
*/
void Cbuf_AddText (char *text)
{
	int	l;

	l = strlen (text);

	if (cmd_text.cursize + l >= cmd_text.maxsize)
	{
		Con_Warning ("Cbuf_AddText: overflow\n");
		return;
	}

	SZ_Write (&cmd_text, text, strlen(text), true /* is command buffer*/);
}

/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
FIXME: actually change the command buffer to do less copying
============
*/
void Cbuf_InsertText (char *text)
{
	char	*temp = NULL;
	int	templen;

//	if (strlen(text) > 8192)
//		Con_Printf ("Warning exceeds maxsize\n");


// copy off any commands still remaining in the exec buffer
	if ((templen = cmd_text.cursize))
	{
		temp = Z_Malloc (templen);
		memcpy (temp, cmd_text.data, templen);
		SZ_Clear (&cmd_text);
	}

// add the entire text of the file
	Cbuf_AddText (text);

// add the copied off data
	if (templen)
	{
		SZ_Write (&cmd_text, temp, templen, true /*is command buffer*/);
		Z_Free (temp);
	}
}

/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute (void)
{
	int		i;
	char	*text;
	char line[1024];
	int		quotes;
	int		notcmd;	// JPG - so that the ENTIRE line can be forwarded

	while (cmd_text.cursize)
	{
// find a \n or ; line break
		text = (char *)cmd_text.data;

		quotes = 0;
		notcmd = strncmp(text, "cmd ", 4);  // JPG - so that the ENTIRE line can be forwarded
		for (i=0 ; i<cmd_text.cursize ; i++)
		{
			if (text[i] == '"')
				quotes++;
			if ( !(quotes&1) &&  text[i] == ';' && notcmd)   // JPG - added && cmd so that the ENTIRE line can be forwareded
				break;	// don't break if inside a quoted string
			if (text[i] == '\n')
				break;
		}

		memcpy (line, text, i);
		line[i] = 0;

// delete the text from the command buffer and move remaining commands down
// this is necessary because commands (exec, alias) can insert data at the
// beginning of the text buffer

		if (i == cmd_text.cursize)
		{
			cmd_text.cursize = 0;
		}
		else
		{
			i++;
			cmd_text.cursize -= i;
			memcpy (text, text + i, cmd_text.cursize);
		}

// execute the command line
		Cmd_ExecuteString (line, src_command);

		if (cmd_wait)
		{
			// skip out while text still remains in buffer, leaving it for next frame
			cmd_wait = false;
			break;
		}
	}
}

/*
==============================================================================

				SCRIPT COMMANDS

==============================================================================
*/

/*
===============
Set commands are added early, so they are guaranteed to be set before
the client and server initialize for the first time.

Other commands are added late, after all initialization is complete.
===============
*/
#if 0
void Cbuf_AddEarlyCommands (void)
{
	int	i;

	for (i = 0 ; i < COM_Argc() - 2 ; i++)
	{
		if (COM_StringNOTMatchCaseless(COM_Argv(i), "+set"))
			continue;
		Cbuf_AddText (va("set %s %s\n", COM_Argv(i+1), COM_Argv(i+2)));
		i += 2;
	}
}
#endif

#if SUPPORTS_STUFFCMDS
/*
===============
Cmd_StuffCmds_f

Adds command line parameters as script statements
Commands lead with a +, and continue until a - or another +
quake +prog jctest.qp +cmd amlev1
quake -nosound +cmd amlev1
===============
*/
static void Cmd_StuffCmds_f (void)
{
	int		i, j;
	int		s;
	char	*text, *build, c;


// build the combined string to parse from
	s = 0;
	for (i=1 ; i<com_argc ; i++)
	{
		if (!com_argv[i])
			continue;		// NEXTSTEP nulls out -NXHost
		s += strlen (com_argv[i]) + 1;
	}
	if (!s)
		return;

	text = Z_Malloc (s + 1);
	text[0] = 0;
	for (i=1 ; i<com_argc ; i++)
	{
		if (!com_argv[i])
			continue;		// NEXTSTEP nulls out -NXHost
		strcat (text, com_argv[i]);   // Dynamic string: no strlcat required
		if (i != com_argc-1)
			strcat (text, " ");   // Dynamic string: no strlcat required
	}

// pull out the commands
	build = Z_Malloc (s + 1);
	build[0] = 0;

	for (i=0 ; i<s-1 ; i++)
	{
		if (text[i] == '+')
		{
			i++;

			for (j=i ; (text[j] != '+') && (text[j] != '-') && (text[j] != 0) ; j++)
				;

			c = text[j];
			text[j] = 0;

			strcat (build, text + i);  // Dynamic string: no strlcat required
			strcat (build, "\n");  // Dynamic string: no strlcat required
			text[j] = c;
			i = j - 1;
		}
	}

	if (build[0])
	{
		Con_DevPrintf (DEV_CVAR, "Stuffcmds:\n------\n%s------\n", build);
		Cbuf_InsertText (build);
	}

	Z_Free (text);
	Z_Free (build);
}
#endif

/*
===============
Cmd_Exec_f
===============
*/

static void Cmd_Exec_f (void)
{
	if (Cmd_Argc() != 2)
	{
		Con_Printf ("exec <filename> : execute a script file\n");
		return;
	}

	do
	{

		int		mark = Hunk_LowMark ();

		// Baker: If kurok, limit command execution to current gamedir.
		char	*use_this_pathlimit = game_kurok.integer ? com_gamedirfull : com_enginedir;
#pragma message ("Quality assurance: Turn Kurok script execution gamedir limitor into a cvar so other mods can limit script searching to only their own gamedir")
		char	name[MAX_OSPATH];
		char	*script_file_to_execute;

		// Copy the name and try to load into memory
		StringLCopy (name, Cmd_Argv(1));
		script_file_to_execute = (char *)QFS_LoadHunkFile(name, use_this_pathlimit /*PATH LIMIT ME*/);

		if (!script_file_to_execute)
		{
			// Couldn't execute it, try .cfg the default extension
			char	*p = StringTemp_SkipPath (name);	// Baker: pointer p is not used to modify name, so this is ok

			if (!strchr(p, '.'))
			{	// no extension, so append default config extension (.cfg) and try again
				strlcat (name, ".cfg",  sizeof(name));
				script_file_to_execute = (char *)QFS_LoadHunkFile (name, use_this_pathlimit /*PATH LIMIT ME*/);
			}

			if (!script_file_to_execute)
			{	// Still failed
				Con_Printf ("couldn't exec %s\n", name);
				break;
			}
		}
		Sys_CarriageReturnFix (script_file_to_execute);	// Mac OS X needs carriage returns eliminated; on Windows this function is blank
#pragma message ("Confirm this is actually and issue with some notepad generated config.cfg")
		Con_Printf ("execing %s\n",name);

		// Baker hack: Detect default.cfg initial execution, add an extra command signaling the end of accepting game defaults
		if (game_in_initialization && COM_StringMatchCaseless (name, "default.cfg"))
			Cbuf_InsertText ("\nhint_defaults_done\n");	// Put this stuff in reverse order, InsertText jumps the line
		// End Baker hack

		Cbuf_InsertText (script_file_to_execute);

		Hunk_FreeToLowMark (mark);
	} while (0);
}

/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
static void Cmd_Echo_f (void)
{
	int	i;

	for (i=1 ; i<Cmd_Argc() ; i++)
		Con_Printf ("%s ", Cmd_Argv(i));
	Con_Printf ("\n");
}

/*
===============
Cmd_Alias_f -- johnfitz -- rewritten

Creates a new command that executes a command string (possibly ; separated)
===============
*/
static void Cmd_Alias_f (void)
{
	cmd_alias_t	*a;
	char		cmd[1024];
	int			i, c;
	char		*s;


	switch (Cmd_Argc())
	{
	case 1: //list all aliases
		for (a = cmd_alias, i = 0; a; a=a->next, i++)
			Con_SafePrintf ("   %s: %s", a->name, a->value);
		if (i)
			Con_SafePrintf ("%i alias command(s)\n", i);
		else
			Con_SafePrintf ("no alias commands found\n");
		break;
	case 2: //output current alias string
		for (a = cmd_alias ; a ; a=a->next)
			if (COM_StringMatch (Cmd_Argv(1), a->name))
				Con_Printf ("   %s: %s", a->name, a->value);
		break;

	default: //set alias string

		s = Cmd_Argv(1);
		if (strlen(s) >= MAX_ALIAS_NAME)
		{
			Con_Printf ("Alias name is too long\n");
			return;
		}

		// if the alias allready exists, reuse it
		for (a = cmd_alias ; a ; a=a->next)
		{
			if (COM_StringMatch (s, a->name))
			{
				Z_Free (a->value);
				break;
			}
		}

		if (!a)
		{
			a = Z_Malloc (sizeof(cmd_alias_t));
			a->next = cmd_alias;
			cmd_alias = a;
		}
		StringLCopy (a->name, s); // Baker: StringLCopy vulnerability

	// copy the rest of the command line
		cmd[0] = 0;		// start out with a null string
		c = Cmd_Argc();
		for (i=2 ; i< c ; i++)
		{
			StringLCat (cmd, Cmd_Argv(i)); // strlcat (cmd, Cmd_Argv(i), sizeof(cmd));
			if (i != c)
				StringLCat (cmd, " "); // strlcat (cmd, " ", sizeof(cmd));
		}
		StringLCat (cmd, "\n");	// strlcat (cmd, "\n", sizeof(cmd));

		a->value = Z_Strdup (cmd);
		break;
	}
}

/*
===============
Cmd_Unalias_f -- johnfitz
===============
*/
static void Cmd_Unalias_f (void)
{
	cmd_alias_t	*a, *prev;

	switch (Cmd_Argc())
	{
	default:
	case 1:
		Con_Printf("unalias <name> : delete alias\n");
		break;
	case 2:
		for (prev = a = cmd_alias; a; a = a->next)
		{
			if (COM_StringMatch (Cmd_Argv(1), a->name))
			{
				prev->next = a->next;
				Z_Free (a->value);
				Z_Free (a);
				prev = a;
				return;
			}
			prev = a;
		}
		break;
	}
}

/*
===============
Cmd_Unaliasall_f -- johnfitz
===============
*/
void Cmd_Unaliasall_f (void)
{
	cmd_alias_t	*blah;

	while (cmd_alias)
	{
		blah = cmd_alias->next;
		Z_Free(cmd_alias->value);
		Z_Free(cmd_alias);
		cmd_alias = blah;
	}
}

/*
============
Cmd_FindAlias
============
*/
cmd_alias_t *Cmd_FindAlias (char *name)
{
	cmd_alias_t	*alias;

	for (alias = cmd_alias ; alias ; alias = alias->next)
		if (COM_StringMatchCaseless (name, alias->name))
			return alias;

	return NULL;
}


/*
=============================================================================
					COMMAND EXECUTION
=============================================================================
*/

static	cmd_function_t	*cmd_functions;		// possible commands to execute

#define	MAX_ARGS	80

static	int	cmd_argc;
static	char	*cmd_argv[MAX_ARGS];
static	char	*cmd_null_string = "";
static	char	*cmd_args = NULL;

cmd_source_t	cmd_source;


/*
================
Cmd_CheckParmMod

Returns the position (1 to argc-1) in tokenized string
where the given parameter appears, or 0 if not present
================
*/
// Baker: Not used by the original source code
// Modifying this ever so slightly to return -1 on failure
// and to search argv[0], otherwise it will fail on
// if match is arg0
int Cmd_CheckParmMod (const char *parm)
{
	int	i;

	if (!parm)
		Sys_Error ("Cmd_CheckParmMod: NULL");

	for (i=0 ; i<cmd_argc ; i++)
	{
		if (!cmd_argv[i])
			continue;
		if (COM_StringMatch (parm,cmd_argv[i]))
			return i;
	}

	return -1;
}


/*
============
Cmd_Argc
============
*/
int Cmd_Argc (void)
{
	return cmd_argc;
}

/*
============
Cmd_Argv
============
*/
char *Cmd_Argv (int arg)
{
	if ((unsigned)arg >= cmd_argc)
		return cmd_null_string;
	return cmd_argv[arg];
}

/*
============
Cmd_Args
============
*/
char *Cmd_Args (void)
{
	if (!cmd_args)
		return "";
	return cmd_args;
}

/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
============
*/
void Cmd_TokenizeString (char *text)
{
	int		idx;
	static	char	argv_buf[1024];

	idx = 0;

	cmd_argc = 0;
	cmd_args = NULL;

	while (1)
	{
	// skip whitespace up to a /n
		while (*text == ' ' || *text == '\t' || *text == '\r')
			text++;

		if (*text == '\n')
		{	// a newline separates commands in the buffer
			text++;
			break;
		}

		if (!*text)
			return;

		if (cmd_argc == 1)
			 cmd_args = text;

		if (!(text = COM_Parse(text)))
			return;

		if (cmd_argc < MAX_ARGS)
		{
			cmd_argv[cmd_argc] = argv_buf + idx;
			strcpy (cmd_argv[cmd_argc], com_token);
			idx += strlen(com_token) + 1;
			cmd_argc++;
		}
	}
}

/*
============
Cmd_AddCommand
============
*/
void Cmd_AddCommand (char *cmd_name, xcommand_t function)
{
	cmd_function_t	*cmd;
	cmd_function_t	*cursor,*prev; //Baker 3.75 - from Fitz johnfitz -- sorted list insert

	if (host_initialized)	// because hunk allocation would get stomped
		Sys_Error ("Cmd_AddCommand after host_initialized");

// fail if the command is a variable name
	if (Cvar_GetStringByName(cmd_name)[0])
	{
		Con_Printf ("Cmd_AddCommand: %s already defined as a var\n", cmd_name);
		return;
	}

// fail if the command already exists
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (COM_StringMatch (cmd_name, cmd->name))
		{
			Con_Printf ("Cmd_AddCommand: %s already defined\n", cmd_name);
			return;
		}
	}

	cmd = Hunk_AllocName (1, sizeof(cmd_function_t), "command");
	cmd->name = cmd_name;
	cmd->function = function;

	//johnfitz -- insert each entry in alphabetical order
    if (cmd_functions == NULL || strcmp(cmd->name, cmd_functions->name) < 0) //insert at front
	{
		cmd->next = cmd_functions;
		cmd_functions = cmd;
	}
    else //insert later
	{
        prev = cmd_functions;
        cursor = cmd_functions->next;
        while ((cursor != NULL) && (strcmp(cmd->name, cursor->name) > 0))
		{
            prev = cursor;
            cursor = cursor->next;
        }
        cmd->next = prev->next;
        prev->next = cmd;
    }
	//johnfitz
}

/*
============
Cmd_FindCommand
============
*/
cmd_function_t *Cmd_FindCommand (const char *cmd_name)
{
	cmd_function_t	*cmd;

	for (cmd = cmd_functions ; cmd ; cmd = cmd->next)
		if (COM_StringMatchCaseless (cmd_name, cmd->name))
			return cmd;

	return NULL;
}

/*
============
Cmd_CompleteCommand
============
*/
char *Cmd_CompleteCommand (const char *partial)
{
	cmd_function_t	*cmd;
	const int		strlen_partial = strlen(partial);

	do
	{
		if (strlen_partial == 0)	// Zero length string
			break;

		// check functions
			for (cmd = cmd_functions ; cmd ; cmd = cmd->next)
				if (COM_StringMatchNCaseless (cmd->name, partial, strlen_partial))
					return cmd->name;
		
		//	for (lcmd = legacycmds ; lcmd ; lcmd = lcmd->next)
		//		if (!strncasecmp(partial, lcmd->oldname, len))
		//			return lcmd->oldname;

	} while (0);

	return NULL;
}

/*
============
Cmd_CompleteCountPossible
============
*/
int Cmd_CompleteCountPossible (char *partial)
{
	cmd_function_t	*cmd;
	const int		strlen_partial		= strlen(partial);
	int				commands_matching 	= 0;

	do
	{
		if (strlen_partial == 0)
			break;

		for (cmd = cmd_functions ; cmd ; cmd = cmd->next)
			if (COM_StringMatchNCaseless (cmd->name, partial, strlen_partial))
				commands_matching++;

	} while (0);

	return commands_matching;
}

//===================================================================

int			RDFlags = 0;
direntry_t	*filelist = NULL;
int			num_files = 0;

static	char	filetype[8] = "file";

static	char	compl_common[MAX_FILELENGTH];
static	int		compl_len;
static	int		compl_clen;

static void FindCommonSubString (const char *s)
{
	if (!compl_clen)
	{
		strlcpy (compl_common, s, sizeof(compl_common));
		compl_clen = strlen (compl_common);
	}
	else
	{
		while (compl_clen > compl_len && strncasecmp(s, compl_common, compl_clen))
			compl_clen--;
	}
}

static void CompareParams (void)
{
	int	i;

	compl_clen = 0;

	for (i=0 ; i<num_files ; i++)
		FindCommonSubString (filelist[i].name);

	if (compl_clen)
		compl_common[compl_clen] = 0;
}

static void PrintEntries (void);
extern	char	key_lines[32][MAXCMDLINE];
extern	int	edit_line;
extern	int	key_linepos;

#define	READDIR_ALL_PATH(p)							\
	for (search = com_searchpaths ; search ; search = search->next)		\
	{									\
		if (!search->pack)						\
		{								\
			RDFlags |= (RD_STRIPEXT | RD_NOERASE);			\
			if (skybox)						\
				RDFlags |= RD_SKYBOX;				\
			ReadDir (va("%s/%s", search->filename, subdir), p);	\
		}								\
	}

/*
============
Cmd_Complete_FileParameter	-- by joe

file parameter completion for various commands
============
*/
void Cmd_Complete_FileParameter (char *partial, const char *attachment)
{
	char		*s, *param, stay[MAX_QPATH], subdir[MAX_QPATH] = "", param2[MAX_QPATH];
	qbool		skybox = false;

	StringLCopy (stay, partial);

// we don't need "<command> + space(s)" included
	param = strrchr (stay, ' ') + 1;
	if (!*param)		// no parameter was written in, so quit
		return;

	compl_len = strlen (param);
	strcat (param, attachment);

	if (COM_StringMatch (attachment, "*.bsp"))
	{
		StringLCopy (subdir, "maps/");
	}
	else if (COM_StringMatch (attachment, "*.tga"))
	{
		if (strstr(stay, "loadsky ") == stay || strstr(stay, "r_skybox ") == stay)
		{
			StringLCopy (subdir, "gfx/env/");
			skybox = true;
		}
		else if (strstr(stay, "loadcharset ") == stay || strstr(stay, "gl_consolefont ") == stay)
		{
			StringLCopy (subdir, "textures/charsets/");
		}
		else if (strstr(stay, "crosshairimage ") == stay)
		{
			StringLCopy (subdir, "crosshairs/");
		}
	}

	if (strstr(stay, "gamedir ") == stay)
	{
		RDFlags |= RD_GAMEDIR;
		ReadDir (com_basedir, param);

		pak_files = 0;	// so that previous pack searches are cleared
	}
	else if (strstr(stay, "load ") == stay || strstr(stay, "printtxt ") == stay) // Baker: this might need adjusted (load)
	{
		RDFlags |= RD_STRIPEXT;
		ReadDir (com_gamedirfull, param);

		pak_files = 0;	// same here
	}
	else
	{
		searchpath_t	*search;

		EraseDirEntries ();
		pak_files = 0;

		READDIR_ALL_PATH(param);
		if (COM_IsExtension(param, ".tga"))
		{
			strlcpy (param2, param, strlen(param)+1-strlen(".tga"));
			strcat (param2, ".pcx");
			READDIR_ALL_PATH(param2);
			QFS_FindFilesInPak (va("%s%s", subdir, param2), compl_len, NULL);
		}
		else if (COM_IsExtension(param, ".dem"))
		{
			strlcpy (param2, param, strlen(param)+1-strlen(".dem"));
			strcat (param2, ".dz");
			READDIR_ALL_PATH(param2);
			QFS_FindFilesInPak (va("%s%s", subdir, param2), compl_len, NULL);
		}
		QFS_FindFilesInPak (va("%s%s", subdir, param), compl_len, NULL);
	}

	if (!filelist)
		return;

	s = strchr (partial, ' ') + 1;
// just made this to avoid printing the filename twice when there's only one match
	if (num_files == 1)
	{
		*s = '\0';
		strcat (partial, filelist[0].name);
		key_linepos = strlen(partial) + 1;
		key_lines[edit_line][key_linepos] = 0;
		return;
	}

	CompareParams ();

	Con_Printf ("]%s\n", partial);
	PrintEntries ();

	*s = '\0';
	strcat (partial, compl_common);
	key_linepos = strlen(partial) + 1;
	key_lines[edit_line][key_linepos] = 0;
}

/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
FIXME: lookupnoadd the token to speed search?
============
*/
void Cmd_ExecuteString (char *text, cmd_source_t src)
{
	cmd_function_t	*cmd;
	cmd_alias_t	*a;

//	Con_Printf("Cmd_ExecuteString: %s \n", text);
	cmd_source = src;
	Cmd_TokenizeString (text);

// execute the command line
	if (!Cmd_Argc())
		return;		// no tokens

// check functions
	for (cmd = cmd_functions ; cmd ; cmd = cmd->next)
	{
		if (COM_StringMatchCaseless (cmd_argv[0], cmd->name))
		{
			cmd->function ();
			return;
		}
	}

// check alias
	for (a = cmd_alias ; a ; a = a->next)
	{
		if (COM_StringMatchCaseless (cmd_argv[0], a->name))
		{
			Cbuf_InsertText (a->value);
			return;
		}
	}

// check cvars
	if (!Cvar_Command ())
	{
	//warncmd: if (developer.value)
		Con_Printf ("Unknown command \"%s\"\n", Cmd_Argv(0));
	}
}


/*
===================
Cmd_ForwardToServer_f

Sends the entire command line over to the server
===================
*/
void Cmd_ForwardToServer_f (void)
{
	//from ProQuake --start
	char *src, *dst, buff[128];			// JPG - used for say/say_team formatting
	int minutes, seconds, match_time;	// JPG - used for %t
	//from ProQuake --end

	if (cls.state != ca_connected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}

	if (cls.demoplayback)
		return; // not really connected

	MSG_WriteByte (&cls.message, clc_stringcmd);

	//----------------------------------------------------------------------
	// JPG - handle say separately for formatting--start
	if ((COM_StringMatchCaseless (Cmd_Argv(0), "say") || COM_StringMatchCaseless (Cmd_Argv(0), "say_team")) && Cmd_Argc() > 1)
	{
		SZ_Print (&cls.message, Cmd_Argv(0));
		SZ_Print (&cls.message, " ");

		src = Cmd_Args();
		dst = buff;
		while (*src && dst - buff < 100)
		{
			if (*src == '%')
			{
				switch (*++src)
				{
				case 'h':
					dst += sprintf(dst, "%d", cl.stats[STAT_HEALTH]);
					break;

				case 'a':
					dst += sprintf(dst, "%d", cl.stats[STAT_ARMOR]);
					break;

				case 'r':
					if (cl.stats[STAT_HEALTH] > 0 && (cl.items & IT_ROCKET_LAUNCHER))
					{
						if (cl.stats[STAT_ROCKETS] < 5)
							dst += sprintf(dst, "%s", msg_needrox.string);
						else
							dst += sprintf(dst, "%s", msg_haverl.string);
					}
					else
						dst += sprintf(dst, "%s", msg_needrl.string);
					break;

				case 'l':
					dst += sprintf(dst, "%s", LOC_GetLocation(cl_entities[cl.player_point_of_view_entity].origin));
					break;

				case 'd':
					dst += sprintf(dst, "%s", LOC_GetLocation(cl.death_location));
					break;

/*				case 'D':
					dst += sprintf(dst, "%s", wih);
					break;    */ // Baker says: Rook must have added a new %D variable

				case 'c':
					dst += sprintf(dst, "%d", cl.stats[STAT_CELLS]);
					break;

				case 'x':
					dst += sprintf(dst, "%d", cl.stats[STAT_ROCKETS]);
					break;

				case 'R':
					if (cl.items & IT_SIGIL1)
					{
						dst += sprintf(dst, "%s", "Resistance");
						break;
					}
					if (cl.items & IT_SIGIL2)
					{
						dst += sprintf(dst, "%s", "Strength");
						break;
					}
					if (cl.items & IT_SIGIL3)
					{
						dst += sprintf(dst, "%s", "Haste");
						break;
					}
					if (cl.items & IT_SIGIL4)
					{
						dst += sprintf(dst, "%s", "Regen");
						break;
					}
					break;

				case 'F':
					if (cl.items & IT_KEY1)
					{
						dst += sprintf(dst, "%s", "Blue Flag");
						break;
					}
					if (cl.items & IT_KEY2)
					{
						dst += sprintf(dst, "%s", "Red Flag");
						break;
					}
					break;

				case 'p':
					if (cl.stats[STAT_HEALTH] > 0)
					{
						if (cl.items & IT_QUAD)
						{
							dst += sprintf(dst, "%s", msg_quad.string);
							if (cl.items & (IT_INVULNERABILITY | IT_INVISIBILITY))
								*dst++ = ',';
						}
						if (cl.items & IT_INVULNERABILITY)
						{
							dst += sprintf(dst, "%s", msg_pent.string);
							if (cl.items & IT_INVISIBILITY)
								*dst++ = ',';
						}
						if (cl.items & IT_INVISIBILITY)
							dst += sprintf(dst, "%s", msg_ring.string);
					}
					break;

				case 'w':	// JPG 3.00
					{
						int first = 1;
						int item;
						char *ch = msg_weapons.string;
						if (cl.stats[STAT_HEALTH] > 0)
						{
							for (item = IT_SUPER_SHOTGUN ; item <= IT_LIGHTNING ; item *= 2)
							{
								if (*ch != ':' && (cl.items & item))
								{
									if (!first)
										*dst++ = ',';
									first = 0;
									while (*ch && *ch != ':')
										*dst++ = *ch++;
								}
								while (*ch && *ch != ':')
									*ch++;
								if (*ch)
									*ch++;
								if (!*ch)
									break;
							}
						}
						if (first)
							dst += sprintf(dst, "%s", msg_noweapons.string);
					}
					break;

				case '%':
					*dst++ = '%';
					break;

				case 't':
					if ((cl.minutes || cl.seconds) && cl.seconds < 128)
					{
						if (cl.match_pause_time)
							match_time = ceil(60.0 * cl.minutes + cl.seconds - (cl.match_pause_time - cl.last_match_time));
						else
							match_time = ceil(60.0 * cl.minutes + cl.seconds - (cl.time - cl.last_match_time));
						minutes = match_time / 60;
						seconds = match_time - 60 * minutes;
					}
					else
					{
						minutes = cl.time / 60;
						seconds = cl.time - 60 * minutes;
						minutes &= 511;
					}
					dst += sprintf(dst, "%d:%02d", minutes, seconds);
					break;

				default:
					*dst++ = '%';
					*dst++ = *src;
					break;
				}
				if (*src)
					src++;
			}
			else
				*dst++ = *src++;
		}
		*dst = 0;

		SZ_Print (&cls.message, buff);
		return;
	}
	// JPG - handle say separately for formatting--end
	//----------------------------------------------------------------------

	if (COM_StringNOTMatchCaseless(Cmd_Argv(0), "cmd"))
	{
		SZ_Print (&cls.message, Cmd_Argv(0));
		SZ_Print (&cls.message, " ");
	}
	if (Cmd_Argc() > 1)
		SZ_Print (&cls.message, Cmd_Args());
	else
		SZ_Print (&cls.message, "\n");
}

/*
================
CmdOld_CheckParm

Returns the position (1 to argc-1) in the command's argument list
where the given parameter apears, or 0 if not present
================
*/
/*
int CmdOld_CheckParm (char *parm)
{
	int	i;

	if (!parm)
		Sys_Error ("Cmd_CheckParmMod: NULL");

	for (i=1 ; i<Cmd_Argc() ; i++)
		if (COM_StringMatchCaseless (parm, Cmd_Argv(i)))
			return i;

	return 0;
}
*/

/*
====================
Cmd_CmdList_f

List all console commands
====================
*/
void Cmd_CmdList_f (void)
{
	cmd_function_t	*cmd;
	int		counter;

	if (cmd_source != src_command)
		return;

	for (counter = 0, cmd = cmd_functions ; cmd ; cmd = cmd->next, counter++)
		Con_Printf ("%s\n", cmd->name);

	Con_Printf ("------------\n%d commands\n", counter);
}

/*
====================
Cmd_Dir_f

List all files in the mod's directory	-- by joe
====================
*/
void Cmd_Dir_f (void)
{
	char			myarg[MAX_FILELENGTH];

	if (cmd_source != src_command)
		return;

	if (COM_StringMatch (Cmd_Argv(1), ""))
	{
		StringLCopy (myarg, "*");
		StringLCopy (filetype, "file");
	}
	else
	{
		StringLCopy (myarg, Cmd_Argv(1));
		// first two are exceptional cases
		if (strstr(myarg, "*"))
			StringLCopy (filetype, "file");
		else if (strstr(myarg, "*.dem"))
			StringLCopy (filetype, "demo");
		else
		{
			if (strchr(myarg, '.'))
			{
				StringLCopy (filetype, StringTemp_FileExtension(myarg));
				filetype[strlen(filetype)] = 0x60;	// right-shadowed apostrophe
			}
			else
			{
				strcat (myarg, "*");
				StringLCopy (filetype, "file");
			}
		}
	}

	RDFlags |= RD_COMPLAIN;
	ReadDir (com_gamedirfull, myarg);
	if (!filelist)
		return;

	Con_Printf ("\x02" "%ss in current folder are:\n", filetype);
	PrintEntries ();
}


static void sAddNewEntry (char *fname, int ftype, long fsize)
{
	int	i, pos;

	filelist = Q_realloc (filelist, (num_files + 1) * sizeof(direntry_t), "Find files");

#ifdef _WIN32 // Operating system case sensitivity

	if (COM_StringNOTMatchCaseless(fname, "Error reading directory") !=0)
		COMD_toLower (fname);
	// else don't convert, linux is case sensitive
#endif

	// inclusion sort
	for (i=0 ; i<num_files ; i++)
	{
		if (ftype < filelist[i].type)
			continue;
		else if (ftype > filelist[i].type)
			break;

		if (strcmp(fname, filelist[i].name) < 0)
			break;
	}
	pos = i;
	for (i=num_files ; i>pos ; i--)
		filelist[i] = filelist[i-1];

    filelist[i].name = Q_strdup (fname, "Filelist entry name");
	filelist[i].type = ftype;
	filelist[i].size = fsize;

	num_files++;
}



void EraseDirEntries (void)
{
	// Baker: Shouldn't it iterate through and free filelist[i].name ?  I mean it is strdup allocated.
	if (filelist)
	{
		Q_realloc_free (filelist);
		filelist = NULL;
		num_files = 0;
	}
}

qbool CheckEntryName (char *ename)
{
	int	i;

	for (i=0 ; i<num_files ; i++)
		if (COM_StringMatchCaseless (ename, filelist[i].name))
			return true;

	return false;
}


/*
=================
ReadDir
=================
*/
void ReadDir (char *path, char *the_arg)
{
#ifdef _WIN32 // file system handle and find file structures
	HANDLE		h;
	WIN32_FIND_DATA	fd;
#else
	int		h, i = 0;
	char		*foundname;
	glob_t		fd;
	struct	stat	fileinfo;
#endif

	if (path[strlen(path)-1] == '/')
		path[strlen(path)-1] = 0;

	if (!(RDFlags & RD_NOERASE))
		EraseDirEntries ();

#ifdef _WIN32 // file system handle and find file structures
	h = FindFirstFile (va("%s/%s", path, the_arg), &fd);
	if (h == INVALID_HANDLE_VALUE)
#else
	h = glob (va("%s/%s", path, the_arg), 0, NULL, &fd);
	if (h == GLOB_ABORTED)
#endif
	{
		if (RDFlags & RD_MENU_DEMOS)
		{
			sAddNewEntry ("Error reading directory", 3, 0);
			num_files = 1;
		}
		else if (RDFlags & RD_COMPLAIN)
		{
			Con_Printf ("No such file\n");
		}
		goto end;
	}

	if (RDFlags & RD_MENU_DEMOS && !(RDFlags & RD_MENU_DEMOS_MAIN))
	{
		sAddNewEntry ("..", 2, 0);
		num_files = 1;
	}

	do {
		int	fdtype;
		long	fdsize;
		char	filename[MAX_FILELENGTH];

#ifdef _WIN32 // file system handle and find file structures
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!(RDFlags & (RD_MENU_DEMOS | RD_GAMEDIR)) || COM_StringMatch (fd.cFileName, ".") || COM_StringMatch (fd.cFileName, ".."))
				continue;

			fdtype = 1;
			fdsize = 0;
			strlcpy (filename, fd.cFileName, sizeof(filename));
		}
		else
		{
			char	ext[8];

			if (RDFlags & RD_GAMEDIR)
				continue;

			strlcpy (ext, StringTemp_FileExtension(fd.cFileName), sizeof(ext));

			if (RDFlags & RD_MENU_DEMOS && COM_StringNOTMatchCaseless (ext, "dem") && COM_StringNOTMatchCaseless (ext, "dz"))
				continue;

			fdtype = 0;
			fdsize = fd.nFileSizeLow;
			if (COM_StringNOTMatchCaseless (ext, "dz") && RDFlags & (RD_STRIPEXT | RD_MENU_DEMOS))
			{
				COM_Copy_StripExtension (fd.cFileName, filename);
				if (RDFlags & RD_SKYBOX)
				{
					int	idx = strlen(filename)-3;

					filename[filename[idx] == '_' ? idx : idx+1] = 0;	// cut off skybox_ext
				}
			}
			else
			{
				strlcpy (filename, fd.cFileName, sizeof(filename));
			}

			if (CheckEntryName(filename))
				continue;	// file already on list
		}
#else
		if (h == GLOB_NOMATCH || !fd.gl_pathc)
			break;

		SLASHJMP(foundname, fd.gl_pathv[i]);
		stat (fd.gl_pathv[i], &fileinfo);

		if (S_ISDIR(fileinfo.st_mode))
		{
			if (!(RDFlags & (RD_MENU_DEMOS | RD_GAMEDIR)))
				continue;

			fdtype = 1;
			fdsize = 0;
			strlcpy (filename, foundname, sizeof(filename));
		}
		else
		{
			char	ext[8];

			if (RDFlags & RD_GAMEDIR)
				continue;

			strlcpy (ext, StringTemp_FileExtension(foundname), sizeof(ext));

			if (RDFlags & RD_MENU_DEMOS && COM_StringNOTMatchCaseless (ext, "dem") && COM_StringNOTMatchCaseless (ext, "dz"))
				continue;

			fdtype = 0;
			fdsize = fileinfo.st_size;
			if (COM_StringNOTMatchCaseless (ext, "dz") && RDFlags & (RD_STRIPEXT | RD_MENU_DEMOS))
			{
				COM_Copy_StripExtension (foundname, filename);
				if (RDFlags & RD_SKYBOX)
				{
					int	idx = strlen(filename)-3;

					filename[filename[idx] == '_' ? idx : idx+1] = 0;	// cut off skybox_ext
				}
			}
			else
			{
				strlcpy (filename, foundname, sizeof(filename));
			}

			if (CheckEntryName(filename))
				continue;	// file already on list
		}
#endif
		sAddNewEntry (filename, fdtype, fdsize);
	}
#ifdef _WIN32 // file system handle and find file structures
	while (FindNextFile(h, &fd));
	FindClose (h);
#else
	while (++i < fd.gl_pathc);
	globfree (&fd);
#endif

	if (!num_files)
	{
		if (RDFlags & RD_MENU_DEMOS)
		{
			sAddNewEntry ("[ no files ]", 3, 0);
			num_files = 1;
		}
		else if (RDFlags & RD_COMPLAIN)
		{
			Con_Printf ("No such file\n");
		}
	}

end:
	RDFlags = 0;
}


int	pak_files = 0;

//extern	int	message_cursor_column;

static void PrintEntries (void)
{
	int	i, filectr;

	filectr = pak_files ? (num_files - pak_files) : 0;

	for (i=0 ; i<num_files ; i++)
	{
		if (!filectr-- && pak_files)
		{
			if (Con_GetCursorColumn())
			{
				Con_Printf ("\n");
				Con_Printf ("\x02" "inside pack file:\n");
			}
			else
			{
				Con_Printf ("\x02" "inside pack file:\n");
			}
		}
		Con_PrintColumnItem (filelist[i].name);
	}

	if (Con_GetCursorColumn())
		Con_Printf ("\n");
}

/*
==================
Cmd_Complete_Command_Or_Cvar

Advanced command completion
==================
*/
// Baker: Needs to complete after semicolons
// Baker: Needs to complete aliases  if "bind" or "unbind" and is 3nd param
// Baker: Needs to complete keynames if "bind" or "unbind" and is 2nd param
// Baker: Needs to autocomplete to the first match always
// Baker: Needs to TAB complete cycle through the human typed part
// Baker: If I type MAP<space> and press TAB, I want a list of maps.
/*
We need a structure for current line

completeable part _ the effective beginning of the current parameter.  Stuff before this is not in play
user_cursorposition the last place a typed character took place

But this isn't what I want to do right now.

Tough turkeys.  Is a project goal and you are warmed up.  Finish and be proud.
*/



void Cmd_Complete_Command_Or_Cvar (void)
{
	int			num_command_completion_matches, num_variable_completion_matches;
	char		*current_line_string = key_lines[edit_line] + 1;   // First character of a line is "[" so advance +1 from beginning
	const char *cmd;

	if (!(compl_len = strlen(current_line_string)))		// Complete length is strlen of the current line, which if that is 0 return.
		return;
	compl_clen = 0;						// Set the complete_clen to 0

	num_command_completion_matches = Cmd_CompleteCountPossible (current_line_string);
	num_variable_completion_matches = Cvar_CompleteCountPossible (current_line_string);

	if (num_command_completion_matches + num_variable_completion_matches > 1)
	{
		Con_Printf ("\n");

		if (num_command_completion_matches)
		{
			cmd_function_t	*cmd;

			Con_Printf ("\x02" "commands:\n");
			// check commands
			for (cmd = cmd_functions ; cmd ; cmd = cmd->next)
			{
				if (!strncasecmp(current_line_string, cmd->name, compl_len))
				{
					Con_PrintColumnItem (cmd->name);
					FindCommonSubString (cmd->name);
				}
			}

			if (Con_GetCursorColumn())
				Con_Printf ("\n");
		}

		if (num_variable_completion_matches)
		{
			cvar_t		*var;

			Con_Printf ("\x02" "variables:\n");
			// check variables
			for (var = cvar_vars ; var ; var = var->next)
			{
				if (!strncasecmp(current_line_string, var->cvarname, compl_len))
				{
					Con_PrintColumnItem (var->cvarname);
					FindCommonSubString (var->cvarname);
				}
			}

			if (Con_GetCursorColumn())
				Con_Printf ("\n");
		}
	}

	if (num_command_completion_matches + num_variable_completion_matches == 1)		// If only a single match, complete the whole thing. (Baker: Don't like because it stops you. )
	{
		if (!(cmd = Cmd_CompleteCommand(current_line_string)))
			cmd = Cvar_CompleteVariable (current_line_string);
	}
	else if (compl_clen)	// Fills it out to the common match
	{
		compl_common[compl_clen] = 0;
		cmd = compl_common;
	}
	else
		return;

	strcpy (key_lines[edit_line]+1, cmd);	// Put the command in after the "]"
	key_linepos = strlen(cmd) + 1;			// Move the cursor beyond the text for partial
	if (num_command_completion_matches + num_variable_completion_matches == 1)
		key_lines[edit_line][key_linepos++] = ' ';	// If it completed a whole command, move the cursor beyond that
	key_lines[edit_line][key_linepos] = 0;
}

/*
====================
Cmd_DemDir_f

List all demo files
====================
*/
void Cmd_DemDir_f (void)
{
	char	myarg[MAX_FILELENGTH];

	if (cmd_source != src_command)
		return;

	if (COM_StringMatch (Cmd_Argv(1), ""))
	{
		StringLCopy (myarg, "*.dem");
	}
	else
	{
		StringLCopy (myarg, Cmd_Argv(1));
		if (strchr(myarg, '.'))
		{
			Con_Printf ("You needn`t use dots in demdir parameters\n");
			if (COM_StringNOTMatch(StringTemp_FileExtension(myarg), "dem"))
			{
				Con_Printf ("demdir is for demo files only\n");
				return;
			}
		}
		else
		{
			strcat (myarg, "*.dem");
		}
	}

	StringLCopy (filetype, "demo");

	RDFlags |= (RD_STRIPEXT | RD_COMPLAIN);
	ReadDir (com_gamedirfull, myarg);
	if (!filelist)
		return;

	Con_Printf ("\x02" "%ss in current folder are:\n", filetype);
	PrintEntries ();
}

/*
====================
AddTabs

Replaces nasty tab character with spaces
====================
*/
static void AddTabs (char *buf)
{
	unsigned char	*s, tmp[256];
	int		i;

	for (s = buf, i = 0 ; *s ; s++, i++)
	{
		switch (*s)
		{
		case 0xb4:
		case 0x27:
			*s = 0x60;
			break;

		case '\t':
			strcpy (tmp, s + 1);
			while (i++ < 8)
				*s++ = ' ';
			*s-- = '\0';
			strcat (buf, tmp);
			break;
		}

		if (i >= 7)
			i = -1;
	}
}

/*
====================
Cmd_PrintTxt_f

Prints a text file into the console
====================
*/
void Cmd_PrintTxt_f (void)
{
	char	name[MAX_FILELENGTH], buf[256] = {0};
	FILE	*f;
	char	com_gamedirfull[MAX_OSPATH];

	if (cmd_source != src_command)
		return;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("printtxt <txtfile> : prints a text file\n");
		return;
	}

	StringLCopy (name, Cmd_Argv(1));

	COMD_DefaultExtension (name, ".txt");

	snprintf (buf, sizeof(buf), "%s/%s", com_gamedirfull, name);
	if (!(f = FS_fopen_read(buf, "rt")))
	{
		Con_Printf ("ERROR: couldn't open %s\n", name);
		return;
	}

	Con_Printf ("\n");
	while (fgets(buf, 256, f))
	{
		AddTabs (buf);
		Con_Printf ("%s", buf);
		memset (buf, 0, sizeof(buf));
	}

	Con_Printf ("\n\n");
	fclose (f);
}

extern void Hunk_Print (qbool all);
void Cmd_PrintHunk_f (void)
{
	Hunk_Print (true);
}

// Baker: We get here after default.cfg is executed because
//        I check for that filename in exec
//        If we get here, and game_initialized == false.  We are done reading defaults.

//        If we get here, and game_initialized == true.  Some other mechanism brought us here.
//        Possibly sources:  someone type "exec default.cfg" in console.
//                           someone did old style reset to defaults
//                           a gamedir change and executing quake.rc


void Cmd_HintDefaultsDone_f (void)
{
//	if (game_initialized == false)
//	{
		// Close the gate.  We are now into config.cfg
		// But first ... fix some bad stuff ...

		// These are user preferences .. we do have the option of just disallowing these to be set during defaulting?
		if (!isDedicated)	Cvar_SetDefaultFloatByRef (&vid_brightness_gamma, 50);	// This isn't a gamedir setting
		if (!isDedicated)	Cvar_SetDefaultFloatByRef (&snd_volume, 0.2);		// Let them turn it up, don't make it so damn loud

		Con_DevPrintf (DEV_CVAR, "default.cfg complete.  Default values no longer accepted.\n");
		accepts_default_values = false;
//	}
}

void Cmd_HintGameInitialized_f (void)
{
	float elapsed = Sys_DoubleTime() - session_startup_time;

	game_in_initialization = false;

	if (!isDedicated)
	{

		Cbuf_AddText ("\nsavefov\n");
//		Cbuf_AddText ("savesensitivity\n");
		Con_DevPrintf (DEV_CVAR, "Game initialized.  Saved fov and sensitivity values as default for the user.\n");

	}

	if (session_in_firsttime_startup)
	{
		Con_Printf ("Benchmarking: Engine startup %3.3f seconds\n", elapsed);
		session_in_firsttime_startup = false;
	}

}



/*
============
Cmd_Init
============
*/
void Cmd_Init (void)
{
// register our commands
#if SUPPORTS_STUFFCMDS
	Cmd_AddCommand ("stuffcmds", Cmd_StuffCmds_f);
#endif
	Cmd_AddCommand ("exec", Cmd_Exec_f);
	Cmd_AddCommand ("echo", Cmd_Echo_f);
	Cmd_AddCommand ("alias", Cmd_Alias_f);
	Cmd_AddCommand ("cmd", Cmd_ForwardToServer_f);

	Cmd_AddCommand ("hint_defaults_done", Cmd_HintDefaultsDone_f);
	Cmd_AddCommand ("hint_gameinitialized", Cmd_HintGameInitialized_f);

	Cmd_AddCommand ("wait", Cmd_Wait_f);
	Cmd_AddCommand ("unalias", Cmd_Unalias_f); //johnfitz
	Cmd_AddCommand ("unaliasall", Cmd_Unaliasall_f); //johnfitz


	Cmd_AddCommand ("dir", Cmd_Dir_f);
	Cmd_AddCommand ("demdir", Cmd_DemDir_f);
	Cmd_AddCommand ("printtxt", Cmd_PrintTxt_f);
	Cmd_AddCommand ("printhunk", Cmd_PrintHunk_f);//R00k
	Cmd_AddCommand ("cmdlist", Cmd_CmdList_f);
}
