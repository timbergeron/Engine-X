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
// vid_gamma.c -- Hardware gamma control

#include "quakedef.h"
#include "winquake.h"


hwgamma_com_t hwgamma;

extern HDC		maindc;
unsigned short	ramps[3][256];

// Baker: ALL gamma changes pass through here, right?  Well no.  Disgusting.
//Note: ramps must point to a static array
void Gamma_SetDeviceGammaRamp (unsigned short *ramps)
{
	if (hwgamma.gammaworks)
	{
		hwgamma.currentgammaramp = ramps;
		if (hwgamma.hwgamma_enabled)
		{
			SetDeviceGammaRamp (maindc, ramps);
			Con_DevPrintf (DEV_GAMMA, "Set in-game\n");
			hwgamma.customgamma = true;
		}
	}
}


void Gamma_Apply_Maybe (void)
{
	static qbool	old_hwgamma_enabled=false;;

	hwgamma.hwgamma_enabled = vid_brightness_method.integer && hwgamma.gammaworks && ActiveApp && !Minimized;
	hwgamma.hwgamma_enabled = hwgamma.hwgamma_enabled && (modestate == MODE_FULLSCREEN || vid_brightness_method.integer == 2);
	if (hwgamma.hwgamma_enabled != old_hwgamma_enabled)
	{
		old_hwgamma_enabled = hwgamma.hwgamma_enabled;
		if (hwgamma.hwgamma_enabled && hwgamma.currentgammaramp)
			Gamma_SetUserHWGamma ();
		else
			Gamma_RestoreHWGamma ();
	}
}



void Gamma_SetUserHWGamma (void)
{
	if (hwgamma.currentgammaramp)
	{
		Gamma_SetDeviceGammaRamp (hwgamma.currentgammaramp);
		Con_DevPrintf (DEV_GAMMA, "Set User Gamma (startup, gain focus, etc.)\n");
	}
}


void Gamma_RestoreHWGamma (void)
{
	if (hwgamma.gammaworks && hwgamma.customgamma)
	{
		hwgamma.customgamma = false;
		SetDeviceGammaRamp (maindc, hwgamma.systemgammaramp);
		Con_DevPrintf (DEV_GAMMA, "Restore Gamma (shutdown, lose focus, etc.)\n");
	}
}


extern	unsigned short	ramps[3][256];

// Unfortunately, view blends are rolled into this.
void Gamma_ApplyHWGammaToImage (byte *buffer, const int imagesize, const int BytesPerPixel)
{
	int	i;

	if (!engine.HWGamma->hwgamma_enabled)
		return;

	for (i=0 ; i<imagesize ; i+=BytesPerPixel)
	{
		buffer[i+0] = ramps[0][buffer[i+0]] >> 8;
		buffer[i+1] = ramps[1][buffer[i+1]] >> 8;
		buffer[i+2] = ramps[2][buffer[i+2]] >> 8;
		// Note: Even if BPP is 4, we don't apply gamma to the alpha channel obviously

	}
}


/*
================
VID_Gamma_Shutdown -- called on exit
================
*/
void Gamma_Shutdown (void)
{
	if (maindc)
	{
		if (hwgamma.gammaworks)
			if (SetDeviceGammaRamp(maindc, hwgamma.systemgammaramp) == false)
				Con_Printf ("Gamma_Shutdown: failed on SetDeviceGammaRamp\n");
	}
}

hwgamma_com_t *Gamma_Init (void)
{
	hwgamma.gammaworks = false;

	if (COM_CheckParm("-nohwgamma"))
	{
		Con_Warning ("Hardware gamma disabled via command line\n");
		goto failcont;
	}

	if (!GetDeviceGammaRamp (maindc, hwgamma.systemgammaramp))
	{
		Con_Warning ("Hardware gamma not available (GetDeviceGammaRamp failed)\n");
		goto failcont;
	}

//	vid_max_contrast = 2.0f;
//	vid_min_gamma    = 0.5f;

	Con_Success ("Hardware gamma enabled\n");
	hwgamma.gammaworks = true;

failcont:

	return &hwgamma;
}


/*
const float vid_max_contrast = 4.0; // hw gamma limit is 2.0, we'll adjust for that
const float vid_min_contrast = 1.0;
const float vid_max_gamma    = 1.0;	// Could extend to 1.6 or even 2.0
const float vid_min_gamma    = 0.5;	// 0.30 should be acceptable but Windows no like
*/

void Gamma_Maybe_Update (void)
{
	static	float	previous_gamma, previous_contrast;
//	float			effective_contrast  = ((vid_brightness_contrast.floater-1) / 3) + 1;			// Baker: Translate contrast range of 1.0-4.0 to 1.0-2.0
	float			effective_contrast  = ((vid_brightness_contrast.floater-1) / 3) + 1;			// Baker: Translate contrast range of 1.0-4.0 to 1.0-2.0

	float			current_gamma		= CLAMP (vid_min_gamma, vid_brightness_gamma.floater,    vid_max_gamma);
	float			current_contrast	= CLAMP (vid_min_contrast,   effective_contrast, (vid_max_contrast/2));
	static qbool	x = false;

	// If nothing changed get out
	if (current_gamma == previous_gamma && current_contrast == previous_contrast) return;

	// Store the new values
	previous_gamma		= current_gamma;
	previous_contrast	= current_contrast;

	// Calculate ramps
	{
		// Adjust for palette gamma

		float			net_contrast = pow (current_contrast, vid_palette_gamma);
		float			net_gamma    = current_gamma/vid_palette_gamma;

		int				i, j, c;

//		Test
//		net_gamma += .3;

		for (i=0 ; i<256 ; i++)		// For each color intensity 0-255
		{
			for (j=0 ; j<3 ; j++)	// For each color RGB
			{

				// apply contrast
				c = i * net_contrast;
				if (c > 255)	c = 255;							// limit contrast effect to 255 since this is a byte percent

				// apply gamma
				c = 255 * pow((c + 0.5)/255.5, net_gamma) + 0.5;
				c = CLAMP (0, c, 255);								// limit to 0-255 range

				ramps[j][i] = c << 8;	// Divide by 256
			}
		}

		Gamma_SetDeviceGammaRamp ((unsigned short *)ramps);
	}


}


