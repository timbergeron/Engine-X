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
// host.c -- coordinates spawning and killing of local servers

#include "quakedef.h"

#if SUPPORTS_AVI_CAPTURE
#include "movie.h"
#endif


double	session_startup_time;
qbool	session_in_firsttime_startup = true;	// As opposed to restarting for a gamedir change


qbool	cvar_in_defaultcfg = false;				// Indicates we are inside default.cfg and accepting game default values
												// *IF* and only if game_initialized is false.  So a user running "exec default.cfg"
												// isn't setting defaults by such an action.

qbool	game_in_initialization = false;			// Game initialized means we are running startup configs.  User values are read here.
												// This is not the same as cvar_in_default.cfg, the game might not be initialized
												// But we may or may not be accepting cvar defaults for the game.
												// Additionally, stuffcmds shouldn't do command crap twice should it?
												// Stuffcmds should refuse to run if "not first_time_startup"

qbool	client_demoplay_via_commandline = false;

char	host_worldname[64];

qbool	Minimized;

#include <time.h>

//#include "r_local.h"

/*

A server can always be started, even if the system started out as a client
to a remote system.

A client can NOT be started if the system started as a dedicated server.

Memory is cleared / released when a server or client begins, not when they end.

*/

engine_globals_def_t engine;


quakeparms_t	host_parms;

qbool	host_initialized;		// true if into command execution


double		host_frametime;
double		host_time;
double		realtime;			// without any filtering or bounding
double		oldrealtime;			// last frame run
//double		last_angle_time;		// JPG - for smooth chasecam

int		host_framecount;

int		host_hunklevel;

int		minimum_memory;

client_t	*host_client;			// current client

jmp_buf 	host_abortserver;

byte		*host_basepal;
byte		*host_colormap;

int		fps_count;

void Host_WriteConfig_f (void);

/*
================
Host_EndGame
================
*/
void Host_EndGame (char *message, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr, message);
	vsnprintf (string,sizeof(string),message,argptr);
	va_end (argptr);
	Con_DevPrintf (DEV_PROTOCOL, "Host_EndGame: %s\n", string);

	if (sv.active)
		Host_ShutdownServer (false);

	if (cls.state == ca_dedicated)
		Sys_Error ("Host_EndGame: %s\n",string);	// dedicated servers exit

	if (cls.demonum != -1)
	{
		CL_StopPlayback ();	// JPG 1.05 - patch by CSR to fix crash
		CL_NextDemo ();
	}
	else
		CL_Disconnect ();

	longjmp (host_abortserver, 1);
}

/*
================
Host_Error

This shuts down both the client and server
================
*/
void Host_Error (char *error, ...)
{
	va_list		argptr;
	char		string[1024];
	static qbool inerror = false;

	if (inerror)
		Sys_Error ("Host_Error: recursively entered");
	inerror = true;

// Baker: thus was your dumbest mistake ever
//#if RENDERER_DIRECT3D_AVAILABLE
//	// Baker: To avoid painful Window with focus behind Direct 3D Window problem
//	if (!isDedicated && engine.Renderer && engine.Renderer->graphics_api == RENDERER_DIRECT3D)
//		VID_Shutdown ();
//#endif

	SCR_EndLoadingPlaque ();		// reenable screen updates

	va_start (argptr, error);
	vsnprintf (string,sizeof(string),error,argptr);
	va_end (argptr);
	Con_Printf ("Host_Error: %s\n",string);

	if (sv.active)
		Host_ShutdownServer (false);

	if (cls.state == ca_dedicated)
		Sys_Error ("Host_Error: %s\n",string);	// dedicated servers exit

	CL_Disconnect ();
	cls.demonum = -1;
	cl.intermission = 0; //johnfitz -- for errors during intermissions (changelevel with no map found, etc.)

	inerror = false;

	longjmp (host_abortserver, 1);
}

