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
// in_win_keyboard.c -- keymapping stuff

#include "quakedef.h"
#include "winquake.h"

//==========================================================================

static byte scantokey[128] =  {
//      0       	1        	2       	3       	4       	5       	6       	7
//      8       	9        	A       	B       	C       	D       	E       	F
		0,   		K_ESCAPE,	'1',    	'2',    	'3',    	'4',    	'5',    	'6',
		'7',    	'8',    	'9',    	'0',    	'-',    	'=',    	K_BACKSPACE, 9,   	// 0
		'q',    	'w',    	'e',    	'r',    	't',    	'y',    	'u',    	'i',
		'o',    	'p',    	'[',    	']',    	K_ENTER,	K_CTRL, 	'a',    	's',	// 1
		'd',    	'f',    	'g',    	'h',    	'j',    	'k',    	'l',    	';',
		'\'',   	'`',    	K_SHIFT,	'\\',   	'z',    	'x',    	'c',    	'v',	// 2
		'b',    	'n',    	'm',    	',',    	'.',    	'/',    	K_SHIFT,	KP_STAR,
		K_ALT,  	' ',  		K_CAPSLOCK,	K_F1,  		K_F2,   	K_F3,   	K_F4,   	K_F5,	// 3
		K_F6,   	K_F7,   	K_F8,   	K_F9,   	K_F10,  	K_PAUSE,	K_SCRLCK,	K_HOME,
		K_UPARROW,	K_PGUP,		KP_MINUS,	K_LEFTARROW,KP_5,		K_RIGHTARROW,KP_PLUS,	K_END,	// 4
		K_DOWNARROW,K_PGDN,		K_INS,		K_DEL, 		0,      	0,      	0,      	K_F11,
		K_F12,  	0,      	0,      	K_LWIN, 	K_RWIN, 	K_MENU, 	0,      	0,		// 5
		0,      	0,      	0,      	0,      	0,      	0,      	0,      	0,
		0,      	0,      	0,      	0,      	0,      	0,      	0,      	0,
		0,      	0,      	0,      	0,      	0,      	0,      	0,      	0,
		0,      	0,      	0,      	0,      	0,      	0,      	0,      	0
};

//Map from windows to quake keynums
int IN_MapKey (int key)
{
	int	extended;

	extended = (key >> 24) & 1;

	key = (key>>16) & 255;
	if (key > 127)
		return 0;

	key = scantokey[key];

	if (in_keypad.integer)
	{
		if (extended)
		{
			switch (key)
			{
			case K_ENTER:		return KP_ENTER;
			case '/':			return KP_SLASH;
			case K_PAUSE:		return KP_NUMLOCK;
#if 0
			case K_LALT:		return K_RALT;
			case K_LCTRL:		return K_RCTRL;
#endif
			};
		}
		else
		{
			switch (key)
			{
			case K_HOME:		return KP_HOME;
			case K_UPARROW:		return KP_UPARROW;
			case K_PGUP:		return KP_PGUP;
			case K_LEFTARROW:	return KP_LEFTARROW;
			case K_RIGHTARROW:	return KP_RIGHTARROW;
			case K_END:			return KP_END;
			case K_DOWNARROW:	return KP_DOWNARROW;
			case K_PGDN:		return KP_PGDN;
			case K_INS:			return KP_INS;
			case K_DEL:			return KP_DEL;
			}
		}
	}
	else
	{
		// cl_keypad 0, compatibility mode
		switch (key)
		{
		case KP_STAR:			return '*';
		case KP_MINUS:			return '-';
		case KP_5:				return '5';
		case KP_PLUS:			return '+';
		}
	}

	return key;
}


void IN_Keyboard_RegisterCvars (void)
{
	// keyboard variables

	Cvar_Registration_Client_Keyboard ();
}

STICKYKEYS StartupStickyKeys = {sizeof (STICKYKEYS), 0};
TOGGLEKEYS StartupToggleKeys = {sizeof (TOGGLEKEYS), 0};
FILTERKEYS StartupFilterKeys = {sizeof (FILTERKEYS), 0};


