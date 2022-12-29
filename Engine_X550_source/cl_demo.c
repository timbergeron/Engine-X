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
// cl_demo.c

// ***************
// Merge State:
//
// ProQuake:  Fixmsg, whatever that does
// Engine X:  Demo autoname (record with no params), DZip support, demo menus
// ***************
// Baker: Validated 6-27-2011

#include "quakedef.h"
#include <time.h>	// easyrecord stats
#if SUPPORTS_DZIP_DEMOS
#include <windows.h> // Required for the DZip handle
#endif

#if SUPPORTS_AVI_CAPTURE
#include "movie.h"
#endif

// added by joe
framepos_t	*dem_framepos = NULL;

qbool	start_of_demo = false;

#if SUPPORTS_DZIP_DEMOS
// .dz playback
#ifdef _WIN32 // Dzip handle
static	HANDLE	hDZipProcess = NULL;
#else
//#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
static qbool hDZipProcess = false;
#endif

static qbool	dz_playback = false;
static	char	tempdem_name[256] = "";	// this file must be deleted after playback is finished
static	char	dem_basedir[256] = "";
qbool	dz_unpacking = false;
static	void CheckDZipCompletion ();
static	void StopDZPlayback ();
#endif

qbool OnChange_demospeed (cvar_t *var, char *string)
{
	float	newval = atof(string);

	if (newval < 0 || newval > 20)
		return true;

	return false;
}


void CL_FinishTimeDemo (void);

/*
==============================================================================

DEMO CODE

When a demo is playing back, all NET_SendMessages are skipped, and
NET_GetMessages are read from the demo file.

Whenever cl.time gets past the last received message, another message is
read from the demo file.
==============================================================================
*/

// JPG 1.05 - support for recording demos after connecting to the server
static byte	demo_head[3][MAX_MSGLEN];
static int	demo_head_size[2];

/*
==============
CL_StopPlayback

Called when a demo file runs out, or the user starts a game
==============
*/
void CL_StopPlayback (void)
{
	if (!cls.demoplayback)
		return;

	fclose (cls.demofile);
	cls.demoplayback = false;
	cls.demofile = NULL;
	cls.state = ca_disconnected;

	Con_DevPrintf(DEV_DEMOS, "Demo playback has ended\n");

#if SUPPORTS_DZIP_DEMOS
	if (dz_playback)
		StopDZPlayback ();
#endif

	if (cls.timedemo)
		CL_FinishTimeDemo ();

#if SUPPORTS_AVI_CAPTURE
	Movie_StopPlayback ();
#endif
}

/*
====================
CL_WriteDemoMessage

Dumps the current net message, prefixed by the length and view angles
====================
*/
void CL_WriteDemoMessage (void)
{
	int	i, len;
	float	f;

	len = LittleLong (net_message.cursize);
	fwrite (&len, 4, 1, cls.demofile);
	for (i=0 ; i<3 ; i++)
	{
		f = LittleFloat (cl.viewangles[i]);
		fwrite (&f, 4, 1, cls.demofile);
	}
	fwrite (net_message.data, net_message.cursize, 1, cls.demofile);
	fflush (cls.demofile);
}

void PushFrameposEntry (long fbaz)
{
	framepos_t	*newf;

	newf = Q_malloc (sizeof(framepos_t), "Demo rewind");
	newf->baz = fbaz;

	if (!dem_framepos)
	{
		newf->next = NULL;
		start_of_demo = false;
	}
	else
	{
		newf->next = dem_framepos;
	}
	dem_framepos = newf;
}

static void EraseTopEntry (void)
{
	framepos_t	*top;

	top = dem_framepos;
	dem_framepos = dem_framepos->next;
	Q_malloc_free (top);
}

