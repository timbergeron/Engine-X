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
// vid_shared.c -- Generic operating system neutral stuff

#include "quakedef.h"


qbool	vid_isLocked = false; // On assignment, use function call
modestate_t	modestate = NO_MODE;	// This is only about 99.9% in-sync with whether or not we are in windowed mode
									// And trying to 100% sync it seems to break stuff


const float vid_max_contrast = 4.0; // hw gamma limit is 2.0, we'll adjust for that
const float vid_min_contrast = 1.0;
const float vid_max_gamma    = 1.0;	// Could extend to 1.6 or even 2.0
const float vid_min_gamma    = 0.5;	// 0.30 should be acceptable but Windows no like


/*
================
R_BrightenScreen
================
*/
// If we aren't using hardware gamma and contrast is under 1, draw a poly over screen
//extern hwgamma_com_t hwgamma;
void R_BrightenScreen (void)
{
//	extern	float	vid_palette_gamma;
	float		f;


	if (engine.HWGamma && engine.HWGamma->hwgamma_enabled)
		return;

	// Baker: We don't care if it is enabled, just if we are using it?
//	if (hwgamma.hwgamma_enabled)	return;		// Because we aren't using "poly gamma"
//	if (vid_brightness_contrast.floater <= 1)	return;		// Because it won't do anything, 1 to any power is 1


	{	// Range limit cvars
		float effective_gamma =		CLAMP (vid_min_gamma, vid_brightness_gamma.floater, vid_max_gamma);
		float effective_contrast =	CLAMP (vid_min_contrast, vid_brightness_contrast.floater, vid_max_contrast);

		if (!(vid_brightness_gamma.floater == effective_gamma))			Cvar_SetFloatByRef (&vid_brightness_gamma, effective_gamma);
		if (!(vid_brightness_contrast.floater == effective_contrast))	Cvar_SetFloatByRef (&vid_brightness_contrast, effective_contrast);

		// Translate gamma to range

		effective_gamma = (effective_gamma-vid_min_gamma)/(vid_max_gamma-vid_min_gamma)*0.95 + 0.05;

		f = vid_brightness_contrast.floater;
		f = powf	(f, 1 - effective_gamma); // Baker: change to vid_hwgamma


	}

	mglPushStates ();

	// Baker: This whole function is VERY slow

	MeglDisable (GL_TEXTURE_2D);
	MeglEnable (GL_BLEND);
	MeglBlendFunc (GL_DST_COLOR, GL_ONE);

	mglFinishedStates ();

	eglBegin (GL_QUADS);
	while (f > 1)
	{
		if (f >= 2)
			MeglColor3ubv (color_white);
		else
			MeglColor3f (f - 1, f - 1, f - 1);

		// Baker: Remember, this is using whatever canvas we have setup!!!!!!
		eglVertex2f (0, 0);
		eglVertex2f (vid.width, 0);				// Baker: shouldn't this be glwidth and glx gly and so forth? NO.  Canvas!
		eglVertex2f (vid.width, vid.height);	// Baker: This does need to be the entire screen.  Why did you think canvas?
		eglVertex2f (0, vid.height);			// Baker: However we need to set the viewport to full client area here
		f *= 0.5;
	}
	eglEnd ();

	MeglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	MeglEnable (GL_TEXTURE_2D);
	MeglDisable (GL_BLEND);
	MeglColor4ubv (color_white);

	mglPopStates ();
}