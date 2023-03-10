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
// movie_avi.h



#ifndef __MOVIE_AVI_H__
#define __MOVIE_AVI_H__


void AVI_LoadLibrary (void);
void ACM_LoadLibrary (void);
qbool Capture_Open (char *filename);
void Capture_WriteVideo (byte *pixel_buffer);
void Capture_WriteAudio (int samples, byte *sample_buffer);
void Capture_Close (void);

#endif // __MOVIE_AVI_H__