/*
====================
CL_GetMessage

Handles recording and playback of demos, on top of NET_ code
====================
*/
int CL_GetMessage (void)
{
	int	r, i;
	float	f;

	if (cl.paused & 2)		// by joe: pause during demo
		return 0;

#if SUPPORTS_DZIP_DEMOS
	CheckDZipCompletion ();
	if (dz_unpacking)
		return 0;
#endif

	if (cls.demoplayback)
	{
		if (start_of_demo && demorewind.integer)
			return 0;

		if (cls.signon < SIGNONS)	// clear stuffs if new demo
			while (dem_framepos)
				EraseTopEntry ();

		// decide if it is time to grab the next message
		if (cls.signon == SIGNONS)	// always grab until fully connected
		{
			if (cls.timedemo)
			{
				if (host_framecount == cls.td_lastframe)
					return 0;		// already read this frame's message
				cls.td_lastframe = host_framecount;
				// if this is the second frame, grab the real td_starttime
				// so the bogus time on the first frame doesn't count
				if (host_framecount == cls.td_startframe + 1)
					cls.td_starttime = realtime;
			}
			// modified by joe to handle rewind playing
			else if (!demorewind.integer && cl.ctime <= cl.mtime[0])
				return 0;		// don't need another message yet
			else if (demorewind.integer && cl.ctime >= cl.mtime[0])
				return 0;

			// joe: fill in the stack of frames' positions
			// enable on intermission or not...?
			// NOTE: it can't handle fixed intermission views!
			if (!demorewind.integer /*&& !cl.intermission*/)
				PushFrameposEntry (ftell(cls.demofile));
		}

		cls._democurpos =ftell(cls.demofile);

		// get the next message
		fread (&net_message.cursize, 4, 1, cls.demofile);							// demo_fread
		VectorCopy (cl.mviewangles[0], cl.mviewangles[1]);
		for (i=0 ; i<3 ; i++)
		{
			r = fread (&f, 4, 1, cls.demofile);										// demo_fread
			cl.mviewangles[0][i] = LittleFloat (f);
		}

		net_message.cursize = LittleLong (net_message.cursize);
		if (net_message.cursize > MAX_MSGLEN)
			Sys_Error ("Demo message > MAX_MSGLEN");

		r = fread (net_message.data, net_message.cursize, 1, cls.demofile);			// demo_fread
		if (r != 1)
		{
			CL_StopPlayback ();
			return 0;
		}



		// joe: get out framestack's top entry
		if (demorewind.integer /*&& !cl.intermission*/)
		{
			if (dem_framepos/* && dem_framepos->baz*/)	// Baker: in theory, if this occurs we ARE at the start of the demo with demo rewind on
			{
				fseek (cls.demofile, dem_framepos->baz, SEEK_SET);
				EraseTopEntry (); // Baker: we might be able to improve this better but not right now.
			}
			if (!dem_framepos)
				start_of_demo = true;
		}

		return 1;
	}

	while (1)
	{
		r = NET_GetMessage (cls.netcon);

		if (r != 1 && r != 2)
			return r;

		// discard nop keepalive message
		if (net_message.cursize == 1 && net_message.data[0] == svc_nop)
			Con_Printf ("<-- server to client keepalive\n");
		else
			break;
	}

	if (cls.demorecording)
		CL_WriteDemoMessage ();

	// JPG 1.05 - support for recording demos after connecting
	if (cls.signon < 2)
	{
		memcpy (demo_head[cls.signon], net_message.data, net_message.cursize);
		demo_head_size[cls.signon] = net_message.cursize;

	}

	return r;
}

char *CheckName(const char *AbsoluteFile_NoExtension, const char *Extension)
{
	static char outname[MAX_OSPATH];
	int i;

	snprintf (outname, sizeof(outname), "%s%s", AbsoluteFile_NoExtension, Extension); // Extensions already has the dot

	if (Sys_FileTime(outname) == -1)
		return outname;		// File doesn't exist

	// Else keep looking

	for (i=1 ; i<10000 ; i++)
	{
		int file_exists = 0;

		snprintf(outname, sizeof(outname), "%s%i%s", AbsoluteFile_NoExtension, i, Extension); // Extension already has the dot

		file_exists = Sys_FileTime(outname);

		if (file_exists == -1)
			return outname; // file doesn't exist
	}

	return NULL;	// Let receiving code figure out the error message
}


#if SUPPORTS_DZIP_DEMOS
#define SUPPORTS_DEMO_AUTOCOMPRESSION 1
#endif