void AllowAccessibilityShortcutKeys (qbool bAllowKeys)
{
	static qbool initialized = false;

	if (!initialized)
	{	// Save the current sticky/toggle/filter key settings so they can be restored them later
		SystemParametersInfo (SPI_GETSTICKYKEYS, sizeof (STICKYKEYS), &StartupStickyKeys, 0);
		SystemParametersInfo (SPI_GETTOGGLEKEYS, sizeof (TOGGLEKEYS), &StartupToggleKeys, 0);
		SystemParametersInfo (SPI_GETFILTERKEYS, sizeof (FILTERKEYS), &StartupFilterKeys, 0);
		Con_Printf ("Accessibility key startup settings saved\n");
		initialized = true;
	}

	if (bAllowKeys)
	{
		// Restore StickyKeys/etc to original state
		// (note that this function is called "allow", not "enable"; if they were previously
		// disabled it will put them back that way too, it doesn't force them to be enabled.)
		SystemParametersInfo (SPI_SETSTICKYKEYS, sizeof (STICKYKEYS), &StartupStickyKeys, 0);
		SystemParametersInfo (SPI_SETTOGGLEKEYS, sizeof (TOGGLEKEYS), &StartupToggleKeys, 0);
		SystemParametersInfo (SPI_SETFILTERKEYS, sizeof (FILTERKEYS), &StartupFilterKeys, 0);

		Con_DevPrintf (DEV_KEYBOARD,"Accessibility keys enabled\n");
	}
	else
	{
		// Disable StickyKeys/etc shortcuts but if the accessibility feature is on,
		// then leave the settings alone as its probably being usefully used
		STICKYKEYS skOff = StartupStickyKeys;
		TOGGLEKEYS tkOff = StartupToggleKeys;
		FILTERKEYS fkOff = StartupFilterKeys;

		if ((skOff.dwFlags & SKF_STICKYKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
			skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;

			SystemParametersInfo (SPI_SETSTICKYKEYS, sizeof (STICKYKEYS), &skOff, 0);
		}

		if ((tkOff.dwFlags & TKF_TOGGLEKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
			tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;

			SystemParametersInfo (SPI_SETTOGGLEKEYS, sizeof (TOGGLEKEYS), &tkOff, 0);
		}

		if ((fkOff.dwFlags & FKF_FILTERKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
			fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;

			SystemParametersInfo (SPI_SETFILTERKEYS, sizeof (FILTERKEYS), &fkOff, 0);
		}

		Con_DevPrintf (DEV_KEYBOARD, "Accessibility keys disabled\n");
	}
}


typedef ULONG ULONG_PTR; // Baker 3.99r: this is not appropriate for 64 bit, only 32 but I'm not building 64 bit version

#if !defined(__GNUC__) && _MSC_VER <=1200 // MSVC6 ONLY -- Do not do for CodeBlocks/MinGW/GCC
typedef struct
{
    DWORD		vkCode;
    DWORD		scanCode;
    DWORD		flags;
    DWORD		time;
    ULONG_PTR dwExtraInfo;
} *PKBDLLHOOKSTRUCT;
#endif

#ifndef WITHOUT_WINKEYHOOK
#define LLKHF_UP			(KF_UP >> 8)
#ifndef KF_UP
#define KF_UP				0x8000
#endif
#endif


LRESULT CALLBACK LLWinKeyHook(int Code, WPARAM wParam, LPARAM lParam)
{
	PKBDLLHOOKSTRUCT p;
	p = (PKBDLLHOOKSTRUCT) lParam;

	if (ActiveApp)
	{ // //	Baker: Not allowing windows and property keys to be bound right now
		switch(p->vkCode)
		{
			case VK_LWIN: /*Key_Event (K_LWIN, !(p->flags & LLKHF_UP));*/ return 1;
			case VK_RWIN: /*Key_Event (K_RWIN, !(p->flags & LLKHF_UP));*/ return 1;
			case VK_APPS: /*Key_Event (K_MENU, !(p->flags & LLKHF_UP));*/ return 1;
		}
	}

	return CallNextHookEx(NULL, Code, wParam, lParam);
}



void AllowWindowsShortcutKeys (qbool bAllowKeys)
{
	static qbool WinKeyHook_isActive = false;
	static HHOOK WinKeyHook;

	if (!bAllowKeys)
	{
		// Disable if not already disabled
		if (!WinKeyHook_isActive)
		{
			if (!(WinKeyHook = SetWindowsHookEx(13, LLWinKeyHook, global_hInstance, 0)))
			{
				Con_Printf("Failed to install winkey hook.\n");
				Con_Printf("Microsoft Windows NT 4.0, 2000 or XP is required.\n");
				return;
			}

			WinKeyHook_isActive = true;
			Con_DevPrintf (DEV_KEYBOARD, "Windows and context menu key disabled\n");
		}
	}

	if (bAllowKeys)
	{	// Keys allowed .. stop hook
		if (WinKeyHook_isActive)
		{
			UnhookWindowsHookEx(WinKeyHook);
			WinKeyHook_isActive = false;
			Con_DevPrintf (DEV_KEYBOARD, "Windows and context menu key enabled\n");
		}
	}
}

void IN_Keyboard_Unacquire (void)
{
	AllowAccessibilityShortcutKeys (true);
	AllowWindowsShortcutKeys (true);
}


void IN_Keyboard_Acquire (void)
{
	AllowAccessibilityShortcutKeys (false);
	AllowWindowsShortcutKeys (false);
}

void IN_Keyboard_Startup (void)
{
	IN_Keyboard_Acquire ();
}

void IN_Keyboard_Shutdown (void)
{
	IN_Keyboard_Unacquire ();

}