/*
================
Host_FindMaxClients
================
*/
void Host_FindMaxClients (void)
{
	int	cmdline_dedicated;
	int cmdline_listen;

	svs.maxclients = 1;

	cmdline_dedicated = COM_CheckParm ("-dedicated");
	if (cmdline_dedicated)
	{
		cls.state = ca_dedicated;
		if (cmdline_dedicated != (com_argc - 1))
			svs.maxclients = atoi (com_argv[cmdline_dedicated+1]);
		else
			svs.maxclients = 8;		// Default for -dedicated with no command line
	}
	else
		cls.state = ca_disconnected;

	cmdline_listen = COM_CheckParm ("-listen");
	if (cmdline_listen)
	{
		if (cls.state == ca_dedicated)
			Sys_Error ("Only one of -dedicated or -listen can be specified");
		if (cmdline_listen != (com_argc - 1))
			svs.maxclients = atoi (com_argv[cmdline_listen+1]);
		else
			svs.maxclients = 8;
	}

	if (svs.maxclients < 1)
		svs.maxclients = 8;
	else if (svs.maxclients > MAX_SCOREBOARD)
		svs.maxclients = MAX_SCOREBOARD;

	if (cmdline_dedicated || cmdline_listen)
	{
		// Baker: only do this if -dedicated or listen
		// So, why does this need done at all since all we are doing is an allocation below.
		// Well, here is why ....
		// we really want the -dedicated and -listen parameters to operate as expected and
		// not be ignored.  This limit shouldn't be able to be changed in game if specified.
		// But we don't want to hurt our ability to play against the bots either.
		svs.maxclientslimit = svs.maxclients;
	}
	else
	{
		svs.maxclientslimit = 16; // Baker: the new default for using neither -listen or -dedicated
	}

	if (svs.maxclientslimit < 4) // No less than 4
		svs.maxclientslimit = 4;
	svs.clients = Hunk_AllocName (svs.maxclientslimit, sizeof(client_t), "clients");

	if (svs.maxclients > 1)
		Cvar_SetFloatByRef (&pr_deathmatch, 1);
	else
		Cvar_SetFloatByRef (&pr_deathmatch, 0);
}



// Baker 3.60 - Host Write Config - from JoeQuake

/*
===============
Host_WriteConfig
===============
*/

void Host_WriteConfig (char *cfgname)	// FWRITERESTRICT
{
	FILE	*f;
	qbool forcebase=false;

	if (!(f = FS_fopen_write(va("%s/%s", com_gamedirfull, cfgname), "w", 0 /* do not create path */)))
	{
		Con_Printf ("Couldn't write %s\n", cfgname);
		return;
	}

	fprintf (f, "// *****************************************\n");
	fprintf (f, "// Generated by %s %s \n", ENGINE_NAME, ENGINE_VERSION);
	fprintf (f, "// *****************************************\n");

	fprintf (f, "// Video Information:\n\n");
	fprintf (f, "// Vendor  :   %s\n",gl_vendor);
	fprintf (f, "// Renderer:   %s\n",gl_renderer);
	fprintf (f, "// GL Version: %s\n",gl_version);
	fprintf (f, "// *****************************************\n");
	fprintf (f, "//\n");
	fprintf (f, "// Your Quake folder: %s\n", com_basedir);
	fprintf (f, "//\n");

	Key_WriteBindings (f);

	Cvar_WriteVariables (f, CVAR_ARCHIVE);	// Write the variables meant to be saved to config
	fprintf (f, "\n// *****************************************\n");

	fclose (f);

#if SUPPORTS_GLVIDEO_MODESWITCH
	// Baker: write startup params
	if (!(f = FS_fopen_write(FULLPATH_TO_EXTERNALCFG, "w", 0 /* do not create path */)))
	{
		Con_Printf ("Couldn't write %s\n", FULLPATH_TO_EXTERNALCFG);
		return;
	}

	VID_SyncCvarsToMode (vid_default); //johnfitz -- write actual current mode to config file, in case cvars were messed with

	Cvar_WriteVariables (f, CVAR_EXTERNAL);	// Write special cvars to this file

	fclose(f);

#endif
}

/*
===============
Host_WriteConfiguration

Writes key bindings and archived cvars to config.cfg
===============
*/
void Host_WriteConfiguration (void)
{
// dedicated servers initialize the host but don't parse and set the config.cfg cvars
	if (host_initialized && !isDedicated) // 1999-12-24 logical correction by Maddes
		Host_WriteConfig ("config.cfg");
}

/*
===============
Host_WriteConfig_f

Writes key bindings and ONLY archived cvars to a custom config file
===============
*/
void Host_WriteConfig_f (void)
{
	char	name[MAX_OSPATH];

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("Usage: %s <filename>\n", Cmd_Argv (0));
		return;
	}

	StringLCopy (name, Cmd_Argv(1));
	COMD_ForceExtension (name, ".cfg");

	Con_Printf ("Writing %s\n", name);

	Host_WriteConfig (name);
}


