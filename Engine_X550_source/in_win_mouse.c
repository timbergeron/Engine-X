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

#define ALTER_WINDOWS_MPARMS	0

#include "quakedef.h"
#include "winquake.h"
#include "winquake_input.h"	// Rect stuff for mouse clip

#if ALTER_WINDOWS_MPARMS
static qbool	m_windows_parms_altered;
static int		originalmouseparms[3], newmouseparms[3] = {0, 0, 1};
static qbool	mouseparmsvalid;
#endif



const int		mouse_buttons = 5;
int				mouse_oldbuttonstate;
POINT			current_pos;
double			mouse_x, mouse_y;
int				old_mouse_x, old_mouse_y, mx_accum, my_accum;

qbool			in_mousestarted;
qbool			in_mouse_acquired;

#if SUPPORTS_DIRECTINPUT
qbool			in_dinput;

// Only in_win_mouse.c is allow to know about these.
// in_win_directinput.c

void IN_Mouse_DInput_Move (int *accumx, int *accumy);
void IN_Mouse_DInput_Acquire (void);
void IN_Mouse_DInput_Unacquire (void);
qbool IN_Mouse_DInput_Startup (void);
qbool IN_Mouse_DInput_Shutdown (void);
void IN_Mouse_DInput_MouseWheel (void);
#endif

/*
===================
IN_ClearStates
===================
*/
void IN_Mouse_ClearStates (void)
{
	if (!in_mouse_acquired) return;

	mx_accum = my_accum = mouse_oldbuttonstate = 0;
}

// Baker: Locks mouse movement to window area
void IN_Mouse_UpdateClipCursor (void)
{
	if (!in_mousestarted || !in_mouse_acquired) return;

	ClipCursor (&MouseWindowLockRegion);
	Con_DevPrintf (DEV_MOUSE, "Mouse roaming region updated to client Window area.\n");
}

// Baker: because showing twice and hiding once leaves cursor still visible
//        it is an incremental that expects a 1:1 ratio
void IN_ShowCursor (qbool showcursor)
{
	static int state = 0;

	if (state == 0 &&  showcursor) { Con_DevPrintf (DEV_MOUSE, "Cursor already showing\n"); return; } // Cursor already is showing
	if (state == 1 && !showcursor) { Con_DevPrintf (DEV_MOUSE, "Cursor already hidden\n"); return; } // Cursor already hidden

	if ( showcursor) state --;
	if (!showcursor) state ++;

	ShowCursor (showcursor);
}

void IN_Mouse_Acquire (void)
{
	if (!in_mousestarted) return;

#if SUPPORTS_DIRECTINPUT
	if (in_dinput)
		IN_Mouse_DInput_Acquire ();
	else
#endif
	{
#if ALTER_WINDOWS_MPARMS
		if (mouse_setparms.integer && mouseparmsvalid)
		{
			m_windows_parms_altered = SystemParametersInfo (SPI_SETMOUSE, 0, newmouseparms, 0);
			Con_DevPrintf (DEV_MOUSE, "Windows mouse parameters set.\n")
		}
#endif
		SetCursorPos (MouseRegionCenter_x, MouseRegionCenter_y);
		SetCapture (mainwindow);	Con_DevPrintf (DEV_MOUSE, "Mouse messages captured\n");
	}
	// MH appears to use the concept that DirectInput show do this too
	ClipCursor (&MouseWindowLockRegion);	Con_DevPrintf (DEV_MOUSE, "Clip cursor region updated\n");
	IN_ShowCursor (false);			Con_DevPrintf (DEV_MOUSE, "Cursor hidden.\n");

	in_mouse_acquired = true;
	Con_DevPrintf (DEV_MOUSE, "Mouse acquired\n");

}


void IN_Mouse_Unacquire (void)
{
	if (!in_mousestarted) return;

#if SUPPORTS_DIRECTINPUT
	if (in_dinput)
		IN_Mouse_DInput_Unacquire ();
	else
#endif
	{
#if ALTER_WINDOWS_MPARMS
		if (mouse_setparms.integer && m_windows_parms_altered)
		{
			SystemParametersInfo (SPI_SETMOUSE, 0, originalmouseparms, 0);
			Con_DevPrintf (DEV_MOUSE, "Windows mouse parameters restored.\n")
		}
#endif
		ReleaseCapture ();	Con_DevPrintf (DEV_MOUSE, "Mouse messages released\n");
	}
	// MH appears to use the concept that DirectInput show do this too
	ClipCursor (NULL);		Con_DevPrintf (DEV_MOUSE, "Clipcursor released.\n");
	IN_ShowCursor (true);	Con_DevPrintf (DEV_MOUSE, "Showing cursor.\n");

	in_mouse_acquired = false;
	Con_DevPrintf (DEV_MOUSE, "Mouse unacquired.\n");

}


