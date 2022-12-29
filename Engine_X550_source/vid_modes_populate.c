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
// vid_modes_populate.c -- Builds the video modes table. This file is about the modes available.  Not the current mode.

#include "quakedef.h"
#include "winquake.h"
#include <windows.h>
#include "winquake_video_modes.h"


#define VIDEO_MODES_MAX_MODE_LIST	600
#define VIDEO_MODES_MAXWIDTH		10000
#define VIDEO_MODES_MAXHEIGHT		10000

vmode_t	video_modes[MAX_MODE_LIST];
int		num_video_modes;

vmode_t	badmode;

/*
typedef struct {
	int		width;
	int		height;
} lmode_t;

lmode_t	lowresmodes[] = {
	{320, 200},
	{320, 240},
	{400, 300},
	{512, 384},
};
*/

void Rebuild_Mode_Description (int myMode)
{

	snprintf (video_modes[myMode].modedesc, sizeof(video_modes[myMode].modedesc), "%dx%dx%d %dHz", //johnfitz -- added bpp, refreshrate
			 video_modes[myMode].width,
			 video_modes[myMode].height,
			 video_modes[myMode].bpp, //johnfitz -- added bpp
			 video_modes[myMode].refreshrate); //johnfitz -- added refreshrate
}


void VID_AddBasicWindowedMode (void)
{
	// Build Mode 0 Windowed Default
	// MODE_WINDOWED MODE
	video_modes[0].displaymode = MODE_WINDOWED;

	video_modes[0].width =			640;	// engine.application.DesktopWidth;
	video_modes[0].height =			480;	// engine.application.DesktopHeight;
	video_modes[0].bpp =			vid.DesktopBPP;
	video_modes[0].refreshrate =	vid.DesktopDispFreq; // Well ...
	video_modes[0].displaymode =	MODE_WINDOWED;

	// Populate the description
	Rebuild_Mode_Description (0);

	num_video_modes++;  // Now this should be 1
}

void VID_AddBasicFullScreenMode (void)
{
	qbool user_mode_forced=false;

	// Build Mode 1 Fullscreen Default
	// MODE_FULLSCREEN_DEFAULT
	video_modes[1].displaymode =		MODE_FULLSCREEN;

	video_modes[1].width =				640;	// engine.application.DesktopWidth;
	video_modes[1].height =				480;	// engine.application.DesktopHeight;
	video_modes[1].bpp =				vid.DesktopBPP;
	video_modes[1].refreshrate	=		vid.DesktopDispFreq; // Well ...

	// Populate the description

	Rebuild_Mode_Description (1);

	num_video_modes++;  // Now this should be 2
}


