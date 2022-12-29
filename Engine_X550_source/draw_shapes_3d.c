
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

/*
================
R_EmitWirePoint -- johnfitz -- draws a wireframe cross shape for point entities
================
*/
void R_EmitWirePoint (const vec_rgba_t myColor, const vec3_t origin)
{
	int size=8;

	mglPushStates ();

	GL_DisableMultitexture ();	// Better otherwise D3D leaves artifact lines
	//MeglDisable (GL_BLEND);
	MeglDisable				(GL_DEPTH_TEST);
	MeglPolygonMode			(GL_FRONT_AND_BACK, GL_LINE);
	//GL_PolygonOffset (OFFSET_SHOWTRIS);
//			MeglEnable(GL_POLYGON_OFFSET_FILL); // B
	MeglDisable				(GL_TEXTURE_2D);
	MeglDisable				(GL_CULL_FACE);
	MeglColor4fv			(myColor);
	
	mglFinishedStates ();
	
	eglBegin (GL_LINES);
	eglVertex3f (origin[0]-size, origin[1], origin[2]);
	eglVertex3f (origin[0]+size, origin[1], origin[2]);
	eglVertex3f (origin[0], origin[1]-size, origin[2]);
	eglVertex3f (origin[0], origin[1]+size, origin[2]);
	eglVertex3f (origin[0], origin[1], origin[2]-size);
	eglVertex3f (origin[0], origin[1], origin[2]+size);
	eglEnd ();


//	MeglDisable(GL_POLYGON_OFFSET_FILL); // B
	MeglEnable				(GL_TEXTURE_2D);
	//MeglEnable (GL_BLEND);
	MeglEnable				(GL_CULL_FACE);
	MeglPolygonMode			(GL_FRONT_AND_BACK, GL_FILL);
	//GL_PolygonOffset (OFFSET_NONE);
	MeglEnable				(GL_DEPTH_TEST);

	mglPopStates ();
}



/*
================
R_EmitWireBox -- johnfitz -- draws one axis aligned bounding box
================
*/

static void R_EmitBox_Edges (const vec3_t mins, const vec3_t maxs)
{
	eglBegin (GL_QUAD_STRIP);
	eglVertex3f (mins[0], mins[1], mins[2]);
	eglVertex3f (mins[0], mins[1], maxs[2]);

	eglVertex3f (maxs[0], mins[1], mins[2]);
	eglVertex3f (maxs[0], mins[1], maxs[2]);

	eglVertex3f (maxs[0], maxs[1], mins[2]);
	eglVertex3f (maxs[0], maxs[1], maxs[2]);

	eglVertex3f (mins[0], maxs[1], mins[2]);
	eglVertex3f (mins[0], maxs[1], maxs[2]);
	
	eglVertex3f (mins[0], mins[1], mins[2]);
	eglVertex3f (mins[0], mins[1], maxs[2]);
	eglEnd ();
}


static void R_EmitBox_Faces (const vec3_t mins, const vec3_t maxs)
{
	eglBegin (GL_QUADS);

    // Front Face
    eglVertex3f (mins[0], mins[1], maxs[2]); 
    eglVertex3f (maxs[0], mins[1], maxs[2]); 
    eglVertex3f (maxs[0], maxs[1], maxs[2]); 
    eglVertex3f (mins[0], maxs[1], maxs[2]); 
    // Back Face
    eglVertex3f (mins[0], mins[1], mins[2]); 
    eglVertex3f (mins[0], maxs[1], mins[2]); 
    eglVertex3f (maxs[0], maxs[1], mins[2]); 
    eglVertex3f (maxs[0], mins[1], mins[2]); 
    // Top Face
    eglVertex3f (mins[0], maxs[1], mins[2]); 
    eglVertex3f (mins[0], maxs[1], maxs[2]); 
    eglVertex3f (maxs[0], maxs[1], maxs[2]); 
    eglVertex3f (maxs[0], maxs[1], mins[2]); 
    // Bottom Face
    eglVertex3f (mins[0], mins[1], mins[2]); 
    eglVertex3f (maxs[0], mins[1], mins[2]); 
    eglVertex3f (maxs[0], mins[1], maxs[2]); 
    eglVertex3f (mins[0], mins[1], maxs[2]); 
    // Right face
    eglVertex3f (maxs[0], mins[1], mins[2]); 
    eglVertex3f (maxs[0], maxs[1], mins[2]); 
    eglVertex3f (maxs[0], maxs[1], maxs[2]); 
    eglVertex3f (maxs[0], mins[1], maxs[2]); 
    // Left Face
    eglVertex3f (mins[0], mins[1], mins[2]); 
    eglVertex3f (mins[0], mins[1], maxs[2]); 
    eglVertex3f (mins[0], maxs[1], maxs[2]); 
    eglVertex3f (mins[0], maxs[1], mins[2]); 

	eglEnd ();
}