/*
===========
IN_StartupMouse
===========
*/
void IN_Mouse_Startup (void)
{
	if (!in_mouse.integer) return;

	in_mousestarted = true;

#if SUPPORTS_DIRECTINPUT
	if (mouse_directinput.integer)
	{
		if ((in_dinput = IN_Mouse_DInput_Startup ()))
			Con_Success ("DirectInput initialized\n");
		else
			Con_Warning ("DirectInput not initialized\n");
	}
#endif

#if SUPPORTS_DIRECTINPUT
	if (!in_dinput)
#endif
	{
#if ALTER_WINDOWS_MPARMS
		if (mouse_setparms.integer)
		{
			mouseparmsvalid = SystemParametersInfo (SPI_GETMOUSE, 0, originalmouseparms, 0);

			if (mouseparmsvalid)
			{
				if (COM_CheckParm ("-noforcemspd"))
					newmouseparms[2] = originalmouseparms[2];

				if (COM_CheckParm ("-noforcemaccel"))
				{
					newmouseparms[0] = originalmouseparms[0];
					newmouseparms[1] = originalmouseparms[1];
				}

				if (COM_CheckParm ("-noforcemparms"))
				{
					newmouseparms[0] = originalmouseparms[0];
					newmouseparms[1] = originalmouseparms[1];
					newmouseparms[2] = originalmouseparms[2];
				}

				Con_DevPrintf (DEV_MOUSE, "Mouse parameters read.\n");
			}
		}
#endif
	}

	IN_Mouse_Acquire ();
	Con_DevPrintf (DEV_MOUSE, "Mouse startup complete.\n");
}

void IN_Mouse_Shutdown (void)
{
	if (!in_mouse.integer) return; // We don't shutdown if we didn't startup

	Con_DevPrintf (DEV_MOUSE, "Mouse shutdown begin ...\n");
	IN_Mouse_Unacquire ();

#if SUPPORTS_DIRECTINPUT
	in_dinput = IN_Mouse_DInput_Shutdown ();
	Con_DevPrintf (DEV_MOUSE, "DirectInput shutdown.\n");
#endif

	IN_Mouse_ClearStates ();

	Con_DevPrintf (DEV_MOUSE, "Mouse shutdown complete.\n");
	in_mousestarted = false;
}



void IN_Mouse_Restart (void)
{
	static qbool firsttime = false;
	qbool wasMouseAcquired = in_mouse_acquired; // It might not be ...

	if (!in_initialized) return;	// We haven't initialized input yet

	if (!firsttime)		// Don't call it a restart if it wasn't a restart
		Con_DevPrintf (DEV_MOUSE, "Mouse restart begin ...\n");

	if (!wasMouseAcquired)
		IN_Mouse_Acquire (); // Grab it if we don't have it ... we will release it in shutdown anyway.

	IN_Mouse_Shutdown ();
	IN_Mouse_Startup  (); // Now at this point ... Mouse startup acquires the mouse.

	if (!wasMouseAcquired)
		IN_Mouse_Unacquire ();

	if (!firsttime)		// Don't call it a restart if it wasn't a restart
		Con_DevPrintf (DEV_MOUSE, "Mouse restart complete.\n");
}

void IN_Mouse_RegisterCvars (void)
{

	Cvar_Registration_Client_Mouse ();

#if ALTER_WINDOWS_MPARMS

	Cvar_CmdLineCheckForceFloatByRef_Maybe (DEV_MOUSE,"-nomouseparms",   &mouse_setparms, 0, "Mouse parameter alterations off");
#endif




#if SUPPORTS_DIRECTINPUT
	Cvar_Registration_Client_Mouse_DirectInput ();


	Cvar_CmdLineCheckForceFloatByRef_Maybe (false, "-dinput",   &mouse_directinput, 1, "DirectInput");
	Cvar_CmdLineCheckForceFloatByRef_Maybe (false, "-nodinput", &mouse_directinput, 0, "Standard mouse input");

	//Force kick it if value is 1 ... otherwise we should be ok
	if (&mouse_directinput.integer)
		Cvar_KickOnChange (&mouse_directinput);	// Cvar needs to execute onchange event to take effect
#endif

	Con_DevPrintf (DEV_MOUSE, "Mouse console variables session_registered.\n");
}





/*
===========
IN_MouseEvent
===========
*/
void IN_Mouse_Event (int mstate)
{
	int	i;

	if (!in_mouse_acquired)	return;
#if SUPPORTS_DIRECTINPUT
	if (in_dinput)			return;
#endif

	// perform button actions
		for (i=0 ; i<mouse_buttons ; i++)
		{
			if ((mstate & (1<<i)) && !(mouse_oldbuttonstate & (1<<i)))
				Key_Event (K_MOUSE1 + i, 0, true);

			if (!(mstate & (1<<i)) && (mouse_oldbuttonstate & (1<<i)))
				Key_Event (K_MOUSE1 + i, 0, false);
		}

		mouse_oldbuttonstate = mstate;
}


