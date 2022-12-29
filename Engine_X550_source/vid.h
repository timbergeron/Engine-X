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
// vid.h -- video driver defs

#ifndef __VID_H__
#define __VID_H__

#define VID_CBITS	6
#define VID_GRADES	(1<<VID_CBITS)

// a pixel can be one, two, or four bytes
typedef	byte	pixel_t;

typedef struct vrect_s
{
	int		x, y, width, height;
//	struct vrect_s	*pnext;
} vrect_t;

typedef struct
{
	pixel_t			*colormap;			// 256 * VID_GRADES size
	int				fullbright;			// index of first fullbright color
	unsigned		width;
	unsigned		height;
	float			aspect;				// width / height -- < 0 is taller than wide
	int				numpages;
	int				recalc_refdef;		// if true, recalc vid-based stuff
	unsigned		conwidth;
	unsigned		conheight;

	// Baker: keep this up-to-date please.  Like if they ALT-TAB out or what not
	int				DesktopBPP;			// query this @ startup and when entering the video menu
	int				DesktopWidth;		// Of active monitor, right?
	int				DesktopHeight;		// Of active monitor, right?
	int 			DesktopDispFreq;	// Display frequency

} viddef_t;

extern	viddef_t	vid;			// global video state
extern	unsigned	d_8to24table[256];
extern void (*vid_menudrawfn)(void);
extern void (*vid_menukeyfn)(int key);

#if SUPPORTS_GLVIDEO_MODESWITCH
extern void (*vid_menucmdfn)(void); //johnfitz
//void VID_SyncCvars (void);
void VID_SyncCvarsToMode (int myMode);
#endif

void TexMgr_BuildPalette (unsigned char *palette);		// called at startup and after any gamma correction

extern int vid_modenum, vid_default;

// vid_wgl.c

// Called at startup to set up translation tables, takes 256 8 bit RGB values
// the palette data will go away after the call, so it must be copied off if
// the video driver will need it again

void VID_Init (void /*unsigned char *palette*/);
void VID_Shutdown (void);					// Called at shutdown

// sets the mode; only used by the Quake engine for resetting to mode 0 (the
// base mode) on memory allocation failures

int VID_SetMode (const int modenum, qbool isReset /*, unsigned char *palette*/);

// vid_gamma.c
//extern	qbool vid_hwgamma_enabled;


void Gamma_SetDeviceGammaRamp (unsigned short *ramps);
hwgamma_com_t *Gamma_Init (void);
void Gamma_Shutdown (void);


void Gamma_RestoreHWGamma (void);
void Gamma_SetUserHWGamma (void);
void Gamma_Apply_Maybe (void);

void Gamma_ApplyHWGammaToImage (byte *buffer, const int imagesize, const int BytesPerPixel);

void Gamma_Maybe_Update (void);

// vid_wgl.c
void VID_Consize_f(void); // For now ...



#if SUPPORTS_GLVIDEO_MODESWITCH
qbool VID_WindowedSwapAvailable(void);
qbool VID_isFullscreen(void);
void VID_Windowed(void);
void VID_Fullscreen(void);
#endif

void CheckVsyncControlExtensions (void);

extern const float vid_max_contrast, vid_min_gamma;
extern const float vid_min_contrast, vid_max_gamma;

extern qbool vid_isLocked;

// Find REAL home for this
typedef enum 
{
	MODE_WINDOWED, MODE_FULLSCREEN, NO_MODE
} modestate_t;

extern modestate_t	modestate;

#endif // __VID_H__

