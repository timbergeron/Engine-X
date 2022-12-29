/*
Copyright (C) 1996-2003 A Nourai, Id Software, Inc.

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
// image.h


byte *Image_LoadTGA_FromOpenFile (FILE *fin, int *my_image_width, int *my_image_height); //, char *name, int matchwidth, int matchheight);
byte *Image_LoadPCX_FromOpenFile (FILE *fin, int *my_image_width, int *my_image_height); //, char *name, int matchwidth, int matchheight);



qbool Image_WriteTGA (const char *absolute_filename, const byte *pixels, const int width, const int height, const int BitsPerPixel, const qbool IsUpsideDown);

#if SUPPORTS_LIBPNG
byte *Image_LoadPNG_FromOpenFile (FILE *fin, const int matchwidth, const int matchheight); //, char *filename, int matchwidth, int matchheight);
qbool Image_WritePNG (const char *absolute_filename, int compression, byte *pixels, int width, int height);
#ifdef GLQUAKE
qbool Image_WritePNGPLTE (const char *absolute_filename, int compression, byte *pixels, int width, int height, byte *palette);
#else
qbool Image_WritePNGPLTE (const char *absolute_filename, int compression, byte *pixels, int width, int height, int rowbytes, byte *palette);
#endif
#endif

#if SUPPORTS_LIBJPEG
byte *Image_LoadJPEG_FromOpenFile (FILE *fin, int *my_image_width, int *my_image_height);
qbool Image_WriteJPEG (const char *filename, const int compression, byte *pixels, const int width, const int height);
#endif

#ifdef GLQUAKE
int Image_WritePCX (char *filename, byte *data, int width, int height, byte *palette);
#else
int Image_WritePCX (char *filename, byte *data, int width, int height, int rowbytes, byte *palette);
#endif

void Image_Init (void);
