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
// winquake.h: Win32-specific Quake header file

#ifndef __WINQUAKE_H__
#define __WINQUAKE_H__

#pragma warning (disable : 4229)  // mgraph gets this

#include <windows.h>
//#define WM_XBUTTONDOWN		0x020B
//#define WM_XBUTTONUP		0x020C



#ifndef WM_GRAPHNOTIFY
#define WM_GRAPHNOTIFY  WM_USER + 13
#endif



extern	HINSTANCE	global_hInstance;
//extern	int		global_nCmdShow;


//#define MODE_WINDOWED		0
//#define NO_MODE			(MODE_WINDOWED - 1)
//#define MODE_FULLSCREEN_DEFAULT	(MODE_WINDOWED + 1)



extern HWND	mainwindow;
extern qbool	ActiveApp, Minimized;
//extern cvar_t _windowed_mouse;




void S_BlockSound (void);
void S_UnblockSound (void);

void VID_SetDefaultMode (void);

#endif // __WINQUAKE_H__
