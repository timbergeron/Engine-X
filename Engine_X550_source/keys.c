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
#include "quakedef.h"

/*

key up events are sent even if in console mode

*/


#define		MAXCMDLINE	256
#define		CMDLINES	64

char		key_lines[CMDLINES][MAXCMDLINE];
int		key_linepos;
int		key_lastpress;

int		edit_line = 0;
int		history_line = 0;

keydest_t	key_dest;
int			key_special_dest = false;

int		key_insert = 1;
int		key_count;		// incremented every key event

char		*keybindings[256];
qbool	consolekeys[256];	// if true, can't be rebound while in console
qbool	menubound[256];		// if true, can't be rebound while in menu
int		keyshift[256];		// key to map to if shift held down in console
int		key_repeats[256];	// if > 1, it is autorepeating
qbool	keydown[256];
qbool	keygamedown[256];  // Baker: to prevent -aliases from triggering



typedef struct
{
	char	*name;
	int	keynum;
} keyname_t;

keyname_t keynames[] =
{
	{"TAB", K_TAB},
	{"ENTER", K_ENTER},
	{"ESCAPE", K_ESCAPE},
	{"SPACE", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},

	{"CAPSLOCK", K_CAPSLOCK},
	{"PRINTSCR", K_PRINTSCR},
	{"SCRLCK", K_SCRLCK},
	{"SCROLLOCK", K_SCRLCK},
	{"PAUSE", K_PAUSE},

	{"UPARROW", K_UPARROW},
	{"DOWNARROW", K_DOWNARROW},
	{"LEFTARROW", K_LEFTARROW},
	{"RIGHTARROW", K_RIGHTARROW},

	{"ALT", K_ALT},
#if 0
	{"LALT", K_LALT},
	{"RALT", K_RALT},
#endif
	{"CTRL", K_CTRL},
#if 0
	{"LCTRL", K_LCTRL},
	{"RCTRL", K_RCTRL},
#endif
	{"SHIFT", K_SHIFT},
#if 0
	{"LSHIFT", K_LSHIFT},
	{"RSHIFT", K_RSHIFT},
#endif

#if 0
	{"WINKEY", K_WIN},
#endif

#ifdef MACOSX
	{"LCOMMAND", K_LWIN},
	{"RCOMMAND", K_RWIN},
#endif
	{"POPUPMENU", K_MENU},

	// keypad keys

	{"NUMLOCK", KP_NUMLOCK},
	{"KP_NUMLCK", KP_NUMLOCK},
	{"KP_NUMLOCK", KP_NUMLOCK},
	{"KP_SLASH", KP_SLASH},
	{"KP_DIVIDE", KP_SLASH},
	{"KP_STAR", KP_STAR},
	{"KP_MULTIPLY", KP_STAR},

	{"KP_MINUS", KP_MINUS},

	{"KP_HOME", KP_HOME},
	{"KP_7", KP_HOME},
	{"KP_UPARROW", KP_UPARROW},
	{"KP_8", KP_UPARROW},
	{"KP_PGUP", KP_PGUP},
	{"KP_9", KP_PGUP},
	{"KP_PLUS", KP_PLUS},

	{"KP_LEFTARROW", KP_LEFTARROW},
	{"KP_4", KP_LEFTARROW},
	{"KP_5", KP_5},
	{"KP_RIGHTARROW", KP_RIGHTARROW},
	{"KP_6", KP_RIGHTARROW},

	{"KP_END", KP_END},
	{"KP_1", KP_END},
	{"KP_DOWNARROW", KP_DOWNARROW},
	{"KP_2", KP_DOWNARROW},
	{"KP_PGDN", KP_PGDN},
	{"KP_3", KP_PGDN},

	{"KP_INS", KP_INS},
	{"KP_0", KP_INS},
	{"KP_DEL", KP_DEL},
	{"KP_ENTER", KP_ENTER},

	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INS", K_INS},
	{"DEL", K_DEL},
	{"PGDN", K_PGDN},
	{"PGUP", K_PGUP},
	{"HOME", K_HOME},
	{"END", K_END},

	{"PAUSE", K_PAUSE},

	{"MWHEELUP", K_MWHEELUP},
	{"MWHEELDOWN", K_MWHEELDOWN},
	{"MOUSE1", K_MOUSE1},
	{"MOUSE2", K_MOUSE2},
	{"MOUSE3", K_MOUSE3},
	{"MOUSE4", K_MOUSE4},
	{"MOUSE5", K_MOUSE5},
	{"MOUSE6", K_MOUSE6},
	{"MOUSE7", K_MOUSE7},
	{"MOUSE8", K_MOUSE8},

	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},

	{"AUX1", K_AUX1},
	{"AUX2", K_AUX2},
	{"AUX3", K_AUX3},
	{"AUX4", K_AUX4},
	{"AUX5", K_AUX5},
	{"AUX6", K_AUX6},
	{"AUX7", K_AUX7},
	{"AUX8", K_AUX8},
	{"AUX9", K_AUX9},
	{"AUX10", K_AUX10},
	{"AUX11", K_AUX11},
	{"AUX12", K_AUX12},
	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},
	{"AUX17", K_AUX17},
	{"AUX18", K_AUX18},
	{"AUX19", K_AUX19},
	{"AUX20", K_AUX20},
	{"AUX21", K_AUX21},
	{"AUX22", K_AUX22},
	{"AUX23", K_AUX23},
	{"AUX24", K_AUX24},
	{"AUX25", K_AUX25},
	{"AUX26", K_AUX26},
	{"AUX27", K_AUX27},
	{"AUX28", K_AUX28},
	{"AUX29", K_AUX29},
	{"AUX30", K_AUX30},
	{"AUX31", K_AUX31},
	{"AUX32", K_AUX32},


	{"SEMICOLON", ';'},	// because a raw semicolon separates commands
	{"TILDE", '~'},
	{"BACKQUOTE", '`'},
	{"QUOTE", '"'},
	{"APOSTROPHE", '\''},
	{NULL,0}
};

