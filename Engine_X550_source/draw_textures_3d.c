
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
// draw_textures_3d.c -- Baker

#include "quakedef.h"

typedef struct fRect_s {
	float	left, top, right, bottom;
} fRect_t;


// Baker: All alpha masked stuff should be sorted by depth as far as I can tell.
/*
void R_EmitCaption (const vec3_t location, const char *caption)
{
	vec3_t				point;
	vec3_t				right, up;
	float charwidth		= 8;
	float charheight    = 8;
	float string_length = strlen(caption);
	float string_width  = string_length * charwidth;
	float string_height = charheight;
	fRect_t				captionbox;
	int					i;

	captionbox.top	  = -string_height/2;
	captionbox.bottom =  string_height/2;
	captionbox.left   = -string_width /2;
	captionbox.right  =  string_width /2;

	// Copy the view origin up and right angles
	VectorCopy (vup, up);
	VectorCopy (vright, right);

	mglPushStates ();

	GL_Bind				(char_texture);
	MeglDisable			(GL_CULL_FACE);
	MeglEnable			(GL_ALPHA_TEST);
//	MeglDepthMask		(GL_FALSE);			// This lets things later in the draw list draw over them even if behind (no Z)
//	MeglDisable			(GL_DEPTH_TEST);

	MeglDepthRange (0, 0.3);

	mglFinishedStates ();

	
	for (i=0; i<string_length; i++)
	{
		fRect_t				charcoords;
		float				s, t;
		int					charcode  = caption[i];	// "ascii" ish character code, but Quake character code
		int					charrow   = charcode >> 4; // Fancy way of dividing by 16
		int					charcol   = charcode & 15; // column = mod (16-1)
		// Calculate s, t texture coords from character set image

		s = charcol * 0.0625f; // 0.0625 = 1/16th
		t = charrow * 0.0625f;

		// Calculate coords

		charcoords.top	  =	 captionbox.top;
		charcoords.bottom =  captionbox.bottom;
		charcoords.left   =  captionbox.left + (i/string_length)*(captionbox.right - captionbox.left);
		charcoords.right   = captionbox.left + ((i+1)/string_length)*(captionbox.right - captionbox.left);
		
		// Draw box

		eglBegin (GL_QUADS);

		// top left		
		eglTexCoord2f (s, t + 0.03125);
		VectorMultiplyAdd (location, charcoords.top, up, point);
		VectorMultiplyAdd (point, charcoords.left, right, point);
		eglVertex3fv (point);

		// top, right
		eglTexCoord2f (s + 0.0625, t + 0.03125);	// 0.03125 = 1/32th;
		VectorMultiplyAdd (location, charcoords.top, up, point);
		VectorMultiplyAdd (point, charcoords.right, right, point);
		eglVertex3fv (point);

		// bottom right
		
		eglTexCoord2f (s + 0.0625, t);
		VectorMultiplyAdd (location, charcoords.bottom, up, point);
		VectorMultiplyAdd (point, charcoords.right, right, point);
		eglVertex3fv (point);

		// bottom, left
		eglTexCoord2f (s, t);
		VectorMultiplyAdd (location, charcoords.bottom, up, point);
		VectorMultiplyAdd (point, charcoords.left, right, point);
		eglVertex3fv (point);

		eglEnd ();
	}

	MeglEnable			(GL_CULL_FACE);
	MeglDisable			(GL_ALPHA_TEST);
//	MeglDepthMask		(GL_TRUE);
//	MeglEnable			(GL_DEPTH_TEST);
	MeglDepthRange (0, 1);

	mglPopStates ();
}
*/


