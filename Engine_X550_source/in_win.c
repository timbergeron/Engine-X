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
// in_win.c -- windows 95 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.

#include "quakedef.h"
#include "winquake.h"

qbool in_initialized = false;

// Only in_win.c is allow to see these.

// in_win_mouse.c

void IN_Mouse_ClearStates (void);
void IN_Mouse_UpdateClipCursor (void);
void IN_Mouse_Acquire (void);
void IN_Mouse_Unacquire (void);
void IN_Mouse_Startup (void);
void IN_Mouse_Shutdown (void);
void IN_Mouse_Restart (void);
void IN_Mouse_RegisterCvars (void);

void IN_Mouse_Event (int mstate);
void IN_Mouse_Move (usercmd_t *cmd);
void IN_Mouse_Accumulate (void);

// in_win_joystick.c
void IN_Joystick_Startup (void);
void IN_Joystick_Shutdown (void);
void IN_Joystick_Commands (void);
void IN_Joystick_RegisterCvars (void);
void IN_Joystick_Move (usercmd_t *cmd);

// in_win_keyboard.c

void IN_Keyboard_RegisterCvars (void);
void IN_Keyboard_Shutdown (void);
void IN_Keyboard_Startup (void);
void IN_Keyboard_Acquire (void);
void IN_Keyboard_Unacquire (void);


void Force_CenterView_f (void)
{
	cl.viewangles[PITCH] = 0;
}

/*
===========
IN_Move
===========
*/
void IN_Move (usercmd_t *cmd)
{
	if (!ActiveApp || Minimized) return;

	IN_Mouse_Move (cmd);
	IN_Joystick_Move (cmd);
}

void IN_Accumulate (void)
{
	IN_Mouse_Accumulate ();
}

void IN_Shutdown (void)
{
	if (!in_initialized)	return;
	IN_Mouse_Shutdown ();
	IN_Joystick_Shutdown ();
	IN_Keyboard_Shutdown ();

	Con_DevPrintf (DEV_INPUT, "Input shutdown complete.\n");
}


void IN_Startup (void)
{
	IN_Mouse_Startup ();
	IN_Joystick_Startup ();
	IN_Keyboard_Startup ();

	Con_DevPrintf (DEV_INPUT, "Input startup complete.\n");
}


void IN_Restart (void)
{
	Con_DevPrintf (DEV_INPUT, "Input restart begin...\n");
	IN_Shutdown ();
	IN_Startup  ();
	Con_DevPrintf (DEV_INPUT, "Input restart complete.\n");
}

void IN_ClearStates (void)
{
	IN_Mouse_ClearStates ();
}

// Baker: This gets called once
void IN_Init (void)
{
	// Input devices

	Cvar_Registration_Client_Devices ();

	Cvar_CmdLineCheckForceFloatByRef_Maybe (false, "-nomouse",    &in_mouse,    0, "Mouse disabled");
	Cvar_CmdLineCheckForceFloatByRef_Maybe (false, "-joystick",   &in_joystick, 1, "Joystick enabled");
	Cvar_CmdLineCheckForceFloatByRef_Maybe (false, "-nojoy",      &in_joystick, 1, "Joystick disabled");
	Cvar_CmdLineCheckForceFloatByRef_Maybe (false, "-nojoystick", &in_joystick, 1, "Joystick disabled");

	// Does cvar registration and command line check
	IN_Mouse_RegisterCvars ();
	IN_Keyboard_RegisterCvars ();
	IN_Joystick_RegisterCvars ();

	Cmd_AddCommand ("force_centerview", Force_CenterView_f);

	in_initialized = true;	// Must occur before IN_Startup as this allows certain input cvars to act

	IN_Startup ();

	Con_SafePrintf ("Input initialized.\n");
}