/*
==============================================================================

			LINE TYPING INTO THE CONSOLE

==============================================================================
*/

/*	Baker: Lovely ... another unused function.
qbool CheckForCommand (void)
{
	char	*s, command[256];

	StringLCopy (command, key_lines[edit_line] + 1);
	for (s = command ; *s > ' ' ; s++)
		;
	*s = 0;

	return (Cvar_FindVarByName(command) || Cmd_FindCommand(command) || Cmd_FindAlias(command));
}
*/

static void AdjustConsoleHeight (const int delta)
{
	int		height;

	if (!cl.worldmodel || cls.signon != SIGNONS)
		return;
	height = (scr_con_size.floater * vid.height + delta + 5) / 10;
	height *= 10;
	if (delta < 0 && height < 30)
		height = 30;
	if (delta > 0 && height > vid.height - 10)
		height = vid.height - 10;
	Cvar_SetFloatByRef (&scr_con_size, (float)height / vid.height);
}

void Key_Extra (int *key)
{
	if (keydown[K_CTRL])
	{
		if (*key >= '0' && *key <= '9')
		{
			*key = *key - '0' + 0x12;	// yellow number
		}
		else
		{
			switch (*key)
			{
			case '[': *key = 0x10; break;
			case ']': *key = 0x11; break;
			case 'g': *key = 0x86; break;
			case 'r': *key = 0x87; break;
			case 'y': *key = 0x88; break;
			case 'b': *key = 0x89; break;
			case '(': *key = 0x80; break;
			case '=': *key = 0x81; break;
			case ')': *key = 0x82; break;
			case 'a': *key = 0x83; break;
			case '<': *key = 0x1d; break;
			case '-': *key = 0x1e; break;
			case '>': *key = 0x1f; break;
			case ',': *key = 0x1c; break;
			case '.': *key = 0x9c; break;
			case 'B': *key = 0x8b; break;
			case 'C': *key = 0x8d; break;
			}
		}
	}

	if (keydown[K_ALT])
		*key |= 0x80;		// red char
}

/*
====================
Key_Console

Interactive line editing and console scrollback
====================
*/

extern	int	rows_into_backscroll;
//extern	int	rows_available;		// added by joe