/*
=================
SV_ClientPrintf

Sends text across to be displayed
FIXME: make this just a stuffed echo?
=================
*/
void SV_ClientPrintf (char *fmt, ...)
{
	va_list	argptr;
	char	string[1024];

	va_start (argptr, fmt);
	vsnprintf (string, sizeof(string),fmt,argptr);
	va_end (argptr);

	MSG_WriteByte (&host_client->message, svc_print);
	MSG_WriteString (&host_client->message, string);
}

/*
=================
SV_BroadcastPrintf

Sends text to all active clients
=================
*/
void SV_BroadcastPrintf (char *fmt, ...)
{
	va_list	argptr;
	char	string[1024];
	int	i;

	va_start (argptr, fmt);
	vsnprintf (string, sizeof(string),fmt,argptr);
	va_end (argptr);

	for (i=0 ; i<svs.maxclients ; i++)
		if (svs.clients[i].active && svs.clients[i].spawned)
		{
			MSG_WriteByte (&svs.clients[i].message, svc_print);
			MSG_WriteString (&svs.clients[i].message, string);
		}
	}

/*
=================
Host_ClientCommands

Send text over to the client to be executed
=================
*/
void Host_ClientCommands (char *fmt, ...)
{
	va_list	argptr;
	char	string[1024];

	va_start (argptr, fmt);
	vsnprintf (string, sizeof(string), fmt,argptr);
	va_end (argptr);

	MSG_WriteByte (&host_client->message, svc_stufftext);
	MSG_WriteString (&host_client->message, string);
}

/*
=====================
SV_DropClient

Called when the player is getting totally kicked off the host
if (crash = true), don't bother sending signofs
=====================
*/
void SV_DropClient (qbool crash)
{
	int		saveSelf;
	int		i;
	client_t	*client;

	// JPG 3.00 - don't drop a client that's already been dropped!
	if (!host_client->active)
		return;

	if (!crash)
	{
		// send any final messages (don't check for errors)
		if (NET_CanSendMessage (host_client->netconnection))
		{
			MSG_WriteByte (&host_client->message, svc_disconnect);
			NET_SendMessage (host_client->netconnection, &host_client->message);
		}

		if (host_client->edict && host_client->spawned)
		{
		// call the prog function for removing a client
		// this will set the body to a dead frame, among other things
			saveSelf = pr_global_struct->self;
			pr_global_struct->self = EDICT_TO_PROG(host_client->edict);
			PR_ExecuteProgram (pr_global_struct->ClientDisconnect);
			pr_global_struct->self = saveSelf;
		}

		Sys_Printf ("Client %s removed\n", host_client->name);
	}

	// JPG 3.00 - check to see if it's a qsmack client
	if (host_client->netconnection->mod == MOD_QSMACK)
		qsmackActive = false;

// break the net connection
	NET_Close (host_client->netconnection);
	host_client->netconnection = NULL;

// free the client (the body stays around)
	host_client->active = false;
	host_client->name[0] = 0;
	host_client->old_frags = -999999;
	net_activeconnections--;

// send notification to all clients
	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		if (!client->active)
			continue;
		MSG_WriteByte (&client->message, svc_updatename);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteString (&client->message, "");
		MSG_WriteByte (&client->message, svc_updatefrags);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteShort (&client->message, 0);
		MSG_WriteByte (&client->message, svc_updatecolors);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteByte (&client->message, 0);
	}
}

/*
==================
Host_ShutdownServer

This only happens at the end of a game, not between levels
==================
*/
void Host_ShutdownServer (qbool crash)
{
	int		i, count;
	sizebuf_t	buf;
	unsigned char	message[4];
	double		start;

	if (!sv.active)
		return;

	sv.active = false;

// stop all client sounds immediately
	if (cls.state == ca_connected)
		CL_Disconnect ();

// flush any pending messages - like the score!!!
	start = Sys_DoubleTime ();
	do
	{
		count = 0;
		for (i=0, host_client=svs.clients ; i<svs.maxclients ; i++, host_client++)
		{
			if (host_client->active && host_client->message.cursize)
			{
				if (NET_CanSendMessage (host_client->netconnection))
				{
					NET_SendMessage (host_client->netconnection, &host_client->message);
					SZ_Clear (&host_client->message);
				}
				else
				{
					NET_GetMessage (host_client->netconnection);
					count++;
				}
			}
		}
		if ((Sys_DoubleTime() - start) > 3.0)
			break;
	}
	while (count);

// make sure all the clients know we're disconnecting
	buf.data = message;
	buf.maxsize = 4;
	buf.cursize = 0;
	MSG_WriteByte (&buf, svc_disconnect);
	count = NET_SendToAll(&buf, 5);
	if (count)
		Con_Printf ("Host_ShutdownServer: NET_SendToAll failed for %u clients\n", count);

	for (i=0, host_client=svs.clients ; i<svs.maxclients ; i++, host_client++)
		if (host_client->active)
			SV_DropClient (crash);

//
// clear structures
//
	memset (&sv, 0, sizeof(sv));
	memset (svs.clients, 0, svs.maxclientslimit * sizeof(client_t));
}


