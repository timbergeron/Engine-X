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
// movie.c -- video capturing

#include "quakedef.h"

#if SUPPORTS_AVI_CAPTURE
#include "movie.h"
#include "movie_avi.h"

extern	float	scr_con_current;
extern qbool	scr_drawloading;
extern	short	*snd_out;
extern	int	snd_linear_count, soundtime;

// Variables for buffering audio
static short	capture_audio_samples[44100];	// big enough buffer for 1fps at 44100Hz
static int	captured_audio_samples;

static	int	out_size, ssize, outbuf_size;
static	byte	*outbuf, *picture_buf;
static	FILE	*moviefile;

static	float	hack_ctr;



qbool movie_is_capturing = false;
qbool	avi_loaded, acm_loaded;

qbool Movie_IsActive (void)
{
	// don't output whilst console is down or 'loading' is displayed
	if ((!capture_console.integer && scr_con_current > 0) || scr_drawloading)
		return false;

	// otherwise output if a file is open to write to
	return movie_is_capturing;
}

void Movie_Start_f (void)
{
	char	name[MAX_FILELENGTH], path[256];

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("capture_start <filename> : Start capturing to named file\n");
		return;
	}

	StringLCopy (name, Cmd_Argv(1));
	COMD_ForceExtension (name, ".avi");

	hack_ctr = capture_hack.floater;

	snprintf (path, sizeof(path), "%s/%s/%s", com_gamedirfull, "videos", name);

// Make videos folder if it isn't there?

	if (!(moviefile = FS_fopen_write (path, "wb", FS_CREATE_PATH)))
	{
		Con_Printf ("ERROR: Couldn't open %s\n", name);
		return;
	}

	movie_is_capturing = Capture_Open (path);
}

void Movie_Stop (void)
{
	movie_is_capturing = false;
	Capture_Close ();
	fclose (moviefile);
}

void Movie_Stop_f (void)
{
	if (!Movie_IsActive())
	{
		Con_Printf ("Not capturing\n");
		return;
	}

	if (cls.capturedemo)
		cls.capturedemo = false;

	Movie_Stop ();

	Con_Printf ("Stopped capturing\n");
}

void Movie_CaptureDemo_f (void)
{
	if (Cmd_Argc() != 2)
	{
		Con_Printf ("Usage: %s <demoname>\n", Cmd_Argv (0));
		return;
	}

	Con_Printf ("Capturing %s.dem\n", Cmd_Argv(1));

	CL_PlayDemo_f ();
	if (!cls.demoplayback)
		return;

	Movie_Start_f ();
	cls.capturedemo = true;

	if (!movie_is_capturing)
		Movie_StopPlayback ();
}

void Movie_Init (void)
{
	extern char	com_basedir[MAX_OSPATH];
	AVI_LoadLibrary ();
	if (!avi_loaded)
		return;

	captured_audio_samples = 0;

	Cvar_Registration_Client_Movie ();

	Cmd_AddCommand ("capture_start", Movie_Start_f);
	Cmd_AddCommand ("capture_stop", Movie_Stop_f);
	Cmd_AddCommand ("capturedemo", Movie_CaptureDemo_f);


//	Cvar_SetStringByRef (&capture_dir, va("%s/%s", com_basedir, capture_dir.string));

	ACM_LoadLibrary ();
	if (!acm_loaded)
		return;
}

void Movie_StopPlayback (void)
{
	if (!cls.capturedemo)
		return;

	cls.capturedemo = false;
	Movie_Stop ();
//	CL_Disconnect ();
}

double Movie_FrameTime (void)
{
	double	time;

	if (capture_fps.floater > 0)
		time = !capture_hack.floater ? 1.0 / capture_fps.floater : 1.0 / (capture_fps.floater * (capture_hack.floater + 1.0));
	else
		time = 1.0 / 30.0;
	return CLAMP (1.0 / 1000, time, 1.0);
}

void Movie_UpdateScreen (void)
{
#ifdef GLQUAKE
	int	i, size = glwidth * glheight * 3;
	byte	*buffer, temp;
#else
	int	i, j, rowp;
	byte	*buffer, *p;
#endif

	if (!Movie_IsActive())
		return;

	if (capture_hack.floater)
	{
		if (hack_ctr != capture_hack.floater)
		{
			if (!hack_ctr)
				hack_ctr = capture_hack.floater;
			else
				hack_ctr--;
			return;
		}
		hack_ctr--;
	}

	buffer = Q_malloc (size, "Movie buffer");
	eglReadPixels (glx, gly, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	Gamma_ApplyHWGammaToImage (buffer, size, RGB__BYTES_PER_PIXEL_IS_3);

	for (i = 0 ; i < size ; i += 3)
	{
		temp = buffer[i];
		buffer[i] = buffer[i+2];
		buffer[i+2] = temp;
	}


	Capture_WriteVideo (buffer);

	Q_malloc_free (buffer);
}

void Movie_TransferStereo16 (void)
{
	if (!Movie_IsActive())
		return;

	// Copy last audio chunk written into our temporary buffer
	memcpy (capture_audio_samples + (captured_audio_samples << 1), snd_out, snd_linear_count * shm->channels);
	captured_audio_samples += (snd_linear_count >> 1);

	if (captured_audio_samples >= Q_rint(host_frametime * shm->speed))
	{
		// We have enough audio samples to match one frame of video
		Capture_WriteAudio (captured_audio_samples, (byte *)capture_audio_samples);
		captured_audio_samples = 0;
	}
}

qbool Movie_GetSoundtime (void)
{
	if (!Movie_IsActive())
		return false;

	soundtime += Q_rint(host_frametime * shm->speed * (Movie_FrameTime() / host_frametime));

	return true;
}

//qbool OnChange_capture_dir (cvar_t *var, const char *string)
//{
//	if (Movie_IsActive())
//	{
//		Con_Printf ("Cannot change capture_dir whilst capturing. Use 'capture_stop' to cease capturing first.\n");
//		return true;
//	}
//
//	return false;
//}

#endif
