// D3D diff 14 of 14
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
// gl_draw.c -- this is the only file outside the refresh that touches the vid buffer

#include "quakedef.h"


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_AlphaFill(const int x, const int y, const int w, const int h, const int c, const float in_alpha)
{
	const	float use_alpha = CLAMP (0, in_alpha, 1);

	if (!use_alpha)
		return;

	mglPushStates ();
	MeglDisable (GL_TEXTURE_2D);
	if (use_alpha < 1)
	{
		MeglEnable (GL_BLEND);
		MeglDisable(GL_ALPHA_TEST);
		MeglColor4f (host_basepal[c * 3] / 255.0,  host_basepal[c * 3 + 1] / 255.0, host_basepal[c * 3 + 2] / 255.0, use_alpha);
	}
	else
	{
		MeglColor3f (host_basepal[c * 3] / 255.0, host_basepal[c * 3 + 1] / 255.0, host_basepal[c * 3 + 2]  /255.0);
	}

	mglFinishedStates ();

	eglBegin (GL_QUADS);
	eglVertex2f (x, y);
	eglVertex2f (x + w, y);
	eglVertex2f (x + w, y + h);
	eglVertex2f (x, y + h);
	eglEnd ();

	MeglEnable (GL_TEXTURE_2D);
	if (use_alpha < 1)
	{
		MeglEnable(GL_ALPHA_TEST);
		MeglDisable (GL_BLEND);
	}
	MeglColor3ubv (color_white);

	mglPushStates ();
}

void Draw_Fill (const int x, const int y, const int w, const int h, const int c)
{
	mglPushStates ();

	MeglDisable (GL_TEXTURE_2D);
	MeglColor3f (host_basepal[c*3] / 255.0, host_basepal[c*3+1] / 255.0, host_basepal[c*3+2] / 255.0);

	mglFinishedStates ();

	eglBegin (GL_QUADS);

	eglVertex2f (x, y);
	eglVertex2f (x+w, y);
	eglVertex2f (x+w, y+h);
	eglVertex2f (x, y+h);

	eglEnd ();
	MeglColor3ubv (color_white);
	MeglEnable (GL_TEXTURE_2D);
}

void Draw_FillRGB (int x, int y, int w, int h, float red, float green, float blue)
{

	mglPushStates ();

	MeglDisable (GL_TEXTURE_2D);
	MeglColor3f (red, green, blue);

	mglFinishedStates ();

	eglBegin (GL_QUADS);

	eglVertex2f (x, y);
	eglVertex2f (x+w, y);
	eglVertex2f (x+w, y+h);
	eglVertex2f (x, y+h);

	eglEnd ();
	MeglColor3ubv (color_white);
	MeglEnable (GL_TEXTURE_2D);

	mglPopStates ();
}

// Angle should degrees + for now
void Draw_FillRGBA (float x, float y, float w, float h, float red, float green, float blue, float alpha)
{
//	float slope1 =
//	float slope2

//  slope = with every w units of change, y must be modified w * slope units
//  but really we are using negative slope

	mglPushStates ();

	MeglDisable (GL_TEXTURE_2D);
	MeglEnable (GL_BLEND);
	MeglDisable(GL_ALPHA_TEST);

	MeglColor4f (red, green, blue, alpha);

	mglFinishedStates ();

	eglBegin (GL_POLYGON);

	eglVertex2f (x, y);
	eglVertex2f (x + w, y);
	eglVertex2f (x + w, y + h);
	eglVertex2f (x, y + h);

	eglEnd ();

	MeglColor3ubv (color_white);;
	MeglEnable(GL_ALPHA_TEST);
	MeglDisable (GL_BLEND);
	MeglEnable (GL_TEXTURE_2D);

	mglPopStates ();
}