/*
================
Host_ClearMemory

This clears all the memory used by both the client and server, but does
not reinitialize anything.
================
*/
void Host_ClearMemory (void)
{
	Con_DevPrintf (DEV_PROTOCOL, "Clearing memory\n");
//	D_FlushCaches ();
	Mod_ClearAll (host_clearmodels.integer);
	if (host_hunklevel)
		Hunk_FreeToLowMark (host_hunklevel);

	cls.signon = 0;
	memset (&sv, 0, sizeof(sv));
	memset (&cl, 0, sizeof(cl));
	ImageWork_ClearMemory ();
}


//==============================================================================
//
// Host Frame
//
//==============================================================================

/*
===================
Host_FilterTime

Returns false if the time is too short to run a frame
===================
*/
qbool Host_FilterTime (double time)
{
	double	fps;
	float	maxfps_for_frame = host_maxfps.floater;	// By default we are using the cvar

	realtime += time;

	if (cls.timedemo)
		maxfps_for_frame = 99999;   // We are performance testing and want the max possible
	else if (cls.demorecording && sv.active && cl.gametype != GAME_DEATHMATCH)
		maxfps_for_frame = 72;		// Baker: JoeQuake does this to limit fps to Quake standard.  I moved it out of _Host_Frame and made it not foobar the cvar

	fps = max(30, maxfps_for_frame);

	// Don't sleep during a capturedemo (CPU intensive!) or during a timedemo (performance testing)
	if (!cls.capturedemo && !cls.timedemo && realtime - oldrealtime < 1.0 / fps)
	{
#if SUPPORTS_SYSSLEEP
		if (host_sleep.integer)
			Sys_Sleep (); // Lower cpu
#endif
		return false;		// framerate is too high
	}

#if SUPPORTS_AVI_CAPTURE
	if (Movie_IsActive())
		host_frametime = Movie_FrameTime ();
	else
#endif
		host_frametime = realtime - oldrealtime;

	if (cls.demoplayback)
		host_frametime *= CLAMP (0, demospeed.floater, 20);
	oldrealtime = realtime;

	//johnfitz -- host_timescale is more intuitive than host_framerate
	if (host_timescale.floater > 0)
		host_frametime *= host_timescale.floater;
	//johnfitz
	else if (host_framerate.floater > 0)
		host_frametime = host_framerate.floater;
	else // don't allow really long or short frames
		host_frametime = CLAMP (0.001, host_frametime, 0.1);

	return true;
}

/*
===================
Host_GetConsoleCommands

Add them exactly as if they had been typed at the console
===================
*/
void Host_GetDedicatedTerminalConsoleCommands (void)
{
	char	*cmd;

	while (1)
	{
		cmd = Sys_ConsoleInput ();
		if (!cmd)
			break;
		Cbuf_AddText (cmd);
	}
}

/*
==================
Host_ServerFrame
==================
*/

void Host_ServerFrame (void)
{
// JPG 3.00 - stuff the port number into the server console once every minute
	static	double	port_time = 0;

	if (port_time > sv.time + 1 || port_time < sv.time - 60)
	{
		port_time = sv.time;
		Cmd_ExecuteString (va("port %d\n", net_hostport), src_command);
	}

	// run the world state
	pr_global_struct->frametime = host_frametime;

	// set the time and clear the general datagram
	SV_ClearDatagram ();

	// check for new clients
	SV_CheckForNewClients ();

	// read client messages
	SV_RunClients ();

	// move things around and think
	// always pause in single player if in console or menus
	if (!sv.paused && (svs.maxclients > 1 || key_dest == key_game))
		SV_Physics ();

	// send all messages to the clients
	SV_SendClientMessages ();
}