/*
=================
VID_AddAllFullScreen_Modes
=================
*/
void VID_AddAllFullScreen_Modes (void)
{

	BOOL		stat;									// Used to test mode validity
	int			originalnummodes = num_video_modes;		// Should be 1 or 2 at this point
	qbool	mode_already_exists;
	int			k;


	for (k = 1; k<3; k++)  // Baker: Round 1: Add normal modes.  Round 2: Add low res modes.
	{
		int			this_hardware_modenum = 0;				// Hardware modes start at 0

		do
		{
			// Baker: Run through every display mode and get information
			DEVMODE	devmode;
					qbool is_low_res;

			stat = EnumDisplaySettings (NULL, this_hardware_modenum, &devmode);

					is_low_res = (devmode.dmPelsWidth < 640 || devmode.dmPelsHeight< 480);

			if (devmode.dmBitsPerPel	>=  15                       &&
			    devmode.dmPelsWidth		<=  VIDEO_MODES_MAXWIDTH     &&
			    devmode.dmPelsHeight	<=  VIDEO_MODES_MAXHEIGHT    &&
						num_video_modes			<   VIDEO_MODES_MAX_MODE_LIST &&
						(    (k == 1 && !is_low_res) || (k==2 && is_low_res))  )
			{

				// Mode is acceptable (meets bpp requirements, isn't too big, etc.)
				devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY; //johnfitz -- refreshrate

				if (eChangeDisplaySettings (&devmode, CDS_TEST | CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
				{
					int i;

					video_modes[num_video_modes].displaymode = MODE_FULLSCREEN;

					video_modes[num_video_modes].width = devmode.dmPelsWidth;
					video_modes[num_video_modes].height = devmode.dmPelsHeight;
					video_modes[num_video_modes].bpp = devmode.dmBitsPerPel;
					video_modes[num_video_modes].refreshrate = devmode.dmDisplayFrequency; //johnfitz -- refreshrate

					// Build description
					Rebuild_Mode_Description (num_video_modes);

					// Cycle through the existing modes
					// This also allows us to honor whatever mode was added with -width/-height as mode #1
					for (i=originalnummodes, mode_already_exists = false ; i<num_video_modes ; i++)
					{
						if (video_modes[num_video_modes].width == video_modes[i].width && video_modes[num_video_modes].height == video_modes[i].height && video_modes[num_video_modes].bpp == video_modes[i].bpp && video_modes[num_video_modes].refreshrate == video_modes[i].refreshrate)
						{
							// Mode already exists
							mode_already_exists = true;
							break;
						}
					}

					if (!mode_already_exists)
						num_video_modes++;
				}
			}

			this_hardware_modenum++;
		} while (stat);
	}

	if (num_video_modes == originalnummodes)
		Con_SafePrintf ("No fullscreen DIB modes found\n");

}


extern int vid_modenum;
extern int vid_default;

//==========================================================================
//
//  COMMANDS
//
//==========================================================================

/*
=================
VID_NumModes
=================
*/
int VID_NumModes (void)
{
	return num_video_modes;
}


/*
=================
VID_GetModePtr
=================
*/
vmode_t *VID_GetModePtr (int modenum)
{

	if ((modenum >= 0) && (modenum < num_video_modes))
		return &video_modes[modenum];

	return &badmode;
}


/*
=================
VID_GetModeDescription
=================
*/
char *VID_GetModeDescription (int mode)
{
	char		*pinfo;
	vmode_t		*pv;
	static	char	temp[100];

	if (mode < 0 || mode >= num_video_modes)
		return NULL;

//	if (!leavecurrentmode)
//	{
		pv = VID_GetModePtr (mode);
		pinfo = pv->modedesc;
//	}
//	else
//	{
//		snprintf (temp, sizeof(temp), "Desktop resolution (%ix%ix%i)", //johnfitz -- added bpp
//				 video_modes[MODE_FULLSCREEN_DEFAULT].width,
//				 video_modes[MODE_FULLSCREEN_DEFAULT].height,
//				 video_modes[MODE_FULLSCREEN_DEFAULT].bpp); //johnfitz -- added bpp
//		pinfo = temp;
//	}

	return pinfo;
}

char *VID_CurModeShort (void)
{
	static	char	pinfo[40];

	snprintf (pinfo, sizeof(pinfo), "%ix%i", video_modes[vid_default].width, (int)video_modes[vid_default].height);
	return pinfo;
}

// KJB: Added this to return the mode driver name in description for console
/*
=================
VID_GetExtModeDescription
=================
*/
char *VID_GetExtModeDescription (int mode)
{
	static	char	pinfo[40];
	vmode_t		*pv;

	if (mode < 0 || mode >= num_video_modes)
		return NULL;

	pv = VID_GetModePtr (mode);
	if (video_modes[mode].displaymode == MODE_FULLSCREEN)
	{
//		if (!leavecurrentmode)
//		{
			snprintf (pinfo, sizeof(pinfo),  "%s fullscreen", pv->modedesc);
//		}
//		else
//		{
//			snprintf (pinfo, sizeof(pinfo),  "Desktop resolution (%ix%ix%i)", //johnfitz -- added bpp
//					 video_modes[MODE_FULLSCREEN_DEFAULT].width,
//					 video_modes[MODE_FULLSCREEN_DEFAULT].height,
//					 video_modes[MODE_FULLSCREEN_DEFAULT].bpp); //johnfitz -- added bpp
//		}
	}
	else
	{
		if (modestate == MODE_WINDOWED)
			snprintf (pinfo, sizeof(pinfo),  "%s windowed", pv->modedesc);
		else
			snprintf (pinfo, sizeof(pinfo),  "windowed");
	}

	return pinfo;
}

/*
=================
VID_DescribeCurrentMode_f
=================
*/
void VID_DescribeCurrentMode_f (void)
{
	Con_Printf ("%s\n", VID_GetExtModeDescription(vid_modenum));
}

/*
=================
VID_DescribeModes_f -- johnfitz -- changed formatting, and added refresh rates after each mode.
=================
*/
void VID_DescribeModes_f (void)
{
	int	i, lnummodes;//, t;
//	char		*pinfo;
	vmode_t		*pv;
	int			lastwidth=0, lastheight=0, lastbpp=0, count=0;

	lnummodes = VID_NumModes ();

//	t = leavecurrentmode;
//	leavecurrentmode = 0;

	for (i=0 ; i<lnummodes ; i++)
	{
		pv = VID_GetModePtr (i);
//		if (lastwidth == pv->width && lastheight == pv->height && lastbpp == pv->bpp)
//		{
//			Con_SafePrintf (" @ %i Hz", pv->refreshrate);
//		}
//		else
		{
			if (count>0)
				Con_SafePrintf ("\n");
			Con_SafePrintf ("%2i: %4i x %4i x %i : %i Hz", i, pv->width, pv->height, pv->bpp, pv->refreshrate);
			lastwidth = pv->width;
			lastheight = pv->height;
			lastbpp = pv->bpp;
			count++;
		}
	}
	Con_Printf ("\n%i modes\n", count);

//	leavecurrentmode = t;
}



int VID_FindMode (int myWidth, int myHeight, int myBpp, int myFreq, qbool bExactMatchOnly)
{
	int i;


	// Baker: Locate matching mode
	for (i=1; i<num_video_modes; i++)
	{
		if (video_modes[i].width == myWidth &&
			video_modes[i].height == myHeight &&
			video_modes[i].bpp == myBpp &&
			video_modes[i].refreshrate == myFreq)
		{
			break;
		}
	}


	// We didn't find one
	if (i == num_video_modes)
	{
		Con_DevPrintf (DEV_VIDEO, "VID_FindMode: No matching mode, using default fullscreen mode.\n");
		return 1; // / Screw it ... they get to use fullscreen mode 1
	}

	return i;

}


int VID_FindFullScreenModeOnCvars (void) // qbool UseCvars, qbool UseCurrentMode, qbool ExactMatchOnly)
{
	int t = VID_FindMode ((int)vid_width.integer, (int)vid_height.integer, (int)vid_bpp.integer, (int)vid_displayfrequency.integer, false /*wants Exact match only*/);

	return t;
}