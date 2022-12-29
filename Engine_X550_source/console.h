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

// console.h
// Baker: Validated 6-27-2011.  Boring functionary changes.

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

//extern	int	con_totallines;

extern qbool con_forcedup;		// because no entities to refresh
extern qbool con_initialized;
extern	byte	*con_chars;
extern	int	con_notifylines;	// scan lines to clear for notify lines

void Con_DrawCharacter (int cx, int line, int num);

void Con_CheckResize (void);
void Con_Init (void);
void Con_Shutdown (void);
void Con_DrawConsole (int lines, qbool drawinput);
void Con_Print (const char *txt);
void Con_Printf (const char *fmt, ...);
//void Con_Debugf (const char *fmt, ...);
void Con_Success (const char *fmt, ...); //johnfitz
void Con_Warning (const char *fmt, ...); //johnfitz


#define DEV_ANY			0		// Simple
#define DEV_INPUT		1
#define DEV_KEYBOARD	2
#define DEV_MOUSE		3
#define DEV_VIDEO		4
#define DEV_GRAPHICS	5
#define DEV_DEMOS		6
#define DEV_IO			7
#define DEV_PROTOCOL    8
#define DEV_MODEL		9
#define DEV_SOUND		10
#define DEV_GAMEDIR		11
#define DEV_IMAGE		12
#define DEV_OPENGL		13
#define DEV_SERVER		14
#define DEV_SYSTEM		15
#define DEV_GAMMA       16
#define DEV_CVAR		17

void Con_DevPrintf (int DevModeType, const char *fmt, ...);


void Con_SafePrintf (const char *fmt, ...);
void Con_Clear_f (void);
void Con_DrawNotify (void);
void Con_ClearNotify (void);
void Con_ToggleConsole_f (void);

void Con_NotifyBox (char *text);	// during startup for sound / cd warnings
char *Con_Quakebar (int len);


void Con_PrintColumnItem (const char *txt);
void Con_PrintColumnItem_MaxWidth (const char *txt);

int Con_GetCursorColumn (void);
int Con_GetMaximumBackscroll (void);

#endif // __CONSOLE_H__