/*
===========
IN_MouseMove
===========
*/
void IN_Mouse_Move (usercmd_t *cmd)
{
	int			mx, my;
//	int			i;

	if (!in_mouse_acquired)
		return;

#if SUPPORTS_DIRECTINPUT
	if (in_dinput)
		IN_Mouse_DInput_Move (&mx, &my);
	else
#endif
	{
		GetCursorPos (&current_pos);
		mx = current_pos.x - MouseRegionCenter_x + mx_accum;
		my = current_pos.y - MouseRegionCenter_y + my_accum;
		mx_accum = my_accum = 0;
	}

//if (mx ||  my)
//	Con_DevPrintf (DEV_MOUSE, "mx=%d, my=%d\n", mx, my);


#if SUPPORTS_XFLIP
	if(scene_invert_x.integer) mx *= -1;   //Atomizer - GL_XFLIP
#endif

	if (in_filter.floater)
	{
        float filterfrac = CLAMP (0, in_filter.floater, 1) / 2.0;
        mouse_x = (mx * (1 - filterfrac) + old_mouse_x * filterfrac);
        mouse_y = (my * (1 - filterfrac) + old_mouse_y * filterfrac);
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}

	old_mouse_x = mx;
	old_mouse_y = my;

	// Baker: Scale the sensitivity so that FOV 90 is the norm, everything is relative to FOV 90
	//        There are 360 degrees of possible view.  Seeing 180 degrees is half.  90 degrees 1/4.
	//        Right now, we are going to scale both X and Y sensitivity by FOV X.
	//        The reason for this is that we don't want different aiming "feels" for different
	//        screen resolutions.  And since we use corrective FOV rendering, we cannot count on 
	//        the refdef screen fov_x to be the same as cvar FOV (or even predictable).
	//        So we are going to use the cvar FOV for now.  And if we add fancier ways to control
	//        weapon zooming and such that doesn't use the CVAR, we'll go from there.
	do
	{
		float my_sensitivity = in_sensitivity.floater;
		float effective_sensitivity_x = my_sensitivity;
		float effective_sensitivity_y = my_sensitivity;

		if (in_fovscale.integer && scene_fov_x.floater /* fov shouldn't ever be zero, but anyways .. */)
		{
			float fov_x_factor = scene_fov_x.floater / 90;
			effective_sensitivity_x *= fov_x_factor;
			effective_sensitivity_y *= fov_x_factor; // Not a typo, multiplying both by fov_x_factor
		}


		if (in_accel.floater)
		{
			float mousespeed = sqrtf (mx * mx + my * my);
			mouse_x *= (mousespeed * in_accel.floater + effective_sensitivity_x);
			mouse_y *= (mousespeed * in_accel.floater + effective_sensitivity_y);
		}
		else
		{
			mouse_x *= effective_sensitivity_x;
			mouse_y *= effective_sensitivity_y;
		}
	} while (0);

// add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || (in_lookstrafe.integer && mlook_active ))
		cmd->sidemove += mouse_lookstrafe_speed_side.floater * mouse_x;
	else
		cl.viewangles[YAW] -= mouse_speed_yaw.floater * mouse_x;

	if (mlook_active)
		View_StopPitchDrift ();

	if (mlook_active && !(in_strafe.state & 1))
	{
		float pitchchange = mouse_speed_pitch.floater * mouse_y;
		
		// Baker: new invert mouse scheme
		if (in_invert_pitch.integer)
			pitchchange = -pitchchange;

		cl.viewangles[PITCH] += pitchchange;
		CL_BoundViewPitch (cl.viewangles);	// mouse lock..  Covers directinput too.
	}
	else
	{
		if ((in_strafe.state & 1) && cl.noclip_anglehack)
			cmd->upmove -= mouse_lookstrafe_speed_forward.floater * mouse_y;
		else
			cmd->forwardmove -= mouse_lookstrafe_speed_forward.floater * mouse_y;
	}

// if the mouse has moved, force it to the center, so there's room to move
	if (mx || my)
		SetCursorPos (MouseRegionCenter_x, MouseRegionCenter_y);
}




/*
===========
IN_Accumulate
===========
*/
void IN_Mouse_Accumulate (void)
{
	if (!in_mouse_acquired) return;
#if SUPPORTS_DIRECTINPUT
	if (in_dinput)			return;
#endif

	GetCursorPos (&current_pos);

	mx_accum += current_pos.x - MouseRegionCenter_x;
	my_accum += current_pos.y - MouseRegionCenter_y;

// force the mouse to the center, so there's room to move
	SetCursorPos (MouseRegionCenter_x, MouseRegionCenter_y);
}

// Baker: Allows us to check for dinput mousewheel events
//        These don't get send via system messages

void IN_Mouse_MouseWheel (void)
{
	if (!in_mouse_acquired) return;
#if SUPPORTS_DIRECTINPUT
	if (in_dinput)
		IN_Mouse_DInput_MouseWheel ();
#endif
}

#if SUPPORTS_DIRECTINPUT
// Baker: reflects if DirectInput is wanted
qbool OnChange_m_directinput (cvar_t *var, const char *string)
{
	extern qbool dinput;
	if (!in_initialized)
	{
		return false;
	}

	if (in_dinput && mouse_directinput.integer)
		return false; // Already on

	if (!in_dinput && !mouse_directinput.integer)
		return false; // Already off

	IN_Mouse_Restart();

	// We could check it here, but we aren't going to

	return false;
}
#endif