// Angle should degrees + for now
void Draw_FillRGBAngle (float x, float y, float w, float h, float red, float green, float blue, float slope)
{
//	float slope1 =
//	float slope2

//  slope = with every w units of change, y must be modified w * slope units
//  but really we are using negative slope

	mglPushStates ();

	MeglDisable (GL_TEXTURE_2D);

	if (red+green+blue == 0)
		MeglColor4f (red, green, blue, 0.4);
	else
		MeglColor4f (red, green, blue, 0.2);

	MeglEnable (GL_BLEND);
	MeglDisable(GL_ALPHA_TEST);
//	eglBlendFunc (GL_SRC_ALPHA_SATURATE, GL_ONE);
//	MeglEnable (GL_POLYGON_SMOOTH);

	mglFinishedStates ();

	eglBegin (GL_POLYGON);

	eglVertex2f (x, y);
	eglVertex2f (x + w, y + slope * w);
	eglVertex2f (x + w, y + h + slope * w );
	eglVertex2f (x, y + h);

	eglEnd ();
#if 0
	{

//		eglLineWidth (1);
		eglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		MeglEnable (GL_LINE_SMOOTH);
		eglBegin (GL_LINE_LOOP);
		eglVertex2f (x, y);
		eglVertex2f (x + w, y + slope * w);
		eglVertex2f (x + w, y + h + slope * w );
		eglVertex2f (x, y + h);
		eglEnd ();
	}
#endif
//	MeglDisable (GL_POLYGON_SMOOTH);
	MeglEnable(GL_ALPHA_TEST);
	MeglDisable (GL_BLEND);
	MeglColor3ubv (color_white);
	MeglEnable (GL_TEXTURE_2D);

	mglPopStates ();
}

void GLDRAW_Circle2 (float x, float y, float radius, GLfloat red, GLfloat green, GLfloat blue)
{
	float angle;

	mglPushStates ();

	MeglDisable (GL_TEXTURE_2D);
	MeglColor3f (red, green, blue);

	// Draw the anti-aliased outline

	MeglEnable (GL_BLEND);
	MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	MeglEnable (GL_LINE_SMOOTH);

	mglFinishedStates ();

	eglBegin (GL_LINE_LOOP);
	for (angle = 0; angle < M_PI * 2; angle += 0.01)
		eglVertex2f (x + sinf(angle) * radius, y + cosf(angle) * radius);

	eglEnd ();

	MeglDisable (GL_LINE_SMOOTH);
	MeglDisable (GL_BLEND);

	// End
	MeglColor3ubv (color_white);		// Restore to white
	MeglEnable (GL_TEXTURE_2D);		// Restore

	mglPopStates ();

}

#if 0
void GLDRAW_CircleOutlined (float x, float y, float radius, GLfloat red, GLfloat green, GLfloat blue, float linewidth)
{
	float angle;

	mglPushStates ();

	MeglDisable (GL_TEXTURE_2D);
	MeglColor3f (red, green, blue);

	// Draw the anti-aliased outline

	MeglEnable (GL_BLEND);
	MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	eglLineWidth (linewidth);

	mglFinishedStates ();

	eglBegin (GL_LINE_LOOP);
	for (angle = 0; angle < M_PI * 2; angle += 0.01)
		eglVertex2f (x + sinf(angle) * radius, y + cosf(angle) * radius);

	eglEnd ();

//	eglLineWidth (1);
//	MeglDisable (GL_LINE_SMOOTH);
	MeglDisable (GL_BLEND);

	// End
	MeglColor3ubv (color_white);		// Restore to white
	MeglEnable (GL_TEXTURE_2D);		// Restore

	mglPopStates ();
}
#endif

void GLDRAW_CircleFilled (float x, float y, float radius, GLfloat red, GLfloat green, GLfloat blue, qbool antialias)
{
	float angle;

	mglPushStates ();
	MeglDisable (GL_TEXTURE_2D);
	MeglColor3f (red, green, blue);

	mglFinishedStates ();

	// Draw solid circle
	eglBegin(GL_POLYGON);
	for (angle = 0; angle < M_PI * 2; angle += 0.01)
	{
//		float pct = (angle/(M_PI * 2));
//		MeglColor3f (red * pct, green, blue);
		MeglColor3f (red, green, blue);
		eglVertex2f (x + sinf(angle) * radius, y + cosf(angle) * radius);
	}

	eglEnd ();

	MeglColor3ubv (color_white);		// Restore to white
	MeglEnable (GL_TEXTURE_2D);		// Restore


//	if (antialias)
//		GLDRAW_Circle (x, y, radius, red, green, blue);

	mglPushStates ();

}

