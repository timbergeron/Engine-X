/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// Baker: OSX and Linux (X Windows) seem to have
// an intermediary function that needs to be called
// to sniff out "special keys" and in those operating
// systems these helper functions will translate and
// call "Key_Events"

// these are the key numbers that should be passed to Key_Event
typedef enum
{
	K_TAB = 9,
	K_ENTER = 13,
	K_ESCAPE = 27,
	K_SPACE	= 32,

// normal keys should be passed as lowercased ascii

	K_BACKSPACE = 127,
	K_CAPSLOCK,
	K_UPARROW,
	K_DOWNARROW,
	K_LEFTARROW,
	K_RIGHTARROW,
	K_ALT,
#if 0
	K_LALT,		// Windows only and we are not allowing?
	K_RALT,		// Windows only and we are not allowing?
#endif
	K_CTRL,
#if 0
	K_LCTRL,	// Windows only and we are not allowing?
	K_RCTRL,	// Windows only and we are not allowing?
#endif
	K_SHIFT,
#if 0
	K_LSHIFT,	// Windows only and we are not allowing?
	K_RSHIFT,	// Windows only and we are not allowing?
#endif
	K_F1,
	K_F2,
	K_F3,
	K_F4,
	K_F5,
	K_F6,
	K_F7,
	K_F8,
	K_F9,
	K_F10,
	K_F11,
	K_F12,		// F12 is Eject CD on Mac in held
	K_PRINTSCR, // K_F13 on Mac
	K_SCRLCK,	// K_F14 on Mac
	K_PAUSE,	// K_F15 on Mac

	K_OSX_EQUAL_PAD,	// OSX has this key?

	K_INS,
	K_DEL,
	K_PGDN,
	K_PGUP,
	K_HOME,
	K_END,
#if 0
	K_WIN,				// K_COMMAND on Mac
#endif
	K_LWIN,
	K_RWIN,
	K_MENU,

// keypad keys
	KP_NUMLOCK,
	KP_SLASH,
	KP_STAR,
	KP_MINUS,

	KP_HOME,		// 1
	KP_UPARROW,		// 2
	KP_PGUP,		// 3
	KP_PLUS,
	KP_LEFTARROW,	// 4
	KP_5,			// 5
	KP_RIGHTARROW,	// 6

	KP_END,			// 7
	KP_DOWNARROW,	// 8
	KP_PGDN,		// 9

	KP_ENTER,
	KP_INS,			// 0
	KP_DEL,			// "." decimal point

// mouse buttons generate virtual keys

	K_MOUSE1 = 200,
	K_MOUSE2,
	K_MOUSE3,
	K_MOUSE4,
	K_MOUSE5,
	K_MOUSE6,	// Baker: Can't actually bind this
	K_MOUSE7,	// Baker: Can't actually bind this
	K_MOUSE8,	// Baker: Can't actually bind this


// joystick buttons

	K_JOY1,
	K_JOY2,
	K_JOY3,
	K_JOY4,

// aux keys are for multi-buttoned joysticks to generate so they can use
// the normal binding process

	K_AUX1,
	K_AUX2,
	K_AUX3,
	K_AUX4,
	K_AUX5,
	K_AUX6,
	K_AUX7,
	K_AUX8,
	K_AUX9,
	K_AUX10,
	K_AUX11,
	K_AUX12,
	K_AUX13,
	K_AUX14,
	K_AUX15,
	K_AUX16,
	K_AUX17,
	K_AUX18,
	K_AUX19,
	K_AUX20,
	K_AUX21,
	K_AUX22,
	K_AUX23,
	K_AUX24,
	K_AUX25,
	K_AUX26,
	K_AUX27,
	K_AUX28,
	K_AUX29,
	K_AUX30,
	K_AUX31,
	K_AUX32,

// JACK: Intellimouse(c) Mouse Wheel Support
// AWE:  Supported by the MacOS X version, too.

	K_MWHEELUP,
	K_MWHEELDOWN,

// Special capture when mouse is freed
	K_MOUSECLICK_BUTTON1 = 1024,
	K_MOUSECLICK_BUTTON2,
	K_MOUSECLICK_BUTTON3,
	K_MOUSECLICK_BUTTON4,
	K_MOUSECLICK_BUTTON5
} keynum_t;

#define	MAXCMDLINE		256


typedef enum
{
	key_game,
	key_console,
	key_message,
	key_menu
} keydest_t;

extern keydest_t	key_dest;
extern char *keybindings[256];
extern	int		key_repeats[256];
extern	int		key_count;			// incremented every key event
extern	int		key_lastpress;

void Key_Event (int key, int ascii, qbool down);
void Key_Init (void);
void Key_WriteBindings (FILE *f);
void Key_SetBinding (int keynum, char *binding);
void Key_ClearAllStates (void);

void History_Shutdown (void);
//#ifdef ENGINEX_DIFFERENCE
void Key_Extra (int *key);
char *Key_KeynumToString (int keynum);
//#endif

//#ifndef ENGINEX_DIFFERENCE
//qboolean Key_InternationalON(void);
//#endif
