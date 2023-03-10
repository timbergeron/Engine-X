/*
Copyright (C) 2002 Quake done Quick

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
// movie.h

#ifndef __MOVIE_H__
#define __MOVIE_H__


void Movie_Init (void);
void Movie_StopPlayback (void);
double Movie_FrameTime (void);
void Movie_UpdateScreen (void);
void Movie_TransferStereo16 (void);
qbool Movie_GetSoundtime (void);
qbool Movie_IsActive (void);

#endif // __MOVIE_H__