#if SUPPORTS_DEMO_AUTOCOMPRESSION
static void sDemoAutoCompress (void)
{

	Con_Printf ("Compressing demo ...\n");
	{	// Auto-compress
		char test_dz_demo_name[MAX_OSPATH];
		char *real_dz_demo_name;


#if DEMO_DZ_AUTONAMES
		COM_Copy_StripExtension (cls._demoname, test_dz_demo_name);	// Copies string less extension.
		real_dz_demo_name = CheckName (test_dz_demo_name, ".dz");		Autonames

		if (!real_dz_demo_name)
			Con_Printf ("Auto-compression of demo failed; not able to find suitable name\n");

		// .. At this point we need to return or something

#else
		StringLCopy (test_dz_demo_name, cls._demoname);
		COMD_ForceExtension (test_dz_demo_name, ".dz");

		real_dz_demo_name = test_dz_demo_name;

		// If existing .dz demo file exists, delete it
		// We could autoname it, however, since "record" overwrites an existing .dem
		// We should have auto-compression overwrite an existing .dz
		// Otherwise confusion ... like "record this" and then immediately a "play this" isn't what you recorded
		// if it already existed
		if (Sys_FileTime(real_dz_demo_name) != -1)
		{
			if (remove (real_dz_demo_name)==-1)	// _wremove
				Con_Printf ("Could not delete '%s'\n");
		}
#endif

		else
		{	// Build command line
			// Super explicit exe line with super-explicit demo line -o and short output name with demos dir as working dir
			char commandline_buffer[1024];
			STARTUPINFO si;
			PROCESS_INFORMATION pi;


			snprintf (commandline_buffer, sizeof(commandline_buffer), "\"%s\" \"%s\" -o \"%s\"", FULLPATH_TO_DZIP_EXE, cls._demoname, StringTemp_SkipPath(real_dz_demo_name));

			#ifdef _WIN32 // Windows create process API
				memset (&si, 0, sizeof(si));
				si.cb = sizeof(si);
				si.wShowWindow = SW_HIDE;
				si.dwFlags = STARTF_USESHOWWINDOW;

				if (!CreateProcess(NULL, commandline_buffer, NULL, NULL, FALSE, 0, NULL, FOLDER_DEMOS /* where tmp file is*/, &si, &pi))
				{
					Con_Printf ("Couldn't execute %s/utils/dzip.exe\n", /*com_basedir*/ com_enginedir);
					return;
				}

				// pi.hProcess is process

				// Wait for it to complete
				do {
					DWORD	ExitCode;

					if (!GetExitCodeProcess(pi.hProcess, &ExitCode))
					{
						Con_Printf ("WARNING: GetExitCodeProcess failed\n");
//						hDZipProcess = NULL;
//						dz_unpacking = dz_playback = cls.demoplayback = false;
//						StopDZPlayback ();
						return;
					}
					if (ExitCode != STILL_ACTIVE)
						break;

					Sleep (1);
				} while (1);


//				hDZipProcess = NULL;

				// Validate filename

				if (Sys_FileTime(real_dz_demo_name)!=-1)
				{
					Con_Printf ("Demo successfully compressed as %s\n", StringTemp_SkipPath(real_dz_demo_name));
					// Remove old file
					if (remove (cls._demoname)==-1)	// _wremove
						Con_Printf ("Could not delete '%s'\n");
					else
						Con_DevPrintf (DEV_DEMOS, "Successfully deleted '%s'\n");
				}


			#else
				Cross That Bridge Later!
			#endif


		}
	}

	Con_Printf ("Demo compression complete.\n");

}
#endif

/*
====================
CL_Stop_f

stop recording a demo
====================
*/
void CL_Stop_f (void)
{
	if (cmd_source != src_command)
		return;

	if (!cls.demorecording)
	{
		Con_Printf ("Not recording a demo\n");
		return;
	}

// write a disconnect message to the demo file
	SZ_Clear (&net_message);
	MSG_WriteByte (&net_message, svc_disconnect);
	CL_WriteDemoMessage ();

// finish up
	fclose (cls.demofile); // Functions as both player and recorder
	cls.demofile = NULL;
	cls.demorecording = false;
	Con_Printf ("Completed demo\n");
#if SUPPORTS_DEMO_AUTOCOMPRESSION
	sDemoAutoCompress ();
#endif


}



