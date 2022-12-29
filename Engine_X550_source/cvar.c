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
// cvar.c -- dynamic variable tracking

#include "quakedef.h"

cvar_t	*cvar_vars;



// Baker: I am going to assume that for this to return successfully the cvar must be registered
//        and it does look that way.  So no need to worry about an unregistered cvar being deleted.
cvar_t *Cvar_FindVarByName (const char *var_name)
{
	cvar_t	*var;

	for (var = cvar_vars ; var ; var = var->next)
		if (COM_StringMatchCaseless (var_name, var->cvarname))
			return var;

	return NULL;
}



void Cvar_ResetVarByRef (cvar_t *var, const qbool bHardReset)
{

	if (!var)	return;

	// Baker: Protect against unregistered cvars being manipulated.  Like conditional ones like vid_vsync that don't always exist.
	if (!var->default_string)	{ Con_Warning ("%s: Variable %s not session_registered.\n", "Cvar_ResetVarByRef", var->cvarname); return; }

	// Baker:  We don't do a reset of ROM vars.  I'm not sure why.  What is our purpose of CVAR ROMS?
	if (bHardReset)
	{
		if (var->flags & CVAR_EXTERNAL)		// Externals do not reset
			return;
		if (var->flags & CVAR_CMDLINE)			// Commandline and such do not reset
			return;
		if (var->flags & CVAR_ROM)
			Con_Printf ("CVAR_ROM encountered: %s during engine reset\n", var->cvarname);

		Cvar_SetDefaultStringByRef (var, var->engine_defaultstring);
	}
	else if (COM_StringNOTMatch(var->string, var->default_string)) // Ok ... if string doesn't match, set the value
		Cvar_SetStringByRef (var, var->default_string);
}

void Cvar_Reset_f (void)
{
	cvar_t	*var;
	char	*s;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("Usage: %s <variable>\n", Cmd_Argv(0));
		return;
	}

	s = Cmd_Argv(1);

	if (var = Cvar_FindVarByName(s))
		Cvar_ResetVarByRef (var, false /* not hard engine default reset*/);
	else
		Con_Printf ("%s : No variable with name %s\n", Cmd_Argv(0), s);
}


/*
============
Cvar_ResetAll_f -- johnfitz
============
*/
void Cvar_ResetAll_f (void)
{
	cvar_t	*var;

	for (var = cvar_vars; var; var = var->next)
	{
		if (var->flags & CVAR_CMDLINE)
		{
			Con_DevPrintf (DEV_CVAR, "Skipping %s reset because it is an initialization cvar\n", var->cvarname);
			continue;
		}

		Cvar_ResetVarByRef (var, false /* not hard engine default reset*/);
	}
}


void Cvar_EngineResetAll_f (void)
{
	cvar_t	*var;

	for (var = cvar_vars; var; var = var->next)
	{
		if (var->flags & CVAR_CMDLINE)
		{
			Con_DevPrintf (DEV_CVAR, "Skipping %s reset because it is an initialization cvar\n", var->cvarname);
			continue;
		}

		Cvar_ResetVarByRef (var, true /* hard reset */);
	}
}


void Cvar_SetDefaultFloatByRef (cvar_t *var, float value)
{
	char	val[128];
	int	i;

	if (!var)	return;

	// Baker: Protect against unregistered cvars being manipulated.  Like conditional ones like vid_vsync that don't always exist.
	if (!var->default_string)	{ Con_Warning ("%s: Variable %s not session_registered.\n", "Cvar_SetDefaultFloatByRef", var->cvarname); return; }

	snprintf (val, sizeof(val), "%f", value);
	for (i = strlen(val) - 1 ; i > 0 && val[i] == '0' ; i--)	// Strip off ending zeros
		val[i] = 0;
	if (val[i] == '.')											// Strip off ending period
		val[i] = 0;

	Z_Free (var->default_string);
	var->default_string = Z_Strdup (val);

//	Z_FREE_AND_Z_STRDUP (var->default_string, val);

	Cvar_SetStringByRef (var, val);
}


