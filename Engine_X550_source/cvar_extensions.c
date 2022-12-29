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
// cvar_extensions.c -- Expanded cvar functionalities

#include "quakedef.h"

// Intended to be off and on ... therefore integer
void Cvar_Toggle_f (void)
{
	cvar_t *var;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("Usage: %s <cvar>\n", Cmd_Argv (0));
		Con_Printf ("       Toggle a cvar on/off\n");
		return;
	}

	var = Cvar_FindVarByName (Cmd_Argv(1));
	if (!var)
	{
		Con_Printf ("Unknown variable \"%s\"\n", Cmd_Argv(1));
		return;
	}
	Cvar_SetStringByRef (var, var->integer ? "0" : "1");
}

/*
============
Cvar_Cycle_f -- johnfitz
============
*/
void Cvar_Cycle_f (void)
{
	cvar_t *var;

	int i;

	if (Cmd_Argc() < 3)
	{
		Con_Printf("cycle <cvar> <value list>: cycle cvar through a list of values\n");
		return;
	}

	//loop through the args until you find one that matches the current cvar value.
	//yes, this will get stuck on a list that contains the same value twice.
	//it's not worth dealing with, and i'm not even sure it can be dealt with.

	var = Cvar_FindVarByName (Cmd_Argv(1));
	if (!var)
	{
		Con_Printf ("Unknown variable \"%s\"\n", Cmd_Argv(1));
		return;
	}

	for (i=2;i<Cmd_Argc();i++)
	{
		//zero is assumed to be a string, even though it could actually be zero.  The worst case
		//is that the first time you call this command, it won't match on zero when it should, but after that,
		//it will be comparing strings that all had the same source (the user) so it will work.
		if (atof(Cmd_Argv(i)) == 0)
		{
			if (COM_StringMatch (Cmd_Argv(i), Cvar_GetStringByName(Cmd_Argv(1))))
				break;
		}
		else
		{
			if (atof(Cmd_Argv(i)) == Cvar_GetFloatByName(Cmd_Argv(1)))
				break;
		}
	}

	if (i == Cmd_Argc())
		Cvar_SetStringByRef (var, Cmd_Argv(2)); // no match
	else if (i + 1 == Cmd_Argc())
		Cvar_SetStringByRef (var, Cmd_Argv(2)); // matched last value in list
	else
		Cvar_SetStringByRef (var, Cmd_Argv(i+1)); // matched earlier in list
}

void Cvar_Inc_f (void)
{
	int c;
	cvar_t *var;
	float delta;

	c = Cmd_Argc();
	if (c != 2 && c != 3)
	{
		Con_Printf ("inc <cvar> [value]\n");
		return;
	}

	var = Cvar_FindVarByName (Cmd_Argv(1));
	if (!var)
	{
		Con_Printf ("Unknown variable \"%s\"\n", Cmd_Argv(1));
		return;
	}
	delta = (c == 3) ? atof (Cmd_Argv(2)) : 1;

	Cvar_SetFloatByRef (var, var->floater + delta);
}

//int Cvar_CvarCompare (const void *p1, const void *p2)
//{
//    return strcmp((*((cvar_t **) p1))->cvarname, (*((cvar_t **) p2))->cvarname);
//}

#if 0
cvar_t *Cvar_Create (char *name, char *string, int cvarflags)
{
	cvar_t	*v;

	if ((v = Cvar_FindVarByName(name)))	// Cvar already exists
		return v;

	v = (cvar_t *)Z_Malloc (sizeof(cvar_t));
	// Cvar doesn't exist, so we create it
	v->next = cvar_vars;
	cvar_vars = v;

	v->name = Z_Strdup (name);
	v->string = Z_Strdup (string);
	v->default_string = Z_Strdup (string);
	v->flags = cvarflags;
	v->floater = atof (v->string);
	v->integer = (int)v->floater;
	return v;
}
#endif

#if 0
// Baker: Neat ... but we aren't going to do it ...
//returns true if the cvar was found (and deleted)
qbool Cvar_DeleteByName (const char *name)
{
	cvar_t	*var, *prev;

	var = Cvar_FindVarByName(name);

	if (!var)
		return false;

	prev = NULL;
	for (var = cvar_vars ; var ; var = var->next)
	{
		if (COM_StringMatchCaseless (var->cvarname, name))
		{
			// unlink from cvar list
			if (prev)
				prev->next = var->next;
			else
				cvar_vars = var->next;

			// free
			Z_Free (var->default_string);
			Z_Free (var->string);
			Z_Free (var->cvarname);
			Z_Free (var);
			return true;
		}
		prev = var;
	}

//	assert(!"Cvar list broken");
	return false;
}
#endif

static qbool cvar_seta = false;


#if 0
void Cvar_Set_f (void)
{
	cvar_t	*var;
	char	*var_name;

	if (Cmd_Argc() != 3)
	{
		Con_Printf ("Usage: %s <cvar> <value>\n", Cmd_Argv(0));
		return;
	}

	var_name = Cmd_Argv (1);
	var = Cvar_FindVarByName (var_name);

	if (var)
	{
		Cvar_SetStringByRef (var, Cmd_Argv(2));
	}
	else
	{
		if (Cmd_FindCommand(var_name))
		{
			Con_Printf ("\"%s\" is a command\n", var_name);
			return;
		}
		var = Cvar_Create (var_name, Cmd_Argv(2), CVAR_USER_CREATED);
	}

/* Baker says: next time!
	if (cvar_seta)
		var->flags |= CVAR_USER_ARCHIVE;
*/
}
#endif

#if 0
void Cvar_Seta_f (void)
{
	cvar_seta = true;
	Cvar_Set_f ();
	cvar_seta = false;
}
#endif


char *external_settings;


qbool Cvar_GetExternal (cvar_t *var)
{
	int i;

	if (!external_settings || !var)		return false;

	// Baker: Protect against unregistered cvars being manipulated.  Like conditional ones like vid_vsync that don't always exist.
	if (!var->default_string)
	{
		Con_Warning ("%s: Variable %s not registered.\n", "Cvar_SetStringByRef", var->cvarname);
		return false;
	}

	Cmd_TokenizeString(external_settings);

	if ( (i = Cmd_CheckParmMod(var->cvarname))!=-1 && i + 1 < Cmd_Argc())
	{
		Cvar_SetStringByRef(var, Cmd_Argv(i+1));
		return true;
	}

	return false;
}