/*
====================
CL_Record_f

record <demoname> <map> [cd track]
====================
*/

void CL_Record_f (void)
{
	int			c, track;
	char		name[MAX_OSPATH], easyname[MAX_OSPATH] = "";  // Jozsef had these MAX_OSPATH*2 for some reason
	extern char com_demodirfull[MAX_OSPATH];

	if (cmd_source != src_command) // Apparently, it is not allow for the server to force a client to record
		return;

	if (cls.demoplayback)
	{
		Con_Printf ("Can't record during demo playback\n");
		return;
	}

	c = Cmd_Argc ();

	if (c > 4)
	{
		Con_Printf ("record <demoname> [<map> [cd track]]\n");
		return;
	}

	if (c == 1 || c == 2)
	{
	//	Baker: Not true.  Quake never requires you to be connected to start recording a demo, so we don't either.
	//
	//	if (cls.state != ca_connected)
	//	{
	//		Con_Printf ("You must be connected for recording\n");
	//		return;
	//	}

		if (c == 1)
		{
			time_t	ltime;
			char	str[MAX_OSPATH];

			time (&ltime);
			strftime (str, sizeof(str)-1, "%b-%d-%Y-%H%M%S", localtime(&ltime));

			snprintf (easyname, sizeof(easyname), "%s-%s", cl.worldname, str);
		}
		else if (c == 2)
		{
			if (strstr(Cmd_Argv(1), ".."))
			{
				Con_Printf ("Relative pathnames are not allowed.\n");
				return;
			}
			if (cls.state == ca_connected && cls.signon < 2)
			{
				Con_Printf ("Can't record - try again when connected\n");
				return;
			}
		}
	}

	// JPG 3.00 - consecutive demo bug
	if (cls.demorecording)
		CL_Stop_f();

// write the forced cd track number, or -1
	if (c == 4)
	{
		track = atoi(Cmd_Argv(3));
		Con_Printf ("Forcing CD track to %i\n", cls.forcetrack);
	}
	else
	{
		track = -1;
	}

	if (easyname[0])
		snprintf (name, sizeof(name), "%s/%s", FOLDER_DEMOS, easyname);
	else
		snprintf (name, sizeof(name), "%s/%s", FOLDER_DEMOS, Cmd_Argv(1));

// start the map up
	if (c > 2)
	{
		Cmd_ExecuteString (va("map %s", Cmd_Argv(2)), src_command);
	// joe: if couldn't find the map, don't start recording
		if (cls.state != ca_connected)
			return;
	}

// open the demo file
	COMD_ForceExtension (name, ".dem");

	Con_Printf ("recording to %s.\n", name);

	// open the file
	cls.demofile = FS_fopen_write(name, "wb", FS_CREATE_PATH);		// FWRITE

	if (!cls.demofile)
	{
		Con_Printf ("ERROR: couldn't open %s\n", name);
		return;
	}

	// copy out data

	// Officially recording ... copy the name for reference
	snprintf(cls._demoname, sizeof(cls._demoname), "%s", name);
	Con_DevPrintf(DEV_DEMOS, "Current demo is: %s\n", cls._demoname);

	cls.forcetrack = track;
	fprintf (cls.demofile, "%i\n", cls.forcetrack);

	cls.demorecording = true;

	// joe: initialize the demo file if we're already connected
	if (c < 3 && cls.state == ca_connected)
	{
		byte	*data = net_message.data;
		int	i, cursize = net_message.cursize;

		for (i=0 ; i<2 ; i++)
		{
			net_message.data = demo_head[i];
			net_message.cursize = demo_head_size[i];
			CL_WriteDemoMessage ();
		}

		net_message.data = demo_head[2];
		SZ_Clear (&net_message);

		// current names, colors, and frag counts
		for (i=0 ; i<cl.maxclients ; i++)
		{
			MSG_WriteByte (&net_message, svc_updatename);
			MSG_WriteByte (&net_message, i);
			MSG_WriteString (&net_message, cl.scores[i].name);
			MSG_WriteByte (&net_message, svc_updatefrags);
			MSG_WriteByte (&net_message, i);
			MSG_WriteShort (&net_message, cl.scores[i].frags);
			MSG_WriteByte (&net_message, svc_updatecolors);
			MSG_WriteByte (&net_message, i);
			MSG_WriteByte (&net_message, cl.scores[i].colors);
		}

		// send all current light styles
		for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
		{
			MSG_WriteByte (&net_message, svc_lightstyle);
			MSG_WriteByte (&net_message, i);
			MSG_WriteString (&net_message, cl_lightstyle[i].map);
		}

		// send stats - OMG from JoeQuake 0.15 ... Baker: What about the CD track or SVC fog ... future consideration.
		MSG_WriteByte (&net_message, svc_updatestat);
		MSG_WriteByte (&net_message, STAT_TOTALSECRETS);
		MSG_WriteLong (&net_message, pr_global_struct->total_secrets);

		MSG_WriteByte (&net_message, svc_updatestat);
		MSG_WriteByte (&net_message, STAT_TOTALMONSTERS);
		MSG_WriteLong (&net_message, pr_global_struct->total_monsters);

		MSG_WriteByte (&net_message, svc_updatestat);
		MSG_WriteByte (&net_message, STAT_SECRETS);
		MSG_WriteLong (&net_message, pr_global_struct->found_secrets);

		MSG_WriteByte (&net_message, svc_updatestat);
		MSG_WriteByte (&net_message, STAT_MONSTERS);
		MSG_WriteLong (&net_message, pr_global_struct->killed_monsters);

		// view entity
		MSG_WriteByte (&net_message, svc_setview);
		MSG_WriteShort (&net_message, cl.player_point_of_view_entity);

		// signon
		MSG_WriteByte (&net_message, svc_signonnum);
		MSG_WriteByte (&net_message, 3);

		CL_WriteDemoMessage ();

		// restore net_message
		net_message.data = data;
		net_message.cursize = cursize;
	}
}