static void R_EmitBox_CalcCoordsMinsMaxs (const vec3_t centerpoint, const vec3_t xyz_mins, const vec3_t xyz_maxs, const float ExpandSize, vec3_t mins, vec3_t maxs)
{	// Calculate mins and maxs
	int		calculation_type = !centerpoint ? 0 : (!xyz_mins ? 2 : 1);
	
	switch (calculation_type)
	{
	case 0:		// No centerpoint so these represent absolute coordinates
				VectorCopy (xyz_mins, mins);				
				VectorCopy (xyz_maxs, maxs);
				break;

				// We have a centerpoint so the other 2 coordinates are relative to the center
	case 1:		VectorAdd (centerpoint, xyz_mins, mins);	
				VectorAdd (centerpoint, xyz_maxs, maxs);
				break;
			
	case 2:		// No mins or maxs (point type entity)
				VectorCopy (centerpoint, mins);
				VectorCopy (centerpoint, maxs);
				break;
	}

	// Optionally, adjust the size a bit like pull the sides back just a little or expand it
	if (ExpandSize)		{ VectorAddFloat (mins,  -ExpandSize, mins);	VectorAddFloat (maxs, ExpandSize, maxs); }

}

vec3_t lastbox_mins, lastbox_maxs;
void R_EmitBox (const vec_rgba_t myColor, const vec3_t centerpoint, const vec3_t xyz_mins, const vec3_t xyz_maxs, const qbool bTopMost, const qbool bLines, const float Expandsize)
{                     
	float	pointentity_size = 8.0f;
	float	smallster = 0.10f;
	vec3_t	mins, maxs;
	
	// Calculate box size based on what we received (i.e., if we have a centerpoint use it otherwise it is absolute coords)
	R_EmitBox_CalcCoordsMinsMaxs (centerpoint, xyz_mins, xyz_maxs, Expandsize, /* outputs -> */ mins, maxs);

	// Setup to draw
	mglPushStates ();

	//	MeglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);		// This is a GL_SetupState default
	MeglDisable			(GL_TEXTURE_2D);
	MeglEnable			(GL_POLYGON_OFFSET_FILL);
	MeglDisable			(GL_CULL_FACE);
	MeglColor4fv		(myColor);

	if (myColor[3])	// If no alpha then it is solid
	{
		MeglEnable		(GL_BLEND);
		MeglDepthMask	(GL_FALSE);							
	}

	if (bTopMost)	// Draw topmost if specified
		MeglDisable		(GL_DEPTH_TEST);

	if (bLines)		// Set to lines
		MeglPolygonMode	(GL_FRONT_AND_BACK, GL_LINE);

	mglFinishedStates ();
	
	if (bLines)
		R_EmitBox_Edges (mins, maxs);
	else
		R_EmitBox_Faces (mins, maxs);

	//	MeglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);		// This is a GL_SetupState default
	MeglEnable			(GL_TEXTURE_2D);
	MeglDisable			(GL_POLYGON_OFFSET_FILL);
	MeglEnable			(GL_CULL_FACE);
	MeglColor4ubv		(color_white);

	if (myColor[3])	// If no alpha then it is solid
	{
		MeglDisable		(GL_BLEND);
		MeglDepthMask	(GL_TRUE);
	}

	if (bTopMost)	// Draw topmost if specified
		MeglEnable		(GL_DEPTH_TEST);
	
	if (bLines)		// Set to lines
		MeglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	

	mglPopStates ();

	VectorCopy (mins, lastbox_mins);
	VectorCopy (maxs, lastbox_maxs);
}

#if 0
void R_EmitCaptionBox (const vec_rgba_t myColor, const vec3_t location, const char *caption)
{
	vec3_t				point;
	vec3_t				right, up;
	float	string_length = strlen(caption);
	float string_width  = string_length * 8;
	float string_height = 8;
	fRect_t				captionbox;

	captionbox.top	  = -string_height/2;
	captionbox.bottom =  string_height/2;
	captionbox.left   = -string_width /2;
	captionbox.right  =  string_width /2;

	// Copy the view origin up and right angles
	VectorCopy (vup, up);
	VectorCopy (vright, right);

	mglPushStates ();

	MeglDisable			(GL_TEXTURE_2D);
	MeglEnable			(GL_POLYGON_OFFSET_FILL);
	MeglDisable			(GL_CULL_FACE);
	MeglColor4fv		(myColor);

	if (myColor[3])	// If no alpha then it is solid
	{
		MeglDisable		(GL_BLEND);
		MeglDepthMask	(GL_TRUE);
	}

	mglFinishedStates ();

	eglBegin (GL_QUADS);

//	eglTexCoord2f (0, 1);

	VectorMultiplyAdd (location, captionbox.bottom, up, point);
	VectorMultiplyAdd (point, captionbox.left, right, point);
	eglVertex3fv (point);

//	eglTexCoord2f (0, 0);
	VectorMultiplyAdd (location, captionbox.top, up, point);
	VectorMultiplyAdd (point, captionbox.left, right, point);
	eglVertex3fv (point);

//	eglTexCoord2f (1, 0);
	VectorMultiplyAdd (location, captionbox.top, up, point);
	VectorMultiplyAdd (point, captionbox.right, right, point);
	eglVertex3fv (point);

//	eglTexCoord2f (1, 1);
	VectorMultiplyAdd (location, captionbox.bottom, up, point);
	VectorMultiplyAdd (point, captionbox.right, right, point);
	eglVertex3fv (point);

	eglEnd ();

	MeglEnable			(GL_TEXTURE_2D);
	MeglDisable			(GL_POLYGON_OFFSET_FILL);
	MeglEnable			(GL_CULL_FACE);

	if (myColor[3])	// If no alpha then it is solid
	{
		MeglDisable		(GL_BLEND);
		MeglDepthMask	(GL_TRUE);
	}

	mglPopStates ();
}
#endif