void Key_Console (int key, int ascii)
{
	int	i;
	char	*cmd;
	ascii = key; // Baker 3.88: Fix for if international keyboard mapping is off?

	switch (key)
	{
		case K_ENTER:
			Cbuf_AddText (key_lines[edit_line]+1);	// skip the "]"
			Cbuf_AddText ("\n");
			Con_Printf ("%s\n", key_lines[edit_line]);
		// joe: don't save same commands multiple times
			if (COM_StringNOTMatch (key_lines[edit_line-1], key_lines[edit_line]))
				edit_line = (edit_line + 1) & (CMDLINES -1 ); // 64 - 1 = 63;
			history_line = edit_line;
			key_lines[edit_line][0] = ']';
			key_lines[edit_line][1] = 0;
			key_linepos = 1;
			if (cls.state == ca_disconnected)
				SCR_UpdateScreen ();	// force an update, because the command
							// may take some time
			return;

		case K_TAB:
			// various parameter completions -- by joe
			cmd = key_lines[edit_line]+1;
			if (strstr(cmd, "playdemo ") == cmd || strstr(cmd, "capture_start ") == cmd ||
			    strstr(cmd, "capturedemo ") == cmd)
				Cmd_Complete_FileParameter (cmd, "*.dem");
			else if (strstr(cmd, "printtxt ") == cmd)
				Cmd_Complete_FileParameter (cmd, "*.txt");
			else if (strstr(cmd, "map ") == cmd || strstr(cmd, "changelevel "))
				Cmd_Complete_FileParameter (cmd, "*.bsp");
			else if (strstr(cmd, "exec ") == cmd)
				Cmd_Complete_FileParameter (cmd, "*.cfg");
			else if (strstr(cmd, "load ") == cmd)
				Cmd_Complete_FileParameter (cmd, "*.sav");
			else if (strstr(cmd, "loadsky ") == cmd || strstr(cmd, "r_skybox ") == cmd ||
				 strstr(cmd, "loadcharset ") == cmd || strstr(cmd, "gl_consolefont ") == cmd ||
				 strstr(cmd, "crosshairimage ") == cmd)
				Cmd_Complete_FileParameter (cmd, "*.tga");
			else if (strstr(cmd, "gamedir ") == cmd)
				Cmd_Complete_FileParameter (cmd, "*");
			else
			{	// command completion
				Cmd_Complete_Command_Or_Cvar ();
			}
			return;

		case K_BACKSPACE:
			if (key_linepos > 1)
			{
				strcpy (key_lines[edit_line] + key_linepos - 1, key_lines[edit_line] + key_linepos);
				key_linepos--;
			}
			return;

		case K_DEL:
			if (key_linepos < strlen(key_lines[edit_line]))
				strcpy (key_lines[edit_line] + key_linepos, key_lines[edit_line] + key_linepos + 1);
			return;

		case K_LEFTARROW:
			if (keydown[K_CTRL])
			{
				// word left
				while (key_linepos > 1 && key_lines[edit_line][key_linepos-1] == ' ')
					key_linepos--;
				while (key_linepos > 1 && key_lines[edit_line][key_linepos-1] != ' ')
					key_linepos--;
				return;
			}
			if (key_linepos > 1)
				key_linepos--;
			return;

		case K_RIGHTARROW:
			if (keydown[K_CTRL])
			{
				// word right
				i = strlen (key_lines[edit_line]);
				while (key_linepos < i && key_lines[edit_line][key_linepos] != ' ')
					key_linepos++;
				while (key_linepos < i && key_lines[edit_line][key_linepos] == ' ')
					key_linepos++;
				return;
			}
			if (key_linepos < strlen(key_lines[edit_line]))
				key_linepos++;
			return;

		case K_INS:
			key_insert ^= 1;
			return;

		case K_UPARROW:
			if (keydown[K_CTRL])
			{
				AdjustConsoleHeight (-10);
				return;
			}

			do 
			{
				history_line = (history_line - 1) & (CMDLINES -1 ); // 64 - 1 = 63;
			} while (history_line != edit_line && !key_lines[history_line][1]);

			if (history_line == edit_line)
				history_line = (edit_line + 1) & (CMDLINES -1 ); // 64 - 1 = 63;

			strcpy (key_lines[edit_line], key_lines[history_line]);
			key_linepos = strlen (key_lines[edit_line]);
			return;

		case K_DOWNARROW:
			if (keydown[K_CTRL])
			{
				AdjustConsoleHeight (10);
				return;
			}
			if (history_line == edit_line)
				return;
			do {
				history_line = (history_line + 1) & (CMDLINES -1 ); // 64 - 1 = 63;
			} while (history_line != edit_line && !key_lines[history_line][1]);

			if (history_line == edit_line)
			{
				key_lines[edit_line][0] = ']';
				key_lines[edit_line][1] = 0;
				key_linepos = 1;
			}
			else
			{
				strcpy (key_lines[edit_line], key_lines[history_line]);
				key_linepos = strlen (key_lines[edit_line]);
			}
			return;

		case K_PGUP:
		case K_MWHEELUP:
			if (keydown[K_CTRL] && key == K_PGUP)
				rows_into_backscroll += ((int)scr_conlines-16)>>3;
			else
				rows_into_backscroll += 2;
			if (rows_into_backscroll > Con_GetMaximumBackscroll ())
				rows_into_backscroll = Con_GetMaximumBackscroll ();
			return;

		case K_PGDN:
		case K_MWHEELDOWN:
			if (keydown[K_CTRL] && key == K_PGDN)
				rows_into_backscroll -= ((int)scr_conlines-16)>>3;
			else
				rows_into_backscroll -= 2;
			if (rows_into_backscroll < 0)
				rows_into_backscroll = 0;
			if (rows_into_backscroll > Con_GetMaximumBackscroll ())
				rows_into_backscroll = Con_GetMaximumBackscroll ();
			return;

		case K_HOME:
			if (keydown[K_CTRL])
				rows_into_backscroll = Con_GetMaximumBackscroll();
			else
				key_linepos = 1;
			return;

		case K_END:
			if (keydown[K_CTRL])
				rows_into_backscroll = 0;
			else
				key_linepos = strlen (key_lines[edit_line]);
			return;
	}

	if ((key == 'V' || key == 'v') && keydown[K_CTRL])
	{
		char	*cliptext;
		int	len;

		if ((cliptext = Sys_GetClipboardData()))
		{
			len = strlen (cliptext);
			if (len + strlen(key_lines[edit_line]) > MAXCMDLINE - 1)
				len = MAXCMDLINE - 1 - strlen(key_lines[edit_line]);
			if (len > 0)
			{	// insert the string
				memmove (key_lines[edit_line] + key_linepos + len, key_lines[edit_line] + key_linepos, strlen(key_lines[edit_line]) - key_linepos + 1);
				memcpy (key_lines[edit_line] + key_linepos, cliptext, len);
				key_linepos += len;
			}
		}
		return;
	}

	if (key < 32 || key > 127)
		return;	// non printable

	Key_Extra (&key);

	i = strlen (key_lines[edit_line]);
	if (i >= MAXCMDLINE-1)
		return;

	// This also moves the ending \0
	memmove (key_lines[edit_line]+key_linepos+1, key_lines[edit_line]+key_linepos, i-key_linepos+1);
	key_lines[edit_line][key_linepos] = key;
	key_linepos++;
}