void StartPlayingOpenedDemo (void)
{
	int		c;
	qbool	neg = false;

	cls.demoplayback = true;
	cls.state = ca_connected;
	cls.forcetrack = 0;

	while ((c = getc(cls.demofile)) != '\n')
	{
		if (c == '-')
			neg = true;
		else
			cls.forcetrack = cls.forcetrack * 10 + (c - '0');
	}

	if (neg)
		cls.forcetrack = -cls.forcetrack;
}

#if SUPPORTS_DZIP_DEMOS
// joe: playing demos from .dz files
static void CheckDZipCompletion (void)
{
#ifdef _WIN32 // DZip process api
	DWORD	ExitCode;

	if (!hDZipProcess)
		return;

	if (!GetExitCodeProcess(hDZipProcess, &ExitCode))
	{
		Con_Printf ("WARNING: GetExitCodeProcess failed\n");
		hDZipProcess = NULL;
		dz_unpacking = dz_playback = cls.demoplayback = false;
		StopDZPlayback ();
		return;
	}

	if (ExitCode == STILL_ACTIVE)
		return;

	hDZipProcess = NULL;
#else
	if (!hDZipProcess)
		return;

	hDZipProcess = false;
#endif

	if (!dz_unpacking || !cls.demoplayback)
	{
		StopDZPlayback ();
		return;
	}

	dz_unpacking = false;

	if (!(cls.demofile = FS_fopen_read(tempdem_name, "rb")))
	{
		Con_Printf ("ERROR: couldn't open %s\n", tempdem_name);
		dz_playback = cls.demoplayback = false;
		cls.demonum = -1;
		return;
	}

	// start playback
	StartPlayingOpenedDemo ();
}

static void StopDZPlayback (void)
{
	if (!hDZipProcess && tempdem_name[0])
	{
		char	temptxt_name[256];

		COM_Copy_StripExtension (tempdem_name, temptxt_name);
		COMD_ForceExtension (temptxt_name, ".txt");
		remove (temptxt_name); // Delete ... removes a .dz txt file included like the SDA does
		if (remove(tempdem_name))
			Con_Printf ("Couldn't delete %s\n", tempdem_name);
		tempdem_name[0] = '\0';
	}
	dz_playback = false;
}

