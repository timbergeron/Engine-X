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
// cvar_set.c -- All cvar changes pass through this

#include "quakedef.h"

qbool	accepts_default_values = true;  // Tells us when to stop updating defaults


void Cvar_KickOnChange(cvar_t *var)
{
	if (!var->default_string)
		Sys_Error ("Cvar System Kick: Cvar %s not registered\n", var->cvarname);

	var->OnChange(var, var->string);
}

// Baker: onchange things need to fire during initialization
qbool Cvar_SetStringByRef (cvar_t *var, const char *value)
{
//	cvar_t		*var;
	qbool	changed;
	static qbool	changing = false;

	if (!var)	return false;

	// Baker: Protect against unregistered cvars being manipulated.  Like conditional ones like vid_vsync that don't always exist.
	if (!var->default_string)
	{
		Con_Warning ("%s: Variable %s not session_registered.\n", "Cvar_SetStringByRef", var->cvarname);
		return false;
	}

	changed = COM_StringNOTMatch (var->string, value);

	if (var->flags & CVAR_ROM || var->flags & CVAR_CMDLINE)
	{
//		if (!con_initialized) return;  // Baker: why?  Still goes to con_debug?  Oh well ...

		if (var->flags & CVAR_CMDLINE)
			Con_Printf ("\"%s\" is read-only due to command line param\n", var->cvarname);
		else if (developer.integer || changed)	// Baker: don't redundantly spam console when no change is actually happening
			Con_Printf ("\"%s\" is read-only\n", var->cvarname);

		return false;
	}

//	if ((var->flags & CVAR_INIT) && host_initialized)
//	{
//		if (developer.integer || changed )  // Baker: don't redundantly spam console when no change is actually happening
//			Con_Printf ("\"%s\" can only be changed with \"+set %s %s\" on the command line.\n", var->cvarname, var->cvarname, value);
//			Con_Printf ("\"%s\" is an initialization cvar.\n", var->cvarname);
//		return false;
//	}

	if (var->flags & CVAR_CMDLINE)
	{
		Con_Printf ("\"%s\" ignored due to command line parameter overrride.\n", var->cvarname);
		return false;
	}

	// CVAR_COMMAND function occurs after the change so don't do it here
	if (var->OnChange						// Baker: Must have an Onchange function
		&& !(var->flags & CVAR_COMMAND)		// Baker: Don't do CVAR_COMMAND here
		&& !changing)						// Baker: Prevents recursion ?
	{
		Con_DevPrintf (DEV_CVAR, "BEGIN Firing normal OnChange: \"%s\" from \"%s\" to \"%s\" ...\n", var->cvarname, var->string, value);
		changing = true;
		if (var->OnChange(var, value))
		{
			changing = false;
			Con_DevPrintf (DEV_CVAR, "CVAR REJECTED CHANGE: \"%s\"\n", var->cvarname);
			Con_DevPrintf (DEV_CVAR, "END   Firing \"%s\"\n", var->cvarname);
			return false;
		}
		changing = false;
		Con_DevPrintf (DEV_CVAR, "END   Firing \"%s\"\n", var->cvarname);
	}
	else if (var->OnChange && (var->flags & CVAR_COMMAND))
		// Announcing this early because
		Con_DevPrintf (DEV_CVAR, "BEGIN Firing command OnChange: \"%s\" from \"%s\" to \"%s\" ...\n", var->cvarname, var->string, value);
#ifdef _DEBUG
	else if (var->OnChange)
	{
		// Find out why it didn't fire
		Con_Printf ("Check 'em\n");
	}

#endif

	if (host_initialized && accepts_default_values)	// Update default value
	{
		Z_Free (var->default_string);
		var->default_string = Z_Strdup (value);

		Con_DevPrintf (DEV_CVAR, "Default value updated for '%s' to '%s'\n",  var->cvarname, var->default_string);
	}

	Z_Free (var->string);	// free the old value string
	var->string = Z_Strdup (value);

//	Z_FREE_AND_Z_STRDUP (var->string, value);

	var->floater = atof (var->string);
	var->integer = (int)var->floater;

	if (var->OnChange && (var->flags & CVAR_COMMAND))
	{
		Con_DevPrintf (DEV_CVAR, "      Firing now \"%s\" ...\n", var->cvarname);
		var->OnChange (var, value);	// Note param 2 just informational
		Con_DevPrintf (DEV_CVAR, "END   Firing \"%s\"\n", var->cvarname);
	}

	if ((var->flags & CVAR_SERVER) && changed && sv.active)
			SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", var->cvarname, var->string);

	// joe, from ProQuake: rcon (64 doesn't mean anything special,
	// but we need some extra space because NET_MAXMESSAGE == RCON_BUFF_SIZE)
	if (rcon_active && (rcon_message.cursize < rcon_message.maxsize - strlen(var->cvarname) - strlen(var->string) - 64))
	{
		rcon_message.cursize--;
		MSG_WriteString (&rcon_message, va("\"%s\" set to \"%s\"\n", var->cvarname, var->string));
	}

//	if (COM_StringMatch (var->cvarname, "pq_lag"))
// {
//		var->value = CLAMP (0, var->value, 400);
//		Cbuf_AddText (va("say \"ping +%d\"\n", (int)var->value));
//	}
	return true;
}