//============================================================================

// JPG - added MAX_CHAT_SIZE
#define	MAX_CHAT_SIZE	45
char		chat_buffer[MAX_CHAT_SIZE];
qbool	team_message = false;

void Key_Message (int key, int ascii)
{
	static	int	chat_bufferlen = 0;

	if (key == K_ENTER)
	{
		if (team_message)
			Cbuf_AddText ("say_team \"");
		else
			Cbuf_AddText ("say \"");
		Cbuf_AddText (chat_buffer);
		Cbuf_AddText ("\"\n");

		key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

	if (key == K_ESCAPE)
	{
		key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

	if (key < 32 || key > 127)
		return;	// non printable

	if (key == K_BACKSPACE)
	{
		if (chat_bufferlen)
		{
			chat_bufferlen--;
			chat_buffer[chat_bufferlen] = 0;
		}
		return;
	}

	if (chat_bufferlen == MAX_CHAT_SIZE - (team_message ? 3 : 1))  // JPG - maximize message length
		return;	// all full

	chat_buffer[chat_bufferlen++] = key;
	chat_buffer[chat_bufferlen] = 0;
}

//============================================================================


/*
===================
Key_StringToKeynum

Returns a key number to be used to index keybindings[] by looking at
the given string. Single ascii characters return themselves, while
the K_* names are matched up.
===================
*/
int Key_StringToKeynum (char *str)
{
	keyname_t	*kn;

	if (!str || !str[0])
		return -1;

	if (!str[1])
#if !defined(FLASH)
		return tolower(str[0]);
#else
		return str[0]; // Fix me: use COM_Tolower or whatever from LordHavoc
#endif

	for (kn = keynames ; kn->name ; kn++)
	{
		if (COM_StringMatchCaseless (str, kn->name))
			return kn->keynum;
	}
	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, or a K_* name) for the
given keynum.
FIXME: handle quote special (general escape sequence?)
===================
*/
char *Key_KeynumToString (int keynum)
{
	keyname_t	*kn;
	static	char	tinystr[2];

	if (keynum == -1)
		return "<KEY NOT FOUND>";

	if (keynum > 32 && keynum < 127)
	{	// printable ascii
		tinystr[0] = keynum;
		tinystr[1] = 0;
		return tinystr;
	}

	for (kn = keynames ; kn->name ; kn++)
		if (keynum == kn->keynum)
			return kn->name;

	return "<UNKNOWN KEYNUM>";
}


/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding (int keynum, char *binding)
{
	if (keynum == -1)
		return;

#if 0
	if (keynum == K_CTRL || keynum == K_ALT || keynum == K_SHIFT || keynum == K_WIN)
	{

		Key_SetBinding (keynum + 1, binding);
		Key_SetBinding (keynum + 2, binding);
		return;
	}
#endif

// free old bindings
	if (keybindings[keynum])
	{
		Z_Free (keybindings[keynum]);
		keybindings[keynum] = NULL;
	}

// allocate memory for new binding
	keybindings[keynum] = Z_Strdup (binding);
}

/*
===================
Key_Unbind
===================
*/
void Key_Unbind (int keynum)
{
	if (keynum == -1)
		return;
#if 0
	if (keynum == K_CTRL || keynum == K_ALT || keynum == K_SHIFT || keynum == K_WIN)
	{

		Key_Unbind (keynum + 1);
		Key_Unbind (keynum + 2);
		return;
	}
#endif
	if (keybindings[keynum])
	{
		Z_Free (keybindings[keynum]);
		keybindings[keynum] = NULL;
	}
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f (void)
{
	int	b;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("Usage:  %s <key> : remove commands from a key\n", Cmd_Argv(0));
		return;
	}

	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b == -1)
	{
		Con_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	Key_Unbind (b);
}

void Key_Unbindall_f (void)
{
	int	i;

	for (i=0 ; i<256 ; i++)
		if (keybindings[i])
			Key_Unbind (i);
}

static void Key_PrintBindInfo (int keynum, char *keyname)
{
	if (!keyname)
		keyname = Key_KeynumToString (keynum);

	if (keynum == -1)
	{
		Con_Printf ("\"%s\" isn't a valid key\n", keyname);
		return;
	}

	if (keybindings[keynum])
		Con_Printf ("\"%s\" = \"%s\"\n", keyname, keybindings[keynum]);
	else
		Con_Printf ("\"%s\" is not bound\n", keyname);
}

/*
============
Key_Bindlist_f -- johnfitz
============
*/
void Key_Bindlist_f (void)
{
	int		i, count;

	count = 0;
	for (i=0 ; i<256 ; i++)
		if (keybindings[i])
			if (*keybindings[i])
			{
				Con_Printf ("   %s \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
				count++;
			}
	Con_Printf ("%i bindings\n", count);
}

/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f (void)
{
	int	i, c, b;
	char	cmd[1024];

	c = Cmd_Argc();

	if (c != 2 && c != 3)
	{
		Con_Printf ("bind <key> [command] : attach a command to a key\n");
		return;
	}
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b == -1)
	{
		Con_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	if (c == 2)
	{
#if 0
		if ((b == K_CTRL || b == K_ALT || b == K_SHIFT || b == K_WIN) && (keybindings[b+1] || keybindings[b+2]))
		{
			if (keybindings[b+1] && keybindings[b+2] && COM_StringMatch (keybindings[b+1], keybindings[b+2]))
			{
				Con_Printf ("\"%s\" = \"%s\"\n", Cmd_Argv(1), keybindings[b+1]);
			}
			else
			{
				Key_PrintBindInfo (b + 1, NULL);
				Key_PrintBindInfo (b + 2, NULL);
			}

		}
		else
#endif
		{
			// and the following should print "ctrl (etc) is not bound" since K_CTRL cannot be bound
			Key_PrintBindInfo (b, Cmd_Argv(1));
		}
		return;
	}

// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for (i=2 ; i<c ; i++)
	{
		if (i > 2)
			strcat (cmd, " ");
		strcat (cmd, Cmd_Argv(i));
	}

	Key_SetBinding (b, cmd);
}

/*
============
Key_WriteBindings

Writes lines containing "bind key value"
============
*/
void Key_WriteBindings (FILE *f)
{
	int	i;

	fprintf (f, "\n// Key bindings\n\n");
	for (i=0 ; i<256 ; i++)
		if (keybindings[i])
			if (*keybindings[i])
				fprintf (f, "bind \"%s\" \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
}


// Added by VVD {
void History_Init (void)
{
	int i, c;
	FILE *hf;

	for (i = 0; i < CMDLINES; i++)
	{
		key_lines[i][0] = ']';
		key_lines[i][1] = 0;
	}
	key_linepos = 1;

//	if (cl_savehistory.integer)
		if ((hf = FS_fopen_read(HISTORY_FILE_NAME, "rt")))
		{
			do
			{
				i = 1;
				do
				{
					c = fgetc(hf);
					key_lines[edit_line][i++] = c;
				} while (c != '\n' && c != EOF && i < MAXCMDLINE);
				key_lines[edit_line][i - 1] = 0;
				edit_line = (edit_line + 1) & (CMDLINES - 1);
			} while (c != EOF && edit_line < CMDLINES);
			fclose(hf);

			history_line = edit_line = (edit_line - 1) & (CMDLINES - 1);
			key_lines[edit_line][0] = ']';
			key_lines[edit_line][1] = 0;
		}
}

void History_Shutdown (void)
{
	int i;
	FILE *hf;

//	if (cl_savehistory.integer)
		if ((hf = FS_fopen_write(HISTORY_FILE_NAME, "wt", 0 /* do not create path */)))
		{
			i = edit_line;
			do
			{
				i = (i + 1) & (CMDLINES - 1);
			} while (i != edit_line && !key_lines[i][1]);

			do
			{
				// fprintf(hf, "%s\n", wcs2str(key_lines[i] + 1));
				fprintf(hf, "%s\n", key_lines[i] + 1);
				i = (i + 1) & (CMDLINES - 1);
			} while (i != edit_line && key_lines[i][1]);
			fclose(hf);
		}
}
// } Added by VVD
/*
===================
Key_Init
===================
*/
void Key_Init (void)
{
	int		i;

	History_Init ();

#if 0
	for (i=0 ; i<32 ; i++)
		{
			key_lines[i][0] = ']';
			key_lines[i][1] = 0;
		}
		key_linepos = 1;
#endif

// init ascii characters in console mode
	for (i=32 ; i<128 ; i++)
		consolekeys[i] = true;
	consolekeys[K_ENTER] = true;
	consolekeys[K_TAB] = true;
	consolekeys[K_LEFTARROW] = true;
	consolekeys[K_RIGHTARROW] = true;
	consolekeys[K_UPARROW] = true;
	consolekeys[K_DOWNARROW] = true;
	consolekeys[K_BACKSPACE] = true;
	consolekeys[K_INS] = true;
	consolekeys[K_DEL] = true;
	consolekeys[K_HOME] = true;
	consolekeys[K_END] = true;
	consolekeys[K_PGUP] = true;
	consolekeys[K_PGDN] = true;
#if 0
	if (!COM_CheckParm("-oldkeys"))
	{

		consolekeys[K_ALT] = true;
		consolekeys[K_LALT] = true;
		consolekeys[K_RALT] = true;
		consolekeys[K_CTRL] = true;
		consolekeys[K_LCTRL] = true;
		consolekeys[K_RCTRL] = true;
	}
#endif
	consolekeys[K_SHIFT] = true;
#if 0
	consolekeys[K_LSHIFT] = true;
	consolekeys[K_RSHIFT] = true;
#endif
	consolekeys[K_MWHEELUP] = true;
	consolekeys[K_MWHEELDOWN] = true;
	consolekeys[K_MWHEELUP] = true;
	consolekeys[K_MWHEELDOWN] = true;
	consolekeys['`'] = false;
	consolekeys['~'] = false;

	consolekeys[KP_SLASH]	 = true;
	consolekeys[KP_STAR] = true;
	consolekeys[KP_MINUS] = true;
	consolekeys[KP_HOME] = true;
	consolekeys[KP_UPARROW] = true;
	consolekeys[KP_PGUP] = true;
	consolekeys[KP_PLUS] = true;
	consolekeys[KP_LEFTARROW] = true;
	consolekeys[KP_5] = true;
	consolekeys[KP_RIGHTARROW] = true;
	consolekeys[KP_END] = true;
	consolekeys[KP_DOWNARROW] = true;
	consolekeys[KP_PGDN]	 = true;
	consolekeys[KP_ENTER] = true;
	consolekeys[KP_INS] = true;
	consolekeys[KP_DEL] = true;


	for (i=0 ; i<256 ; i++)
		keyshift[i] = i;
	for (i='a' ; i<='z' ; i++)
		keyshift[i] = i - 'a' + 'A';
	keyshift['1'] = '!';
	keyshift['2'] = '@';
	keyshift['3'] = '#';
	keyshift['4'] = '$';
	keyshift['5'] = '%';
	keyshift['6'] = '^';
	keyshift['7'] = '&';
	keyshift['8'] = '*';
	keyshift['9'] = '(';
	keyshift['0'] = ')';
	keyshift['-'] = '_';
	keyshift['='] = '+';
	keyshift[','] = '<';
	keyshift['.'] = '>';
	keyshift['/'] = '?';
	keyshift[';'] = ':';
	keyshift['\''] = '"';
	keyshift['['] = '{';
	keyshift[']'] = '}';
	keyshift['`'] = '~';
	keyshift['\\'] = '|';

	menubound[K_ESCAPE] = true;
	for (i=0 ; i<12 ; i++)
		menubound[K_F1+i] = true;

// register our functions

	Cvar_Registration_Host_Keys ();

	Cmd_AddCommand ("bindlist",Key_Bindlist_f); //johnfitz
	Cmd_AddCommand ("bind", Key_Bind_f);
	Cmd_AddCommand ("unbind", Key_Unbind_f);
	Cmd_AddCommand ("unbindall", Key_Unbindall_f);
}

qbool Key_isSpecial (int key)
{
	if (key == K_INS || key == K_DEL || key == K_HOME ||
	    key == K_END || key == K_ALT || key == K_CTRL)
		return true;

	return false;
}

qbool Key_DemoKey (int key)
{
	switch (key)
	{
	case K_LEFTARROW:
		Cvar_SetFloatByRef (&demorewind, 1);
		// If we are showing FPS, show demo speed info
		return true;

	case K_RIGHTARROW:
		Cvar_SetFloatByRef (&demorewind, 0);
		// If we are showing FPS, show demo speed info
		return true;

	case K_UPARROW:
		Cvar_SetFloatByRef (&demospeed, demospeed.floater + 0.1);
		// If we are showing FPS, show demo speed info
		return true;

	case K_DOWNARROW:
		Cvar_SetFloatByRef (&demospeed, demospeed.floater - 0.1);
		// If we are showing FPS, show demo speed info
		return true;

	case K_ENTER:
		Cvar_SetFloatByRef (&demorewind, 0);
		Cvar_SetFloatByRef (&demospeed, 1);
		// If we are showing FPS, show demo speed info
		return true;
	}

	return false;
}

/*
===================
Key_Event

Called by the system between frames for both key up and key down events
Should NOT be called during an interrupt!
===================
*/
extern qbool	cl_inconsole; // Baker 3.76 - from Qrack
void Key_Event (int key, int ascii, qbool down)
{
	char	*kb;
	char    cmd[1024];
	qbool wasgamekey = false;
	ascii = key;

	// Baker: special ... K_MOUSECLICK
	if (key >= K_MOUSECLICK_BUTTON1 && key <= K_MOUSECLICK_BUTTON5)
	{
		if (key_special_dest == 1) // Name maker
		{
			M_Keydown (K_MOUSECLICK_BUTTON1, 0, down);  // down will = true
		}
		else if (key_special_dest == 2) // Customize Controls
		{
			if (key == K_MOUSECLICK_BUTTON1)
			{
				M_Keydown (K_MOUSE1, 0, down);
			}
			else if (key == K_MOUSECLICK_BUTTON2)
				M_Keydown (K_MOUSE2, 0, down);
			else if (key == K_MOUSECLICK_BUTTON3)
				M_Keydown (K_MOUSE3, 0, down);
			else if  (key == K_MOUSECLICK_BUTTON4)
				M_Keydown (K_MOUSE4, 0, down);
			else if  (key == K_MOUSECLICK_BUTTON5)
				M_Keydown (K_MOUSE5, 0, down);
		}
		return; // Get outta here
	}


#if SUPPORTS_GLVIDEO_MODESWITCH
	if (key == K_ENTER  && !down && keydown[K_ALT] && vid_altenter_toggle.integer)
	{
		float old_altenter = vid_altenter_toggle.integer;
		vid_altenter_toggle.integer = 0;		// To prevent recursion		// CVAR ROGUE

		if (VID_WindowedSwapAvailable())
		{ // We can switch to/from Windowed mode?
			if (VID_isFullscreen())

				VID_Windowed();
			else
				VID_Fullscreen();
		}
		else
		{
			// Let someone know this isn't available
			if (vid_isLocked)
				Con_Printf ("Video mode switch locked due to command line\n");
			else // some other reason like bpp ...
				Con_Printf("Switching between windowed/fullscreen mode is locked\n");
		}

		// BAKER: Do NOT get out.  If we do, the keystates are cleared, but the key is still pressed
		//        Which can result in unpressable keys and related problems.
		vid_altenter_toggle.integer = old_altenter;	// CVAR ROGUE
	}
#endif

#if 0
// Baker: is there a risk we are double triggering with this????
	if (key == K_LALT || key == K_RALT)
		Key_Event (K_ALT, 0, down);
	else if (key == K_LCTRL || key == K_RCTRL)
		Key_Event (K_CTRL, 0, down);
	else if (key == K_LSHIFT || key == K_RSHIFT)
		Key_Event (K_SHIFT, 0, down);
	else if (key == K_LWIN || key == K_RWIN)
		Key_Event (K_WIN, 0, down);
#endif

	keydown[key] = down;


	// Baker: the problem with this is some maybe a keydown is getting
	// ignored sometimes?
	wasgamekey = keygamedown[key]; // Baker: to prevent -aliases being triggered in-console needlessly
	if (!down)
	{
		keygamedown[key] = false; // We can always set keygamedown to false if key is released
	}
	// Baker: the only situation this doesn't address is how in
	// Windowed mode we stop receiving mouse event if console is up
	// because the mouse is freed.  Technically we should clear the mouse
	// state if this occurs.  But this is minor, so maybe next time.
	// For example, if you press fire with mouse1 in Windowed mode and then
	// pull the console down, the mouse control is released to Windows
	// and we no longer get mouse events, so the gun will be perpetually
	// firing.


	if (!down)
		key_repeats[key] = 0;

	key_lastpress = key;
	key_count++;
	if (key_count <= 0)
	{
		return;		// just catching keys for Con_NotifyBox
	}

// update auto-repeat status
	if (down)
	{
		key_repeats[key]++;
		if (key_repeats[key] > 1)	// joe: modified to work as ZQuake
		{
			if ((key != K_BACKSPACE && key != K_DEL && key != K_LEFTARROW &&
			     key != K_RIGHTARROW && key != K_UPARROW && key != K_DOWNARROW &&
			     key != K_PGUP && key != K_PGDN && (key < 32 || key > 126 || key == '`')) ||
			    (key_dest == key_game && cls.state == ca_connected))
				return;	// ignore most autorepeats
		}

// Baker: to raise awareness of binding extra mouse buttons
		if (key >= K_MOUSE5 && key <= K_MOUSE8 && !keybindings[key])
			Con_Printf ("%s is unbound, hit F4 to set.\n", Key_KeynumToString(key));
	}

// handle escape specialy, so the user can never unbind it
	if (key == K_ESCAPE)
	{
		if (!down)
			return;
		switch (key_dest)
		{
			case key_message:
				Key_Message (key, ascii);
				break;

			case key_menu:
				M_Keydown (key, ascii, down);
				break;

			case key_game:
			case key_console:
				M_ToggleMenu_f ();
				break;

			default:
				Sys_Error ("Bad key_dest");
		}
		return;
	}

// key up events only generate commands if the game key binding is
// a button command (leading + sign). These will occur even in console mode,
// to keep the character from continuing an action started before a console
// switch. Button commands include the kenum as a parameter, so multiple
// downs can be matched with ups
	if (!down)
	{
		// Baker: we only want to trigger -alias if appropriate
		//        but we ALWAYS want to exit is key is up
		if (wasgamekey)
		{
			kb = keybindings[key];
			if (kb && kb[0] == '+')
			{
				snprintf (cmd, sizeof(cmd), "-%s %i\n", kb+1, key);
				Cbuf_AddText (cmd);
			}
			if (keyshift[key] != key)
			{
				kb = keybindings[keyshift[key]];
				if (kb && kb[0] == '+')
				{
					snprintf (cmd, sizeof(cmd), "-%s %i\n", kb+1, key);
					Cbuf_AddText (cmd);
				}
			}
		}
		return;
	}

// if not a consolekey, send to the interpreter no matter what mode is
	if ((key_dest == key_menu && menubound[key]) ||
	    (key_dest == key_console && !consolekeys[key]) ||
	    (key_dest == key_game && (!con_forcedup || !consolekeys[key])))// ||
//	    (Key_isSpecial(key) && keybindings[key] != NULL))
	{
		if (cls.demoplayback &&  cl_inconsole == false && Key_DemoKey(key))
			return;
		if ((kb = keybindings[key]))
		{
			// Baker: if we are here, the key is down
			//        and if it is retrigger a bind
			//        it must be allowed to trigger the -bind
			//
			keygamedown[key]=true; // Let it be untriggered anytime

			if (kb[0] == '+')
			{	// button commands add keynum as a parm
				snprintf (cmd, sizeof(cmd), "%s %i\n", kb, key);
				Cbuf_AddText (cmd);
			}
			else
			{
				Cbuf_AddText (kb);
				Cbuf_AddText ("\n");
			}
		}
		return;
	}

	// Baker: I think this next line is unreachable!
//	if (!down)
//		return;		// other systems only care about key down events

	if (keydown[K_SHIFT])
		key = keyshift[key];

	switch (key_dest)
	{
		case key_message:
			Key_Message (key, ascii);
			break;

		case key_menu:
			M_Keydown (key, ascii, down);
			break;

		case key_game:
		case key_console:
			Key_Console (key, ascii);
			break;

		default:
			Sys_Error ("Bad key_dest");
	}
}


/*
================
Key_ClearAllStates - Baker 3.71 - this should be here
================
*/
void Key_ClearAllStates (void)
{
	int		i;

// Baker 3.99n: We need the binds cleared particularly for ALT-ENTER
// send an up event for each key, to make sure the server clears them all
	for (i=0 ; i<256 ; i++)
	{
		// If the key is down, trigger the up action if, say, +showscores or another +bind is activated
		if (keydown[i])
			Key_Event (i, 0, false);
	} //  Baker 3.99n: Restored this!  (Baker 3.71 -- DP doesn~t do this)
#ifndef MACOSX //
 	IN_ClearStates ();  //Baker 3.99n: Restored this! (Baker 3.71 - DP doesn~t do this)
#endif
#pragma message ("OSX Deal with IN_ClearStates somehwow")

	// Baker 3.87: Clear the shift/ctrl/alt status as well!
	//shift_down=false;
	//ctrl_down=false;
	//alt_down=false;
}