//Baker: This needs to switch to the path that the demo is and unpack it
static void PlayDZDemo (void)
{
	char	*name, *p, dz_name[256];
#ifdef _WIN32 // DZip process api
	char	cmdline[512];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	if (hDZipProcess)
	{
		Con_Printf ("Cannot unpack -- DZip is still running!\n");
		return;
	}
#else
	char	dzipRealPath[256], tmppath[256];
	pid_t	pid;
#endif

	name = Cmd_Argv (1);

	// Remove path changing crap from demo name for security reasons
	if (strstr(name, "..") == name)
	{
		snprintf (dem_basedir, sizeof(dem_basedir), "%s%s", com_basedir, name + 2);
		p = strrchr (dem_basedir, '/');
		*p = 0;
		name = strrchr (name, '/') + 1;	// we have to cut off the path for the name
	}
	else
	{	// Otherwise we are copying com_gamedirfull to dem_basedir for the demos menu browser
		StringLCopy (dem_basedir, com_gamedirfull);
	}

#ifndef _WIN32 // DZip process stuff ... maybe non-Windows has to change the dir
	if (!realpath(dem_basedir, tmppath))
	{
		Con_Printf ("Couldn't realpath '%s'\n", dem_basedir); // Baker: Uggggh!  Fix this!
		return;
	}
	StringLCopy (dem_basedir, tmppath);
#endif

	snprintf (dz_name, sizeof(dz_name), "%s/%s", dem_basedir, name); // Baker: Uggggh!  Fix this!

	// check if the file exists
	if (Sys_FileTime(dz_name) == -1)
	{
		Con_Printf ("ERROR: couldn't open %s\n", name);
		return;
	}

	COM_Copy_StripExtension (dz_name, tempdem_name);
	COMD_ForceExtension (tempdem_name, ".dem");
//	Con_Printf("tempdem_name is %s\n", tempdem_name);
//	MessageBox (NULL, va("tempdem_name is %s\n", tempdem_name), "", MB_OK);

	if ((cls.demofile = FS_fopen_read(tempdem_name, "rb")))
	{
		// .dem already exists, so just play it
		Con_Printf ("Playing demo from %s\n", tempdem_name);
		StartPlayingOpenedDemo ();
		return;
	}

	Con_Printf ("\x02" "\nunpacking demo. please wait...\n\n");
	key_dest = key_game;

	// start DZip to unpack the demo
#ifdef _WIN32 // DZip process api
	memset (&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;

	snprintf (cmdline, sizeof(cmdline), "%s -x -f \"%s\"", FULLPATH_TO_DZIP_EXE, name);
//	Con_Printf ("DZip cmdline: %s\n", cmdline);
	if (!CreateProcess(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, dem_basedir, &si, &pi))
	{
		Con_Printf ("Couldn't execute %s\n", FULLPATH_TO_DZIP_EXE);
		return;
	}

	hDZipProcess = pi.hProcess;
#else
	if (!realpath(com_basedir, dzipRealPath))
	{
		Con_Printf ("Couldn't realpath '%s'\n", com_basedir);
		return;
	}

	if (chdir(dem_basedir) == -1)
	{
		Con_Printf ("Couldn't chdir to '%s'\n", dem_basedir);
		return;
	}

	switch (pid = fork())
	{
	case -1:
		Con_Printf ("Couldn't create subprocess\n");
		return;

	case 0:
		if (execlp(va("%s/dzip-linux", dzipRealPath), "dzip-linux", "-x", "-f", dz_name, NULL) == -1)
		{
			Con_Printf ("Couldn't execute %s/dzip-linux\n", com_basedir);
			exit (-1);
		}

	default:
		if (waitpid(pid, NULL, 0) == -1)
		{
			Con_Printf ("waitpid failed\n");
			return;
		}
		break;
	}

	if (chdir(dzipRealPath) == -1)
	{
		Con_Printf ("Couldn't chdir to '%s'\n", dzipRealPath);
		return;
	}

	hDZipProcess = true;
#endif

	dz_unpacking = dz_playback = true;

	// demo playback doesn't actually start yet, we just set cls.demoplayback
	// so that CL_StopPlayback() will be called if CL_Disconnect() is issued
	cls.demoplayback = true;
	cls.demofile = NULL;
	cls.state = ca_connected;
}
#endif

/*
====================
CL_PlayDemo_f

playdemo [demoname]
====================
*/
void CL_PlayDemo_f (void)
{
	char	demo_to_play[MAX_OSPATH];

	if (cmd_source != src_command)
		return;

	Con_DevPrintf (DEV_DEMOS, "Beginning new demo ...\n");

#ifdef SUPPORTS_EXTENDED_MENUS
	// With no params, JoeQuake does demo menu
	if (Cmd_Argc() == 1)
	{
		Cbuf_AddText ("menu_demos\n");
		return;
	}
#endif

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("playdemo <demoname> : plays a demo\n");
		return;
	}

// disconnect from server
	CL_Disconnect ();

	// added by joe, but FIXME!
//	if (demorewind.integer)
//	{
//		Con_Printf ("ERROR: rewind is on\n");
//		cls.demonum = -1;
//		return;
//	}

	// Revert
	Cvar_SetFloatByRef (&demorewind, 0);
	Cvar_SetFloatByRef (&demospeed, 1);

// open the demo file
	StringLCopy (demo_to_play, Cmd_Argv(1));

//  Baker: Check for .dem first

#if SUPPORTS_DZIP_DEMOS
	// Explicit extensions
	if (COM_IsExtension(demo_to_play, ".dz"))
	{
		PlayDZDemo ();
		return;
	}
#endif

	COMD_DefaultExtension (demo_to_play, ".dem");

	{
		QFS_FOpenFile (demo_to_play, &cls.demofile, NULL);
		if (!cls.demofile)
		{
	#if SUPPORTS_DEMO_AUTOPLAY
			// Baker 3.76 - Check outside the demos folder!
			cls.demofile = FS_fopen_read(demo_to_play, "rb"); // Baker 3.76 - check DarkPlaces file system

			if (!cls.demofile)  // Baker 3.76 - still failed
	#endif
			{
				Con_Printf ("ERROR: couldn't open %s\n", demo_to_play);
				cls.demonum = -1;		// stop demo loop
				return;
			}
		}
		cls._demostartpos = ftell (cls.demofile);	// qfs_lastload.offset instead?
		cls._demolength   = qfs_lastload.filelen;
		cls.titledemo     = false;
//		Con_Printf ("length now each read advances how far\n");
	}



	Con_Printf ("Playing demo from %s\n", StringTemp_SkipPath(demo_to_play));

	// Title demos get no HUD and no crosshair
	if (COM_IsInList(demo_to_play, list_models_r_noshadow.string))
		cls.titledemo     = true;

	StartPlayingOpenedDemo ();
}

/*
====================
CL_FinishTimeDemo
====================
*/
void CL_FinishTimeDemo (void)
{
	int	frames;
	float	time;

	cls.timedemo = false;

// the first frame didn't count
	frames = (host_framecount - cls.td_startframe) - 1;
	time = realtime - cls.td_starttime;
	if (!time)
		time = 1;
	Con_Printf ("%i frames %5.1f seconds %5.1f fps\n", frames, time, frames/time);
}

/*
====================
CL_TimeDemo_f

timedemo [demoname]
====================
*/
void CL_TimeDemo_f (void)
{
	if (cmd_source != src_command)
		return;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("timedemo <demoname> : gets demo speeds\n");
		return;
	}

	// Baker: This is a performance benchmark.  No reason to have console up.
	if (key_dest != key_game)
		key_dest = key_game;

	CL_PlayDemo_f ();

// cls.td_starttime will be grabbed at the second frame of the demo, so
// all the loading time doesn't get counted

	cls.timedemo = true;
	cls.td_startframe = host_framecount;
	cls.td_lastframe = -1;		// get a new message this frame
}