/*
==================
Host_Frame

Runs all active servers
==================
*/
void _Host_Frame (double time)
{
	static double		time1 = 0;
	static double		time2 = 0;
	static double		time3 = 0;
	int		pass1, pass2, pass3;

	if (setjmp(host_abortserver))
		return;		// something bad happened, or the server disconnected

	// keep the random time dependent
	rand ();

	// decide the simulation time
	if (!Host_FilterTime(time))
	{
		// JPG - if we're not doing a frame, still check for lagged moves to send
		if (!sv.active && (cl.movemessages > 2))
			CL_SendLagMove ();
		return;			// don't run too fast, or packets will flood out

	}

	// get new key events
#if !defined(MACOSX)
	Sys_SendKeyEvents ();
#endif // This is done in Windows and Linux.  Confirmed from pq350src

	// allow mice or other external controllers to add commands
#ifdef _WIN32
	IN_Joystick_Commands ();	// Baker: This is ONLY joystick; renamed to indicate that
#else
	// MACOSX really does get mouse commands here.  Who knew!
	IN_Mouse_Commands_OSX ();
#endif
	// process console commands
	Cbuf_Execute ();

	NET_Poll ();

	// if running the server locally, make intentions now
	if (sv.active)
		CL_SendCmd ();  // This is where mouse input is read
#if WINDOWS_SCROLLWHEEL_PEEK
	else if (con_forcedup && key_dest == key_game) // Allows console scrolling when con_forcedup
		IN_Mouse_MouseWheel ();	// Grab mouse wheel input
#endif

//-------------------
//
// server operations
//
//-------------------

	// Baker: Host_GetConsoleCommands is ONLY dedicated server
	// check for commands typed to the host
	Host_GetDedicatedTerminalConsoleCommands ();

	if (sv.active)
		Host_ServerFrame ();

//-------------------
//
// client operations
//
//-------------------

	// if running the server remotely, send intentions now after
	// the incoming messages have been read
	if (!sv.active)
		CL_SendCmd ();

	host_time += host_frametime;

	// fetch results from server
	if (cls.state == ca_connected)
		CL_ReadFromServer ();

	if (host_speeds.integer)
		time1 = Sys_DoubleTime ();

	// update video
	SCR_UpdateScreen ();

	if (host_speeds.integer)
		time2 = Sys_DoubleTime ();

	if (cls.signon == SIGNONS)	// Fully connected to a game
	{
		// update audio
		S_Update (r_origin, vpn, vright, vup);
//		CL_DecayLights ();
//		ViewBlends_FadeDamageBonus_Buckets ();
	}
	else
	{
		S_Update (vec3_origin, vec3_origin, vec3_origin, vec3_origin);
	}

	MP3Audio_Update ();

	if (host_speeds.integer)
	{
		pass1 = (time1 - time3) * 1000;
		time3 = Sys_DoubleTime ();
		pass2 = (time2 - time1) * 1000;
		pass3 = (time3 - time2) * 1000;
		Con_Printf ("%3i tot %3i server %3i gfx %3i snd\n", pass1 + pass2 + pass3, pass1, pass2, pass3);
	}

//	if (!cls.demoplayback && demorewind.integer)
//	{
//		Cvar_SetFloatByRef (&demorewind, 0);
//		Con_Printf ("Demorewind is only enabled during playback\n");
//	}



	host_framecount++;
	fps_count++;
}

void Host_Frame (double time)
{
	double		time1, time2;
	static	double	timetotal;
	static	int	timecount;
	int		i, c, m;

	if (!serverprofile.integer)
	{
		_Host_Frame (time);
		return;
	}

	time1 = Sys_DoubleTime ();
	_Host_Frame (time);
	time2 = Sys_DoubleTime ();

	timetotal += time2 - time1;
	timecount++;

	if (timecount < 1000)
		return;

	m = timetotal * 1000 / timecount;
	timecount = timetotal = 0;
	c = 0;
	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active)
			c++;
	}

	Con_Printf ("serverprofile: %2i clients %2i msec\n", c, m);
}

//============================================================================

///int COM_FileOpenRead (char *path, FILE **hndl);

extern	FILE	*vcrFile;
#define	VCR_SIGNATURE	0x56435231
// "VCR1"

