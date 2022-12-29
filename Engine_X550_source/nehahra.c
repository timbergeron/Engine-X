/*
Copyright (C) 2000	LordHavoc, Ender

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
// nehahra.c

#include "quakedef.h"
#ifdef _WIN32
#include "winquake.h"
#endif

#include "fmod/include/fmod.h"
#include "fmod/include/fmod_errors.h"
#ifndef _WIN32 // Header
#include <dlfcn.h>
#endif

int	NehGameType = 0;
static qbool modplaying = false;

// Baker: these are now defined elsewhere

//cvar_t	gl_fogenable	= {"gl_fogenable", "0"};
//cvar_t	gl_fogdensity	= {"gl_fogdensity", "0.8"};
//cvar_t	gl_fogred	= {"gl_fogred","0.3"};
//cvar_t	gl_fogblue	= {"gl_fogblue","0.3"};
//cvar_t	gl_foggreen	= {"gl_foggreen","0.3"};

//float	r_modelalpha;

char	prev_skybox[64];


int	num_sfxorig;
void Neh_CheckMode (void);

FMUSIC_MODULE	*mod = NULL;

static signed char (F_API *qFSOUND_Init)(int, int, unsigned int);
static signed char (F_API *qFSOUND_SetBufferSize)(int);
static int (F_API *qFSOUND_GetMixer)(void);
static signed char (F_API *qFSOUND_SetMixer)(int);
static int (F_API *qFSOUND_GetError)(void);
static void (F_API *qFSOUND_Close)(void);
static FMUSIC_MODULE * (F_API *qFMUSIC_LoadSongEx)(const char *, int, int, unsigned int, const int *, int);
static signed char (F_API *qFMUSIC_FreeSong)(FMUSIC_MODULE *);
static signed char (F_API *qFMUSIC_PlaySong)(FMUSIC_MODULE *);

#ifdef _WIN32 // GetProcAddress
static	HINSTANCE fmod_handle = NULL;
#else
static	void	*fmod_handle = NULL;
#endif
static qbool fmod_loaded;

#ifdef _WIN32 // GetProcAddress
#define FSOUND_GETFUNC(f, g) (qFSOUND_##f = (void *)GetProcAddress(fmod_handle, "_FSOUND_" #f #g))
#define FMUSIC_GETFUNC(f, g) (qFMUSIC_##f = (void *)GetProcAddress(fmod_handle, "_FMUSIC_" #f #g))
#else
#define FSOUND_GETFUNC(f, g) (qFSOUND_##f = (void *)dlsym(fmod_handle, "FSOUND_" #f))
#define FMUSIC_GETFUNC(f, g) (qFMUSIC_##f = (void *)dlsym(fmod_handle, "FMUSIC_" #f))
#endif

void FMOD_LoadLibrary (void)
{
	fmod_loaded = false;

#ifdef _WIN32 // GetProcAddress
	if (!(fmod_handle = LoadLibrary(RELAPATH_TO_FMOD_DLL)))	// Baker: concerned .. relative path... cannot guarantee CWD
#else
	if (!(fmod_handle = dlopen("libfmod-3.73.so", RTLD_NOW)))
#endif
	{
		Con_Printf ("\x02" "FMOD module not found\n");
		goto fail;
	}

	FSOUND_GETFUNC(Init, @12);
	FSOUND_GETFUNC(SetBufferSize, @4);
	FSOUND_GETFUNC(GetMixer, @0);
	FSOUND_GETFUNC(SetMixer, @4);
	FSOUND_GETFUNC(GetError, @0);
	FSOUND_GETFUNC(Close, @0);
	FMUSIC_GETFUNC(LoadSongEx, @24);
	FMUSIC_GETFUNC(FreeSong, @4);
	FMUSIC_GETFUNC(PlaySong, @4);

	fmod_loaded = qFSOUND_Init && qFSOUND_SetBufferSize && qFSOUND_GetMixer &&
			qFSOUND_SetMixer && qFSOUND_GetError && qFSOUND_Close &&
			qFMUSIC_LoadSongEx && qFMUSIC_FreeSong && qFMUSIC_PlaySong;

	if (!fmod_loaded)
	{
		Con_Printf ("\x02" "FMOD module not initialized\n");
		goto fail;
	}

	Con_Printf ("FMOD module initialized\n");
	return;

fail:
	if (fmod_handle)
	{
#ifdef _WIN32 // GetProcAddress
		FreeLibrary (fmod_handle);
#else
		dlclose (fmod_handle);
#endif
		fmod_handle = NULL;
	}
}

void FMOD_Stop_f (void)
{
	if (modplaying)
		qFMUSIC_FreeSong (mod);

	modplaying = false;
}

void FMOD_Play_f (void)
{
	char	modname[256], *buffer;
	int	mark;

	StringLCopy (modname, Cmd_Argv(1));

	if (modplaying)
		FMOD_Stop_f ();

	if (strlen(modname) < 3)
	{
		Con_Printf ("Usage: %s <filename.ext>", Cmd_Argv (0));
		return;
	}

	mark = Hunk_LowMark ();

	if (!(buffer = (char *)QFS_LoadHunkFile(modname, NULL)))
	{
		Con_Printf ("ERROR: Couldn't open %s\n", modname);
		return;
	}

	mod = qFMUSIC_LoadSongEx (buffer, 0, qfs_lastload.filelen, FSOUND_LOADMEMORY, NULL, 0);

	Hunk_FreeToLowMark (mark);

	if (!mod)
	{
		Con_Printf ("%s\n", FMOD_ErrorString(qFSOUND_GetError()));
		return;
	}

	modplaying = true;
	qFMUSIC_PlaySong (mod);
}

void FMOD_Init (void)
{
//	qFSOUND_SetBufferSize (300);
	if (!qFSOUND_Init(11025, 32, 0))
	{
		Con_Printf ("%s\n", FMOD_ErrorString(qFSOUND_GetError()));
		return;
	}

	Cmd_AddCommand ("stopmod", FMOD_Stop_f);
	Cmd_AddCommand ("playmod", FMOD_Play_f);
}

void FMOD_Close (void)
{
	if (fmod_loaded)
		qFSOUND_Close ();
}

void Neh_DoBindings (void)
{
#if 0 // Baker: I don't like this idea
	if (NehGameType == TYPE_DEMO)
	{
		Cbuf_AddText ("bind F1 \"cl_demospeed 0.4\"\n");
		Cbuf_AddText ("bind F2 \"cl_demospeed 0.6\"\n");
		Cbuf_AddText ("bind F3 \"cl_demospeed 0.8\"\n");
		Cbuf_AddText ("bind F4 \"cl_demospeed 1.0\"\n");
		Cbuf_AddText ("bind F5 \"cl_demospeed 1.4\"\n");
		Cbuf_AddText ("bind F6 \"cl_demospeed 1.6\"\n");
		Cbuf_AddText ("bind F7 \"cl_demospeed 1.8\"\n");
		Cbuf_AddText ("bind F8 \"cl_demospeed 2.0\"\n");
		Cbuf_AddText ("bind F9 \"cl_demospeed 2.4\"\n");
		Cbuf_AddText ("bind F10 \"cl_demospeed 2.6\"\n");

		Cbuf_AddText ("bind PAUSE pausedemo\n");
	}
#endif
}

void Neh_Init (void)
{
//	Cvar_RegisterVariable (&gl_fogenable, NULL);
//	Cvar_RegisterVariable (&gl_fogdensity, NULL);
//	Cvar_RegisterVariable (&gl_fogred, NULL);
//	Cvar_RegisterVariable (&gl_foggreen, NULL);
//	Cvar_RegisterVariable (&gl_fogblue, NULL);

//	Cvar_SetFloatByRef (&gl_hwblend, 0);

//	Cmd_AddLegacyCommand ("pausedemo", "pause");

	Cvar_Registration_Extension_Nehahra ();

	Neh_CheckMode ();

//	if (COM_CheckParm("-matrox"))
//		Cvar_SetFloatByRef (&r_nospr32, 1); // Video exception

	FMOD_LoadLibrary ();
	if (fmod_loaded)
		FMOD_Init ();
}


#define SHOWLMP_MAXLABELS	256
typedef struct showlmp_s
{
	qbool	isactive;
	float		x;
	float		y;
	char		label[32];
	char		pic[128];
} showlmp_t;

static showlmp_t	showlmp[SHOWLMP_MAXLABELS];

void SHOWLMP_decodehide (void)
{
	int	i;
	byte	*lmplabel;

	lmplabel = MSG_ReadString ();
	for (i=0 ; i<SHOWLMP_MAXLABELS ; i++)
	{
		if (showlmp[i].isactive && COM_StringMatch (showlmp[i].label, lmplabel))
		{
			showlmp[i].isactive = false;
			return;
		}
	}
}

void SHOWLMP_decodeshow (void)
{
	int	i, k;
	byte	lmplabel[256], picname[256];
	float	x, y;

	strcpy (lmplabel, MSG_ReadString());
	strcpy (picname, MSG_ReadString());
	x = MSG_ReadByte ();
	y = MSG_ReadByte ();
	k = -1;
	for (i=0 ; i<SHOWLMP_MAXLABELS ; i++)
	{
		if (showlmp[i].isactive)
		{
			if (COM_StringMatch (showlmp[i].label, lmplabel))
			{
				k = i;
				break;	// drop out to replace it
			}
		}
		else if (k < 0)	// find first empty one to replace
		{
			k = i;
		}
	}

	if (k < 0)
		return;	// none found to replace
	// change existing one
	showlmp[k].isactive = true;
	strcpy (showlmp[k].label, lmplabel);
	strcpy (showlmp[k].pic, picname);
	showlmp[k].x = x;
	showlmp[k].y = y;
}

void SHOWLMP_drawall (void)
{
	int	i;

	for (i=0 ; i<SHOWLMP_MAXLABELS ; i++)
		if (showlmp[i].isactive)
			Draw_TransPic (showlmp[i].x, showlmp[i].y, Draw_CachePic(showlmp[i].pic));
}

void SHOWLMP_clear (void)
{
	int	i;

	for (i=0 ; i<SHOWLMP_MAXLABELS ; i++)
		showlmp[i].isactive = false;
}

void Neh_CheckMode (void)
{
	qbool	movieinstalled = false, gameinstalled = false;

	Con_Printf ("Beginning Nehahra check...\n");

// Check for movies
	if (QFS_FindFile_Pak_And_Path("hearing.dem", NULL) || QFS_FindFile_Pak_And_Path("hearing.dz", NULL))	// Baker: unlikely to matter
		movieinstalled = true;

	Con_Printf ("Found Movie...\n");

// Check for game
	if (QFS_FindFile_Pak_And_Path("maps/neh1m4.bsp", NULL))	// Baker: unlikely to matter
		gameinstalled = true;

	Con_Printf ("Found Game...\n");

	if (gameinstalled && movieinstalled)
	{
		NehGameType = TYPE_BOTH;             // mainmenu.lmp
		Con_Printf ("Running Both Movie and Game\n");
		return;
	}

	if (gameinstalled && !movieinstalled)
	{
		NehGameType = TYPE_GAME;             // gamemenu.lmp
		Con_Printf ("Running Game Nehahra\n");
		return;
	}

	if (!gameinstalled && movieinstalled)
	{
		NehGameType = TYPE_DEMO;             // demomenu.lmp
		Con_Printf ("Running Movie Nehahra\n");
		return;
	}

	Sys_Error ("You must specify the Nehahra game directory in -game!");
}

#ifdef SUPPORTS_NEHAHRA
// nehahra supported
void Neh_ResetSFX (void)
{
	int	i;

	if (num_sfxorig == 0)
		num_sfxorig = num_sfx;

	num_sfx = num_sfxorig;
	Con_DevPrintf (DEV_SOUND, "Current SFX: %d\n", num_sfx);

	for (i=num_sfx+1 ; i<MAX_SFX ; i++)
	{
		strlcpy (known_sfx[i].name, "dfw3t23EWG#@T#@", sizeof(known_sfx[i].name));
		if (known_sfx[i].cache.data)
			Cache_Free (&known_sfx[i].cache);
	}
}
#endif
