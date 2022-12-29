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
// vid_paint_screen.c -- Early drawing

#include "quakedef.h"

#ifdef _WIN32
#include "winquake.h"

#include "resource.h"
//#include <commctrl.h>
#include "winquake_video_modes.h"
#endif

typedef struct 
{
	int canvas_width;
	int canvas_height;

	int checker_x_size;
	int checker_y_size;
	int checker_count_cols;
	int checker_count_rows;

} intro_def_x;

static intro_def_x intro;

void VID_Intro_Setup (void)
{
	int k;
	// GL_SetupState

	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	intro.canvas_width =  glwidth;
	intro.canvas_height = glheight;

	// Locate largest multiple of 8 that divides wholly
	for (k = 8; k >= 1; k --)
		if (intro.canvas_width / (8 * k) == (float)(intro.canvas_width / (8.0f * (float)k)))	// Is whole number?
				break;

	intro.checker_x_size = k*8, intro.checker_y_size = k * 8 /*48*/;
	intro.checker_count_cols = intro.canvas_width  / intro.checker_x_size;
	intro.checker_count_rows = intro.canvas_height / intro.checker_y_size;


	// Do our setup state
	MeglClearColor (1, 1, 1, 1);
	MeglDisable (GL_TEXTURE_2D);
}

//Baker: The VID_Intro_Frame provides 3 tools for animations
//       This frame will run from 0% to 100% done
//
//       Tool #1:  Percent done  -  goes from 0.0                 to 1.0 (at 100%) at a constant pace (obviously)
//       Tool #2:  Sine Curve    -  goes from 0.0 to 1.0 (at 50%) to 0.0 (at 100%) with a sine curve.
//       Tool #3:  Accelerator   -  goes from 0 0                 to 1.0 (at 100%) but starts slowly and goes fast towards end
void VID_Intro_Frame (float percent_done)
{
	float accelerator = percent_done * percent_done * percent_done;
	float sine_curve  = sinf((percent_done *90.0f) / 90.0f * M_PI);

	float magnification;

	magnification = -sine_curve * .05;


	// Frame setup
	eglClear (GL_COLOR_BUFFER_BIT);
    eglMatrixMode (GL_PROJECTION);
	eglLoadIdentity ();
	eglOrtho(0 - (intro.canvas_width * magnification), intro.canvas_width + (intro.canvas_width * magnification), intro.canvas_height + (intro.canvas_height * magnification), 0 - (intro.canvas_height * magnification), 0, 999);

//	Baker: We'd need to activate multisample for this to look any good (gots jaggies)
//	eglRotatef (3   * sine_curve, 0, 0, 1);
//	eglRotatef (0.5 * sine_curve, 0, 1, 0);

//	eglTranslatef (0,0, -3 *sine_curve);

	{	int i, j;
		mglPushStates (); eglPushMatrix ();

		mglFinishedStates ();
		for (i = 0; i<intro.checker_count_cols; i++)
			for (j = 0; j<intro.checker_count_rows+1; j++)
			{
				int x1 = i * intro.checker_x_size;  // 128 = left start, 32 = tile size
				int y1 = j * intro.checker_y_size; // 128 = top start, 32 = tile size
				int x2 = (i + 1) * intro.checker_x_size;
				int y2 = (j + 1) * intro.checker_y_size;


				if ((i+j) & 1)
					MeglColor3f (0.15 + 0.85 * sine_curve,      0.15 * accelerator,    0.15 * accelerator);
				else
					MeglColor3f (1    - 0.85 * accelerator, 1 - 0.85 * accelerator, 1- 0.85 * accelerator);

				eglBegin (GL_QUADS);
					eglVertex2f (x1, y1);
					eglVertex2f (x2, y1);
					eglVertex2f (x2, y2);
					eglVertex2f (x1, y2);
				eglEnd ();
			}
			eglPopMatrix (); mglPopStates ();
	}


#ifdef _WIN32
	// Swap the buffer
#if RENDERER_DIRECT3D_AVAILABLE
	if (engine.Renderer->graphics_api == RENDERER_DIRECT3D) // #if defined(DX8QUAKE)
		FakeSwapBuffers();
	else
#endif
		SwapBuffers (maindc);
#endif

	return;
}

void VID_Intro_Cleanup (void)
{
	// Restore to GL_SetupState
	MeglClearColor (0, 0, 0, 0);
	MeglEnable (GL_TEXTURE_2D);
}

void VID_Intro_Run (void)
{
	const float seconds_duration = 2; // first_time_engine_used ? 2 : 0.250;	// One quarter of a second
	const float starttime = Sys_DoubleTime ();
	const float finishedtime = starttime + seconds_duration;
	float		newtime;

	VID_Intro_Setup ();

	// FPS regulator

	do
	{
		static			last_frame_time;
		const float		maxfps_for_frame = 200;
		newtime = Sys_DoubleTime ();

		// Don't sleep during a capturedemo (CPU intensive!) or during a timedemo (performance testing)
		if ( newtime - last_frame_time < 1.0 / maxfps_for_frame)
		{
			// Frame rate is too high
			Sys_Sleep ();
			continue;
		}

		// Run a frame
		last_frame_time = newtime;
		{
			float			pct = CLAMP(0, (newtime - starttime)/(finishedtime - starttime), 1);
			VID_Intro_Frame (pct);
		}

	} while (newtime < finishedtime);


	VID_Intro_Cleanup ();
}