void GLDRAW_Speedometer (float x, float y, float radius, float mspeed)
{
	float speed = CLAMP (0, mspeed, 480);
	float angle;
	char str[16];
	snprintf (str, sizeof(str), "%3d", (int)CLAMP (0, mspeed, 999));

	GLDRAW_CircleFilled (x, y, radius+2 , 0, 0, 0, true);
	GLDRAW_CircleFilled (x, y, radius, .04, .01, 0, true);

    // Speed marks
	{

		// This looks a little ugly, I'll explain ...
		float startangle =  0.125;		 // We are starting the marks at 12.5% left of bottom center
		float endangle = 1 - startangle; // We are ending the marks at 12.5% right of bottom center
		float usableRange = 0.75;        // Our usable range is 75% of the diameter

		mglPushStates ();
		MeglDisable (GL_TEXTURE_2D);

		MeglEnable (GL_BLEND);
		MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		MeglColor3f (.12, .03, 0);  // Mostly whiteish

		if (engine.Renderer->graphics_api != RENDERER_DIRECT3D) // glLineWidth isn't supported in the wrapper
			eglLineWidth (3); // #ifndef DX8QUAKE_NO_LINEWIDTH

		mglFinishedStates ();

		eglBegin (GL_LINES);

		for (angle = startangle; angle < endangle; angle += (usableRange/10))
		{
			//MessageBox (NULL, va("startangle %2.4f endangle %2.4f angle %2.4f", startangle, endangle, angle) , "Done", 0);
			eglVertex2f (x + sinf(angle * D_PI) * radius * usableRange, y + cosf(angle * D_PI) * radius * usableRange);  // We start the line inside a little
			eglVertex2f (x + sinf(angle * D_PI) * radius, y + cosf(angle * D_PI) * radius);  // To the end point at the edge
		}
		eglEnd ();
	}

	// Needle
	{
		angle= (speed / 480); // Speed percent
		angle = angle * .75 +.125;	  // Only 80% is usable range
		angle = 1 - angle;	  // Angle is anti-angle
		//angle = angle + .1;	  // Floor .1, max .9

		if (engine.Renderer->graphics_api != RENDERER_DIRECT3D) // glLineWidth isn't supported in the wrapper
			eglLineWidth (1); // #ifndef DX8QUAKE_NO_LINEWIDTH

		//MeglColor3f (0.5, 0, 0); // Red
		MeglColor3f (0.9, 0.35, 0.2); // Red
		eglBegin (GL_LINES);

	   // ticker range is constrained to 75% of radius
		//eglVertex2f (x + sinf(angle * D_PI) * radius * 0.125, y + cosf(angle* D_PI) * radius * 0.125);
		eglVertex2f (x, y);
		eglVertex2f (x + sinf(angle * D_PI) * radius * 0.875, y + cosf(angle* D_PI) * radius * 0.875);

		eglEnd ();
	}

	MeglColor3f (1,1,1);
	MeglDisable (GL_BLEND);
	MeglEnable (GL_TEXTURE_2D);

	{
		float xleft = x - (8* 1.5 ) +1;
		float ytop =  y + radius -(8+4) -1;
		Draw_Fill (xleft-1, ytop, 24+2, 8-1+2, 0);
		Draw_String (xleft, ytop, str);
	}

//	{
//		char str[16];
//		snprintf (str, sizeof(str), "%3d", (int)speed);
//		Draw_String (x, y, str);
//		Draw_String (0, 0, str);
//	}

}


void Viewport_PolyBlend_RGBAfv (const GLfloat *v)
{


	if (!scene_polyblend.integer)		return;
	if (cls.state != ca_connected)	return;
	if (!v[3])	return;		// No alpha so no point

	// Don't render view blends if we aren't connected or we have gl_polyblend 0

	mglPushStates ();
	MeglDisable (GL_ALPHA_TEST);
	MeglEnable (GL_BLEND);
	MeglDisable (GL_TEXTURE_2D);

	MeglColor4f (v[0],  v[1],  v[2], v[3]);

	mglFinishedStates ();

	eglBegin (GL_QUADS);
	eglVertex2f (r_refdef.vrect.x, r_refdef.vrect.y);
	eglVertex2f (r_refdef.vrect.x + r_refdef.vrect.width, r_refdef.vrect.y);
	eglVertex2f (r_refdef.vrect.x + r_refdef.vrect.width, r_refdef.vrect.y + r_refdef.vrect.height);
	eglVertex2f (r_refdef.vrect.x, r_refdef.vrect.y + r_refdef.vrect.height);
	eglEnd ();

	MeglDisable (GL_BLEND);
	MeglEnable (GL_TEXTURE_2D);
	MeglEnable (GL_ALPHA_TEST);

	MeglColor3ubv (color_white);

	mglPopStates ();


}



