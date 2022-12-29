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
// cvar_listings.c -- Cvar list, autocomplete and such

#include "quakedef.h"


// Used by void Cmd_ExecuteString
qbool Cvar_Command (void)
{

	cvar_t	*var;

// check variables
	if (!(var = Cvar_FindVarByName(Cmd_Argv(0))))
		return false;

// perform a variable print or set
	if (Cmd_Argc() == 1)
		Con_Printf ("\"%s\" is:\"%s\" default:\"%s\"\n", var->cvarname, var->string, var->default_string);
	else
		Cvar_SetStringByRef (var, Cmd_Argv(1));

	return true;
}



const char *Cvar_CompleteVariable (const char *partial)
{
	cvar_t	*cvar;
	int	len;

	len = strlen(partial);

	if (!len)
		return NULL;

	// check exact match
	for (cvar = cvar_vars; cvar; cvar = cvar->next)
		if (COM_StringMatchCaseless (partial,cvar->cvarname))
			return cvar->cvarname;

	// check partial match
	for (cvar = cvar_vars ; cvar ; cvar = cvar->next)
		if (!strncasecmp(partial, cvar->cvarname, len))
			return cvar->cvarname;

	return NULL;
}

int Cvar_CompleteCountPossible (const char *partial)
{
	cvar_t	*cvar;
	int		strlen_partial = strlen(partial), 
			cvars_matching = 0;

	do
	{
		if (strlen_partial == 0)	
			break;	// cvars_matching = 0

		// check partial match
		for (cvar = cvar_vars ; cvar ; cvar = cvar->next)
			if (!strncasecmp(cvar->cvarname, partial, strlen_partial))
				cvars_matching++;

	} while (0);

	return cvars_matching;
}

//Writes lines containing "set variable value" for all variables with the archive flag set to true.
// Dedicated server doesn't do this (confirmed)
void Cvar_WriteVariables (FILE *f, const int SaveType)
{
	cvar_t	*var;

	if (SaveType == CVAR_ARCHIVE)
		fprintf (f, "\n// Variables\n\n");


	for (var = cvar_vars ; var ; var = var->next)
	{
		if (SaveType == CVAR_EXTERNAL)
		{
			if (var->flags & CVAR_EXTERNAL)	// Baker:  The following is very sensitive to space due to how I read the cvars with COM_Parse
				fprintf (f, "%s \"%s\"\n", var->cvarname, var->string);	// Baker: newline delimited for legibility; convert to spaces onload
		}
		else
		{	// CVAR_ARCHIVE
			if (var->flags & CVAR_ARCHIVE)
				fprintf (f, "%s \"%s\"\n", var->cvarname, var->string);
			// don't save read-only or initial cvars, nor cl_warncmd (it has different role)
//			else if ((var->flags & CVAR_ROM) || (var->flags & CVAR_INIT) || COM_StringMatch (var->cvarname, "cl_warncmd"))
//				continue;
//			else if ((session_savevars.integer == 2 && strcmp(var->string, var->default_string)) || session_savevars.integer == 3)
//				fprintf (f, "%s \"%s\"\n", var->cvarname, var->string);
			// cvar_savevars 2 = anything that is changed.  cvar_savevars 3 = anything at all
		}
	}
}

void Cvar_CvarList_f (void)
{
	cvar_t	*var;
	qbool	basic = false;
	qbool   changed_only = false;
	int	count;

	if (cmd_source != src_command)		// why?
		return;

	if (Cmd_Argc () == 2 && (COM_StringMatchCaseless (Cmd_Argv(1), "basic")))
		basic = true;
	else if (Cmd_Argc () == 2 && (COM_StringMatchCaseless (Cmd_Argv(1), "changed")))
		changed_only = true;

	Con_PrintColumnItem ("\nVariable");
	if (!basic)
	{
		Con_PrintColumnItem ("Value");
		Con_PrintColumnItem ("Default");
	}
	Con_Printf ("\n\n");

	for (var = cvar_vars, count = 0; var ; var = var->next, count++)
	{
		if (basic)
		{
			Con_Printf ("%s\n", var->cvarname);
		}
		else
		{
			// Detailed list
			qbool isDefault = COM_StringMatch (var->string, var->default_string);

			if (isDefault && changed_only)
				continue;

			Con_PrintColumnItem_MaxWidth (var->cvarname);
			Con_PrintColumnItem_MaxWidth (var->string);
			if (COM_StringMatch (var->string, var->default_string))
					Con_PrintColumnItem ("[unchanged]");
			else
				Con_PrintColumnItem_MaxWidth (var->default_string);

			Con_Printf ("\n");
		}
	}

	Con_Printf ("\n-------------\n%d variables\n", count);
	if (!basic && !changed_only)
	{
		Con_Printf ("note: \"cvarlist basic\"   shows only list\n", count);
		Con_Printf ("      \"cvarlist changed\" shows only changed\n", count);
	}

}