void Text_Get_Columns_And_Rows (const char *myText, int *return_columns, int *return_rows);
void R_EmitCaption (const vec3_t location, const char *caption)
{
	const int			string_length = strlen(caption);
	const float			charwidth	= 8;
	const float			charheight  = 8;
	
	vec3_t				point;
	vec3_t				right, up;
	
	int					string_columns, string_rows;
	float				string_width, string_height; // pixels
	fRect_t				captionbox;
	int					i, x, y;
	qbool				bronze = false;

	// Step 1: Detemine Rows and columns
	Text_Get_Columns_And_Rows (caption, &string_columns, &string_rows);
	string_width = (float)string_columns * charwidth;
	string_height = (float)string_rows * charheight;


	// Step 2: Calculate bounds of rectangle

	captionbox.top	  = -(float)string_height/2;
	captionbox.bottom =  (float)string_height/2;
	captionbox.left   = -(float)string_width /2;
	captionbox.right  =  (float)string_width /2;

	// Copy the view origin up and right angles
	VectorCopy (vup, up);
	VectorCopy (vright, right);

	mglPushStates ();

	GL_Bind				(char_texture);
	MeglDisable			(GL_CULL_FACE);
	MeglEnable			(GL_ALPHA_TEST);

	MeglDepthRange (0, 0.3);	// Baker: The "hack the depth range" trick.  Not sure why this works, but it does ...

	mglFinishedStates ();
	
	for (i=0, x = 0, y =0; i<string_length; i++)
	{	
		// Deal with the string
		if (caption[i] == '\n')		{ x = 0; y ++; continue; }		// Reset the row, ignore and carry on
		if (caption[i] == '\b')		
		{ 
			bronze = !bronze; 
			continue; 
		}	// Toggle the bronzing and carry on

		x++;	// New character to print
		{
			fRect_t				charcoords;
			float				s, t;
			int					charcode  = caption[i] | (bronze * 128);	// "ascii" ish character code, but Quake character code
			int					charrow   = charcode >> 4; // Fancy way of dividing by 16
			int					charcol   = charcode & 15; // column = mod (16-1)
			// Calculate s, t texture coords from character set image

//			if (bronze) charcode |=128;

			s = charcol * 0.0625f; // 0.0625 = 1/16th
			t = charrow * 0.0625f;

			// Calculate coords

//			charcoords.top	  =	 captionbox.top;
//			charcoords.bottom =  captionbox.bottom;
//			charcoords.left   =  captionbox.left + (x/string_length)*(captionbox.right - captionbox.left);
//			charcoords.right   = captionbox.left + ((x+1)/string_length)*(captionbox.right - captionbox.left);

//			charcoords.top	  =	 captionbox.top + (y/(float)string_rows)*(captionbox.bottom - captionbox.top);
//			charcoords.bottom =  captionbox.top + ((y+1)/(float)string_rows)*(captionbox.bottom - captionbox.top);

			charcoords.top	  =	 captionbox.top + ((string_rows - 1 - y)/(float)string_rows)*(captionbox.bottom - captionbox.top);
			charcoords.bottom =  captionbox.top + (((string_rows - 1 - y)+1)/(float)string_rows)*(captionbox.bottom - captionbox.top);			
			charcoords.left   =  captionbox.left + (x/(float)string_columns)*(captionbox.right - captionbox.left);
			charcoords.right  = captionbox.left + ((x+1)/(float)string_columns)*(captionbox.right - captionbox.left);


			// Draw box

			eglBegin (GL_QUADS);

			// top left		
			eglTexCoord2f (s, t + 0.03125);
			VectorMultiplyAdd (location, charcoords.top, up, point);
			VectorMultiplyAdd (point, charcoords.left, right, point);
			eglVertex3fv (point);

			// top, right
			eglTexCoord2f (s + 0.0625, t + 0.03125);	// 0.03125 = 1/32th;
			VectorMultiplyAdd (location, charcoords.top, up, point);
			VectorMultiplyAdd (point, charcoords.right, right, point);
			eglVertex3fv (point);

			// bottom right
			
			eglTexCoord2f (s + 0.0625, t);
			VectorMultiplyAdd (location, charcoords.bottom, up, point);
			VectorMultiplyAdd (point, charcoords.right, right, point);
			eglVertex3fv (point);

			// bottom, left
			eglTexCoord2f (s, t);
			VectorMultiplyAdd (location, charcoords.bottom, up, point);
			VectorMultiplyAdd (point, charcoords.left, right, point);
			eglVertex3fv (point);

			eglEnd ();
		}
	}

	MeglEnable			(GL_CULL_FACE);
	MeglDisable			(GL_ALPHA_TEST);
	MeglDepthRange (0, 1);

	mglPopStates ();
}