void Cvar_SetFloatByRef (cvar_t *var, float value)
{
	char	val[128];
	int	i;

	if (!var)	return;

	// Baker: Protect against unregistered cvars being manipulated.  Like conditional ones like vid_vsync that don't always exist.
	if (!var->default_string)	{ Con_Warning ("%s: Variable %s not session_registered.\n", "Cvar_SetFloatByRef", var->cvarname); return; }

	snprintf (val, sizeof(val), "%f", value);

	for (i = strlen(val) - 1 ; i > 0 && val[i] == '0' ; i--)
		val[i] = 0;
	if (val[i] == '.')
		val[i] = 0;

	Cvar_SetStringByRef (var, val);
}

int Cvar_GetFlagsByRef (const cvar_t *var)
{
	return var->flags;
}

void Cvar_SetFlagByRef (cvar_t *var, int newflag)
{
	var->flags |= newflag;
}




void Cvar_ForceSetStringByRef (cvar_t *var, const char *value)
{
	int	saved_flags;

	if (!var)
		return;

	// Baker: Protect against unregistered cvars being manipulated.  Like conditional ones like vid_vsync that don't always exist.
	if (!var->default_string)	{ Con_Warning ("%s: Variable %s not session_registered.\n", "Cvar_ForceSetStringByRef", var->cvarname); return; }

	saved_flags = var->flags;
	var->flags &= ~CVAR_ROM;		// Baker; technically this might not be enough
	Cvar_SetStringByRef (var, value);
	var->flags = saved_flags;
}

void Cvar_ForceFloatByRef (cvar_t *var, float value)
{
	char	val[128];
	int	i;

	if (!var)	return;

	// Baker: Protect against unregistered cvars being manipulated.  Like conditional ones like vid_vsync that don't always exist.
	if (!var->default_string)	{ Con_Warning ("%s: Variable %s not session_registered.\n", "Cvar_SetFloatByRef", var->cvarname); return; }

	snprintf (val, sizeof(val), "%f", value);

	for (i = strlen(val) - 1 ; i > 0 && val[i] == '0' ; i--)
		val[i] = 0;
	if (val[i] == '.')
		val[i] = 0;

	Cvar_ForceSetStringByRef (var, val);
}


// Baker: Makes a cvar unwriteable.  Strengthen server security I hope.
//        By making certain settings unchangeable.
void Cvar_CvarLock_f (void)
{
	cvar_t	*var;
	char	*s;
	int		i;

	if (Cmd_Argc() < 2)
	{
		Con_Printf ("Usage: %s <variable> [variable2 [variable3 ..]]\n", Cmd_Argv(0));
		return;
	}

	s = Cmd_Argv(1);

	for (i=1 ; i<Cmd_Argc() ; i++)
	{
		s = Cmd_Argv(i);

		if ((var = Cvar_FindVarByName(s)))
		{
			if (var->flags & CVAR_ROM)
				Con_Warning ("%s : Variable \"%s\" already read-only.\n", Cmd_Argv(0), var->cvarname);
			else
			{
				Con_Printf ("LOCKED: Variable \"%s\" is now read-only.\n", var->cvarname);
				var->flags |= CVAR_ROM;
			}
		}
		else
			Con_Printf ("%s : No variable with name %s\n", Cmd_Argv(0), s);
	}
}

qbool Cvar_CmdLineCheckForceFloatByRef_Maybe (int DevOnlyLevel, const char *cmdline_param, cvar_t *var, float value, const char *msg)
{
	if (!COM_CheckParm(cmdline_param))	return false;			// Command line parameter not found

	// Found: Force the value, set the flag and do the warning
	Cvar_ForceFloatByRef (var, value);
	Cvar_SetFlagByRef    (var, CVAR_CMDLINE | CVAR_ROM);

	if (DevOnlyLevel)
		Con_DevPrintf (DevOnlyLevel, "%s forced by command line (param \"%s\")\n", msg, cmdline_param);
	else
		Con_Warning          ("%s forced by command line (param \"%s\")\n", msg, cmdline_param);

	return true;
}



