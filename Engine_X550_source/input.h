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
// input.h -- external (non-keyboard) input devices

#ifndef __INPUT_H__
#define __INPUT_H__


#define mlook_active	(in_freelook.integer || (in_mlook.state & 1))


// cl_input.c

void CL_BoundViewPitch (float *viewangles);



void IN_Mouse_Commands_OSX (void);

// in_win.c

extern qbool in_initialized;
void IN_Accumulate (void);
void IN_Move (usercmd_t *cmd);
void IN_Mouse_MouseWheel (void);
void IN_ClearStates (void);
void IN_Init (void);
void IN_Shutdown (void);


// in_win_joystick.c
void IN_Joystick_Commands (void);

// in_win_mouse.c
void IN_Mouse_Event (int mstate);
void IN_Mouse_Acquire (void);
void IN_Mouse_Unacquire (void);
void IN_Mouse_UpdateClipCursor (void);
void IN_Mouse_Restart (void);

extern qbool in_mouse_acquired;

// in_win_keyboard.c

void IN_Keyboard_Acquire (void);
void IN_Keyboard_Unacquire (void);

#endif // __INPUT_H__