void Host_InitVCR (quakeparms_t *parms)
{
	int	i, len, n;
	char	*p;

	if (COM_CheckParm("-playback"))
	{
		if (com_argc != 2)
			Sys_Error("No other parameters allowed with -playback\n");

		COM_FileOpenRead ("quake.vcr", &vcrFile);
		if (!vcrFile)
			Sys_Error("playback file not found\n");

		fread (&i, 1, sizeof(int), vcrFile);
		if (i != VCR_SIGNATURE)
			Sys_Error("Invalid signature in vcr file\n");

		fread (&com_argc, 1, sizeof(int), vcrFile);
		com_argv = malloc(com_argc * sizeof(char *));
		com_argv[0] = parms->argv[0];
		for (i=0 ; i<com_argc ; i++)
		{
			fread (&len, 1, sizeof(int), vcrFile);
			p = Q_malloc (len, "VCR stuff");
			fread (p, 1, len, vcrFile);
			com_argv[i+1] = p;
		}
		com_argc++; /* add one for arg[0] */
		parms->argc = com_argc;
		parms->argv = com_argv;
	}

	if ((n = COM_CheckParm("-record")))		// FWRITERESTRICT
	{
		vcrFile = FS_fopen_write ("quake.vcr", "wb", 0);	// Bleh ... uses current directory, and who knows what that might be

		i = VCR_SIGNATURE;
		fwrite (&i, 1, sizeof(int), vcrFile);
		i = com_argc - 1;
		fwrite (&i, 1, sizeof(int), vcrFile);
		for (i = 1; i < com_argc; i++)
		{
			if (i == n)
			{
				len = 10;
				fwrite (&len, 1, sizeof(int), vcrFile);
				fwrite (&"-playback", 1, len, vcrFile);
				continue;
			}
			len = strlen (com_argv[i]) + 1;
			fwrite (&len, 1, sizeof(int), vcrFile);
			fwrite (&com_argv[i], 1, len, vcrFile);
		}
	}

}


/*
=======================
Host_InitLocal
======================
*/
void Host_InitLocal (void)
{
	void Con_DevMode_f (void);

	Host_InitCommands ();

	Cvar_Registration_Host ();


	if (Cvar_CmdLineCheckForceFloatByRef_Maybe (false, "-dev", &developer, 2, "Developer mode 2 (filtered)")) // Console isn't initialized so message will not be seen.  Do message again later.
	{
		int i = COM_CheckParm("-dev");
		if (i + 1 < com_argc)
			Cvar_CmdLineCheckForceFloatByRef_Maybe (false, "-dev", &developer_filter, atoi(com_argv[i+1]), "Developer mode filter set");
	}
	else // We don't bother doing this if -dev was in the command line
		Cvar_CmdLineCheckForceFloatByRef_Maybe (false, "-developer", &developer, 1, "Developer mode"); // Console isn't initialized so message will not be seen.  Do message again later.

	Cmd_AddCommand ("dev", Con_DevMode_f);
	Cmd_AddCommand ("writeconfig", Host_WriteConfig_f);	// by joe

	Host_FindMaxClients ();

	host_time = 1.0;		// so a think at time 0 won't get called


//	Host_InitDeQuake();	// JPG 1.05 - initialize dequake array
}



/*
===============
Host_Shutdown

FIXME: this is a callback from Sys_Quit and Sys_Error. It would be better
to run quit through here before the final handoff to the sys code.
===============
*/
void Host_Shutdown (void)
{
	static qbool isdown = false;

	Con_Printf ("Host shutdown ...\n");
	if (isdown)
	{
		printf ("recursive shutdown\n");
		return;
	}
	isdown = true;

// keep Con_Printf from trying to update the screen
	scr_disabled_for_loading = true;

	Host_WriteConfiguration ();		// Write config
	IPLog_WriteLog ();				// Save iplog JPG 1.05 - ip loggging

	if (con_initialized)
		History_Shutdown ();		// Write command line history


	NET_Shutdown ();				// Shutdown network
	Con_Shutdown ();				// Closes qconsole.log

	if (cls.state != ca_dedicated)
	{
		VID_Shutdown ();			// Shutdown Video
		MP3Audio_Shutdown ();			// Shutdown MP3
		S_Shutdown ();					// Shutdown sound
		IN_Shutdown ();					// Shutdown input


	}
}