void Cvar_SetDefaultStringByRef (cvar_t *var, const char *new_default)
{
	if (!var)	return;

	// Baker: Protect against unregistered cvars being manipulated.  Like conditional ones like vid_vsync that don't always exist.
	if (!var->default_string)	{ Con_Warning ("%s: Variable %s not session_registered.\n", "Cvar_SetDefaultFloatByRef", var->cvarname); return; }

	Z_Free (var->default_string);
	var->default_string = Z_Strdup (new_default);

//	Z_FREE_AND_Z_STRDUP	(var->default_string, new_default);

	if (Cvar_SetStringByRef (var, new_default))
		Con_DevPrintf (DEV_CVAR, "New default string set for variable \"%s\" = \"%s\"\n", var->cvarname, var->string);
	else
		Con_DevPrintf (DEV_CVAR, "New default string set for variable \"%s\" REJECTED\n", var->cvarname);
}

// QuakeC uses this and for some reason I have the D3D wrapper use this so I don't have to extern a ton of cvar stuff for it.
// Cvar cycle uses this too
float Cvar_GetFloatByName (const char *var_name)
{
	cvar_t	*var;

	var = Cvar_FindVarByName (var_name);
	if (!var)
		return 0;
	return atof (var->string);
}

char *Cvar_GetStringByName (const char *var_name)
 {
	cvar_t	*var;

	var = Cvar_FindVarByName (var_name);
	if (!var)
		return "";
	return var->string;
}


void Cvar_Init (void)
{

	Cmd_AddCommand ("cvarlist", Cvar_CvarList_f);
	Cmd_AddCommand ("toggle", Cvar_Toggle_f);
	Cmd_AddCommand ("cycle", Cvar_Cycle_f);
//	Cmd_AddCommand ("set", Cvar_CvarLock_f);
//	Cmd_AddCommand ("seta", Cvar_Seta_f);
	Cmd_AddCommand ("inc", Cvar_Inc_f);

	Cmd_AddCommand ("cvar_reset", Cvar_Reset_f);
	Cmd_AddCommand ("cvar_resetall", Cvar_ResetAll_f);
	Cmd_AddCommand ("cvar_engineresetall", Cvar_EngineResetAll_f);

	if (isDedicated)
		Cmd_AddCommand ("cvar_lock", Cvar_CvarLock_f);

//	Cvar_RegisterVariable (&cvar_viewdefault, NULL);
//	Cvar_RegisterVariable (&session_savevars, NULL);

}

/*
Adds a freestanding variable to the variable list.

If the variable already exists, the value will not be set
The flags will be or'ed in if the variable exists.
*/
void Cvar_RegisterVariable (cvar_t *var,  void *function)
{
//	char	string[512];
	cvar_t	*old;
	cvar_t	*cursor,*prev; //johnfitz -- sorted list insert

// first check to see if it has already been defined
	old = Cvar_FindVarByName (var->cvarname);
	if (old /*&& !(old->flags & CVAR_USER_CREATED)*/)
	{
		Con_Printf ("Can't register variable %s, already defined\n", var->cvarname);
		return;
	}

// check for overlap with a command
	if (Cmd_FindCommand(var->cvarname))
	{
		Con_Printf ("Cvar_RegisterVariable: %s is a command\n", var->cvarname);
		return;
	}

//	var->engine_string =	Baker: Shouldn't this be what it already is ?
	var->default_string = Z_Strdup (var->engine_defaultstring);

#if 0
	if (old)
	{
		var->flags |= old->flags & ~CVAR_USER_CREATED;
		StringLCopy (string, old->string, sizeof(string));
		Cvar_DeleteByName (old->name);
		if (!(var->flags & CVAR_ROM))
			var->string = Z_Strdup (string);
		else
			var->string = Z_Strdup (var->string);
	}
	else
#endif
		var->string = Z_Strdup (var->engine_defaultstring);  // allocate the string on zone because future sets will Z_Free it

	var->floater = atof (var->string);
	var->integer = (int)var->floater;
// link the variable in alphabetically

	//johnfitz -- insert each entry in alphabetical order
    if (cvar_vars == NULL || strcmp(var->cvarname, cvar_vars->cvarname) < 0) //insert at front
	{
		var->next = cvar_vars;
		cvar_vars = var;
	}
    else //insert later
	{
        prev = cvar_vars;
        cursor = cvar_vars->next;
        while (cursor && (strcmp(var->cvarname, cursor->cvarname) > 0))
		{
            prev = cursor;
            cursor = cursor->next;
        }
        var->next = prev->next;
        prev->next = var;
    }
	//johnfitz
}