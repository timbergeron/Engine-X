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
// winquake_sound.h: Win32-specific Quake header file

#ifndef __WINQUAKE_SOUND_H__
#define __WINQUAKE_SOUND_H__

#ifndef SERVERONLY


//#include <ddraw.h>
#ifdef __GNUC__
#include "dxsdk\sdk\inc\dsound.h"
#else
#include "dxsdk\sdk8\include\dsound.h"
#endif

//#define SNDBUFSIZE 65536
extern DWORD gSndBufSize;
//extern LPDIRECTSOUND pDS;
extern LPDIRECTSOUNDBUFFER pDSBuf;



#endif // SERVERONLY
#endif // __WINQUAKE_SOUND_H__