/*
====================
Host_Init
====================
*/
extern char *external_settings;
void Host_Init (quakeparms_t *parms)
{

// Moved to sys_win_main.c
//	if (parms->memsize < MINIMUM_WIN_MEMORY)
//		Sys_Error ("Only %4.1f megs of memory available, can't execute game", parms->memsize / (float)0x100000);

	host_parms = *parms;

	com_argc = parms->argc;
	com_argv = parms->argv;

#if SUPPORTS_CHEATFREE_MODE
		// JPG 3.00 - moved this here
	srand(time(NULL) ^ Sys_getpid());
#endif

	Memory_Init (parms->memsize);	// Pass the memory base and the size to Memory_Init
													// Zone and cache get allocated in this
	Cbuf_Init ();				// Allocates 8192 for command buffer
	Cmd_Init ();				// Just registers some commands
	Cvar_Init ();				// Just registers some commands and a couple of cvars
	
	Host_InitVCR (parms);		// If the vcr command line stuff isn't there, does nothing at all
	COM_Init (parms->basedir);	// Endianness check and sets up gamedirs
	Host_InitLocal ();			// Registers host cvars and commands

	W_LoadWadFile ("gfx.wad");	// Load the gfx.wad (status bar pics)
	Key_Init ();				// Does important stuff

	Con_Init ();				// Initializes console and opens qconsole.log

	// Baker: we can't "do" this message until here since we don't have a console
	if (COM_CheckParm("-developer"))
		Con_Warning ("Developer mode set by command line.\n");

	if (COM_CheckParm("-dev"))
		Con_Warning ("Developer filtered mode set by command line.\n");

	
	PR_Init ();					// Just commands and/or cvars
	Mod_Init ();				// Initializes mod_novis and does a cvar
	Security_Init ();			// JPG 3.20 - cheat free (get proc address stuff is here)
	NET_Init ();				// True network and cheatfree stuff goes on here
	SV_Init ();					// Registers some variables and does a little initialization
	IPLog_Init ();				// JPG 1.05 - ip address logging ... allocates memory for iplog and reads it in

	Con_Printf ("Exe: "__TIME__" "__DATE__"\n");
	Con_Printf ("%4.1f megabyte heap\n",parms->memsize/ (1024*1024.0));

	if (cls.state != ca_dedicated)	// Why not rely on IsDedicated instead.
	{
		// Baker: Read externals early
		// FS_fopen it.  q_malloc allocate space on file size.  LATER: free it.
#ifndef MACOSX
		external_settings = (byte *)QFS_LoadHunkFile (EXTERNALS_CONFIG_BARE, NULL);	// Baker: Flaw ... if someone uses -game it could load this from a gamedir.  Shouldn't ever happen.  Is not bug ... is feature?
		if (external_settings) COMD_StringReplaceChar (external_settings, '\n', ' ');
#endif
#pragma message ("Quality assurance: OSX hates something about the above")
#pragma message ("Quality assurance: Verify that externals config doesn't bust the COM_Parse limit")
		host_basepal = (byte *)QFS_LoadHunkFile ("gfx/palette.lmp", NULL);
		if (!host_basepal)
			Sys_Error ("Couldn't load gfx/palette.lmp");
		host_colormap = (byte *)QFS_LoadHunkFile ("gfx/colormap.lmp", NULL);
		if (!host_colormap)
			Sys_Error ("Couldn't load gfx/colormap.lmp");

		Palette_Check_Gamma (host_basepal);	// Builds palette ( vid_palette_gamma_table ) for gl texture upload and checks -gamma and adds to table
		TexMgr_BuildPalette (host_basepal);		// Builds d_8_to_24 tables (x2) // Baker: This isn't a video function(!)

#ifndef _WIN32 // on non win32, mouse comes before video for security reasons
		IN_Init ();				// True input (mouse/joystick) initialization
#endif

		Renderer_AskInit ();		// Decides on OpenGL or Direct3D wrapper


		VID_Init (/*host_basepal*/);
#ifdef _WIN32 // on Win32, mouse after video
		IN_Init ();
#endif
		Image_Init ();			// Just registers a couple of cvars


		// Allocate texture slots

		playertextures = texture_extension_number;
		texture_extension_number += MAX_SCOREBOARD;;

		// fullbright skins
		texture_extension_number += MAX_SCOREBOARD;;

		// Sky and water
		skyboxtextures = texture_extension_number;
		texture_extension_number += 6;	// Skybox has 6 images

		solidskytexture = texture_extension_number++;
		alphaskytexture = texture_extension_number++;

		// Lightmaps

		lightmap_textures = texture_extension_number;
		texture_extension_number += MAX_LIGHTMAPS;

		// Scrap

		scrap_texnum = texture_extension_number;
		texture_extension_number += MAX_SCRAPS;

		// Particles
		classic_particletexture = texture_extension_number++;
		classic_particletexture_blocky = texture_extension_number++;

		View_Init ();				// Just registers some commands and cvars
		M_Init ();					// Menu ... just commands
		Chase_Init ();				// Registers some cvars
		Draw_Init ();			// Among other things, loads the console and charset.  So this must occur after Vid_init
		SCR_Init ();
		R_Init ();

		engine.sound = Sound_Init ();	// Hook sound system in
#ifndef MACOSX
		engine.mp3   = MP3_Init ();

#else
		engine.mp3   = NULL;
//		MP3Audio_Init (); // Right?
#endif

		Sbar_Init ();					// Loads the sbar pics
		CL_Init ();						// Does some allocations
#ifdef _WIN32
		Sys_InfoInit();  				// Client only commands.  We don't care about dedicated servers for this
#pragma message ("This needs enabled for OSX")
#endif
		/*
		{
			extern qbool qclassic;

			if (qclassic)
			{
				Con_Printf ("Note: Classic mode set by command line \"-classic\"\n");
				Cvar_CmdLineCheckForceFloatByRef_Maybe (DEV_GAMEDIR, "-classic", &external_lits, 0, "Colored lights off");
				Cvar_CmdLineCheckForceFloatByRef_Maybe (DEV_GAMEDIR, "-classic", &tex_font_smooth,   0, "Font smoothing off");
				Cvar_CmdLineCheckForceFloatByRef_Maybe (DEV_GAMEDIR, "-classic", &scr_hud_style,     0, "Classic HUD on");
				Cvar_CmdLineCheckForceFloatByRef_Maybe (DEV_GAMEDIR, "-classic", &scr_menu_center,  1, "Centered menu on");
				Cvar_CmdLineCheckForceFloatByRef_Maybe (DEV_GAMEDIR, "-classic", &scr_menu_scale,   0, "Scaled menu");
				Cvar_CmdLineCheckForceFloatByRef_Maybe (DEV_GAMEDIR, "-classic", &r_pass_lumas_brush,   0, "Fullbright brush models off");
				Cvar_CmdLineCheckForceFloatByRef_Maybe (DEV_GAMEDIR, "-classic", &r_pass_lumas_alias,    0, "Fullbright alias models off");
				Cvar_CmdLineCheckForceFloatByRef_Maybe (DEV_GAMEDIR, "-classic", &scene_lerpmove,    0, "Animation smoothing off");
				Cvar_CmdLineCheckForceFloatByRef_Maybe (DEV_GAMEDIR, "-classic", &scene_lerpmodels,    0, "Movement smoothing off");
				Cvar_CmdLineCheckForceFloatByRef_Maybe (DEV_GAMEDIR, "-classic", &r_viewmodel_ringalpha,    0, "Ring alpha off");
				Cvar_CmdLineCheckForceFloatByRef_Maybe (DEV_GAMEDIR, "-classic", &sv_bouncedownslopes,    0, "Grenade bounce fix off");
				Cvar_CmdLineCheckForceFloatByRef_Maybe (DEV_GAMEDIR, "-classic", &sv_freezenonclients,    0, "Freezeall command disabled");
				Con_Printf ("Baker: need to turn off stupid dynamic light hack");
				Con_Printf ("Baker: should do overbrights off here");
				Con_Printf ("Baker: Check particle lights to make sure they are correct");
				Con_Printf ("Baker: vgunkick isn't right");
				Con_Printf ("Baker: water ripple");
			}
		}
		*/

#if SUPPORTS_DEMO_AUTOPLAY
		if (com_argc >= 2 && COM_ArgIsDemofile (com_argv[1]))
		{
			char			build_cmd[1024];
			snprintf		(build_cmd, sizeof(build_cmd), "playdemo \"%s\"\n", com_argv[1]);
			Cbuf_AddText	(build_cmd);

			client_demoplay_via_commandline = true;
		}
#endif
		DisplayBox_Init ();

	}

#ifdef SUPPORTS_NEHAHRA
	if (nehahra)
	        Neh_Init ();
#endif

	Hunk_AllocName (0, 0, "-HOST_HUNKLEVEL-");
	host_hunklevel = Hunk_LowMark ();

	host_initialized = true;

	Con_Printf ("Exe: "__TIME__" "__DATE__"\n");
	Con_Printf ("Hunk allocation: %4.1f MB\n", (float)parms->memsize / (1024 * 1024));

	Con_Printf ("\nEngine X version %s\n\n", VersionString());

	Con_Printf ("\x1d\x1e\x1e\x1e\x1e\x1e\x1e Engine X Initialized \x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f\n");

	Cbuf_AddText ("exec quake.rc\n");

	Cbuf_AddText ("\nhint_gameinitialized\n");

#ifdef SUPPORTS_NEHAHRA
	if (nehahra)
	        Neh_DoBindings ();

	CheckParticles ();
#endif

	//MessageBox(NULL, "stage b07", "Error2", 0 /* MB_OK */ );
}

