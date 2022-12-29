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
// quakedef.h -- primary header for client

#ifndef __QUAKEDEF_H__
#define __QUAKEDEF_H__

//#define	GLTEST					// experimental stuff

#define	QUAKE_GAME					// as opposed to utilities

#define ENGINE_NAME					"Engine X"
#define ENGINE_VERSION 				"550-BETA R1"
#define ENGINE_HOMEPAGE_URL			"http://www.quakeone.com/proquake"
#define PROQUAKE_SERIES_VERSION		5.50

#include "version.h"

//define	PARANOID				// speed sapping error checking

#define	MAX_QPATH					64			// max length of a quake game pathname
#define	MAX_OSPATH					128			// max length of a filesystem pathname

extern char com_basedir[MAX_OSPATH];
extern char com_gamedirfull[MAX_OSPATH];		// Should go bye bye
extern char com_enginedir[MAX_OSPATH];			// Should go bye bye
extern char com_gamedirshort[MAX_QPATH]; 		// Baker: allows us to directly support bots


#define	GAMENAME				"id1"											// directory to look in by default
#define ENGINEFOLDER			"enginex"

#define EXTERNALS_CONFIG_BARE	"enginex_external.cfg"	// Because ProQuake or some other engine might have it
#define FULLPATH_TO_EXTERNALCFG	va("%s/%s", com_enginedir, EXTERNALS_CONFIG_BARE)
#define FULLPATH_TO_CONFIG_CFG	va("%s/config.cfg", com_gamedirfull)


#define	HISTORY_FILE_NAME		va("%s/proquake_history.txt", com_enginedir)

#define ONLYFILENAME_EX_PAK		"engine_x.pak"
#define ONLYFILENAME_EXX_PAK	"engine_x_extras.pak"
#define ONLYFILENAME_EXX2_PAK	"engine_x_extras_litepcx.pak"
#define FULLPATH_TO_EX_PAK		va("%s/%s", com_enginedir, ONLYFILENAME_EX_PAK)
#define FULLPATH_TO_EXX_PAK		va("%s/%s", com_enginedir, ONLYFILENAME_EXX_PAK)
#define FULLPATH_TO_EXX2_PAK	va("%s/%s", com_enginedir, ONLYFILENAME_EXX2_PAK)

#define FOLDER_UTILS			va("%s/%s", com_enginedir, "utils")

#define FULLPATH_TO_DZIP_EXE	va("%s/dzip.exe", FOLDER_UTILS)			// "c:\quake1\enginex\utils\dzip.exe"

#define FULLPATH_TO_PAKSCAPE	va("%s/pakscape.exe", FOLDER_UTILS)			// "c:\quake1\enginex\utils\dzip.exe"
#define FULLPATH_TO_FIMG_EXE	va("%s/fimg.exe", FOLDER_UTILS)			// "c:\quake1\enginex\utils\dzip.exe"

#define RELAPATH_TO_FMOD_DLL	va("%s/fmod.dll", FOLDER_UTILS)		// "enginex/utils/fmod.dll"
#define GENERIC_SCREENSHOT_NAME "enginex"										// Like how screens are engine named
//#define NORMAL_GAMEDIR		"enginex"	// Used for single player --> newgame to go to non-bots folder
											// in reality this is not enough.  Demos, connecting to a server, etc. etc.
											// Maybe we can make bots just a pure different progs.dat modification
											// With a startup script

// Dedicated server:  You might have 15 hosted on a box.  Dedicated server each needs own gamedir to keep them separate.
// Baker: Unusually ... we are having id1 own this for clients.
#define FULLPATH_TO_IPLOGDIR	isDedicated ? com_gamedirfull : va("%s/%s", com_basedir, GAMENAME)

#define FULLPATH_TO_IPLOG_DAT	va("%s/iplog.dat", FULLPATH_TO_IPLOGDIR)
#define FULLPATH_TO_IPLOG_TXT	va("%s/iplog.txt", FULLPATH_TO_IPLOGDIR)


#define FULLPATH_TO_QCONSOLE_LOG isDedicated ? va("%s/qconsole.log", com_gamedirfull) : va("%s/qconsole.log", com_enginedir)	// Baker: We kinda want this to be engine-owned by client, not gamedir owned

#define FOLDER_QUAKE			com_basedir
#define	FOLDER_DEMOS			va("%s/%s", com_gamedirfull, "demos")	// Baker: not supporting id1 override for this.  Reasons: 1. standard place is id1 not id1\demos.  It's sloppy.  Reasons 2.  Incompatible demos could be in there (DarkPlaces or whatever).
#define	FOLDER_MUSIC			va("%s/%s", com_gamedirfull, "music")	// Baker: not supporting id1 override for this.  First, most engines don't support music.  Second, I'm not going to have gamedata for Engine X in id1 folder.

// Baker: id1 maps ... should adjust to be
#define isGamedired strcasecmp(com_gamedirshort, ENGINEFOLDER)
#define	FOLDER_MAPS				isGamedired ? va("%s/%s", com_gamedirfull, "maps") : va("%s/%s/%s", com_basedir, GAMENAME, "maps")
// Baker: absolutely WRONG for Engine X to use own dir for save games
#define	FOLDER_SAVES			isGamedired ? com_gamedirfull                      : va("%s/%s"   , com_basedir, GAMENAME)



// These are user media not gamedir dependent.  They are owned by the engine.
// Except if dedicated server (dedicated server doesn't support Engine X folder or at least won't)
// And dedicated server cannot support this stuff anyway.
#define	FOLDER_SHOTS			va("%s/%s", com_enginedir, "screenshots")
#define	FOLDER_VIDEOS			va("%s/%s", com_enginedir, "videos")

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define	MINIMUM_MEMORY			0x550000
#define	MINIMUM_MEMORY_LEVELPAK	(MINIMUM_MEMORY + 0x100000)

#define	MAXCMDLINE				256			// Maximum command line length
#define MAX_NUM_ARGVS			50			// Max numbers of arguments to anything


#define	PITCH					0			// up / down
#define	YAW						1			// left / right
#define	ROLL					2			// fall over


#define	ON_EPSILON				0.1			// point on plane side epsilon

// Baker: Make this work


#define DATAGRAM_MTU					1400
#define MIN_EDICTS_FLOOR				 256		// johnfitz -- lowest allowed value for max_edicts cvar

typedef enum
{
//	FitzQuake limit					 // Standard limit (WinQuake)
	MAX_EDICTS_PROTOCOL_666			= 32000,  MAX_EDICTS_PROTOCOL_15		=  8192, // Baker: Although WinQuake only supports 640 edicts, the protocol can handle up to 8192 correctly.
	MAX_FITZQUAKE_SANE_DEF_EDICTS	=  2048,  MAX_WINQUAKE_SANE_DEF_EDICTS	=   640, // Baker: These are my arbitrary values.  The latter number keeps compatibility, the former is just sanity.

	MAX_FITZQUAKE_MSGLEN			= 32000,  MAX_WINQUAKE_MSGLEN			=  8000,
	MAX_FITZQUAKE_DATAGRAM			= 32000,  MAX_WINQUAKE_DATAGRAM			=  1024,

// per-level limits
	MAX_FITZQUAKE_BEAMS				=    32,  MAX_WINQUAKE_BEAMS			=    24,
	MAX_FITZQUAKE_EFRAGS			=  2048,  MAX_WINQUAKE_EFRAGS			=   640,
	MAX_FITZQUAKE_DLIGHTS			=    64,  MAX_WINQUAKE_DLIGHTS			=    32,
	MAX_FITZQUAKE_STATIC_ENTITIES	=   512,  MAX_WINQUAKE_STATIC_ENTITIES	=   128,
	MAX_FITZQUAKE_TEMP_ENTITIES		=   256,  MAX_WINQUAKE_TEMP_ENTITIES	=    64,
	MAX_FITZQUAKE_VISEDICTS			=  1024,  MAX_WINQUAKE_VISEDICTS		=   256,

	MAX_FITZQUAKE_LIGHTMAPS			=   256,  MAX_WINQUAKE_LIGHTMAPS		=    64,

	MAX_FITZQUAKE_MAX_EDICTS		= 32000,  MAX_WINQUAKE_EDICTS			=   640,
	MAX_FITZQUAKE_MODELS			=  2048,  MAX_WINQUAKE_MODELS			=   256,
	MAX_FITZQUAKE_SOUNDS			=  2048,  MAX_WINQUAKE_SOUNDS			=   256
} engine_limits;

#if HIGH_MEMORY_SYSTEM

#define DEFAULT_PROTOCOL	PROTOCOL_FITZQUAKE
#define ALLOW_PROTOCOL_666	1									// Normal.
#define MAX_EDICTS_CAP		MAX_EDICTS_PROTOCOL_666				// Maximum dynamic possible.
#define DEFAULT_MAX_EDICTS	MAX_FITZQUAKE_SANE_DEF_EDICTS		// Default statically allocated amount, default dynamic cvar value.  Baker: I do not foresee us using static in the future.

#define	MAX_MSGLEN			MAX_FITZQUAKE_MSGLEN			
#define	MAX_DATAGRAM		MAX_FITZQUAKE_DATAGRAM			

// per-level limits// per-level limits
#define	MAX_BEAMS			MAX_FITZQUAKE_BEAMS				
#define	MAX_EFRAGS			MAX_FITZQUAKE_EFRAGS			
#define	MAX_DLIGHTS			MAX_FITZQUAKE_DLIGHTS			
#define	MAX_STATIC_ENTITIES	MAX_FITZQUAKE_STATIC_ENTITIES	
#define	MAX_TEMP_ENTITIES	MAX_FITZQUAKE_TEMP_ENTITIES		
#define	MAX_VISEDICTS		MAX_FITZQUAKE_VISEDICTS			

#define	MAX_LIGHTMAPS		MAX_FITZQUAKE_LIGHTMAPS			

#define	MAX_MAX_EDICTS		MAX_FITZQUAKE_MAX_EDICTS		
#define	MAX_MODELS			MAX_FITZQUAKE_MODELS			
#define	MAX_SOUNDS			MAX_FITZQUAKE_SOUNDS	

#else	// LOW MEMORY SYSTEM

#define DEFAULT_PROTOCOL	PROTOCOL_NETQUAKE
#define ALLOW_PROTOCOL_666	0									// Do not allow protocol 666?
#define MAX_EDICTS_CAP		MAX_EDICTS_PROTOCOL_15				// Maximum dynamic possible.
#define DEFAULT_MAX_EDICTS  MAX_WINQUAKE_SANE_DEF_EDICTS		// Default statically allocated amount, default dynamic cvar value.  Baker: I do not foresee us using static in the future.

#define	MAX_MSGLEN			MAX_WINQUAKE_MSGLEN			
#define	MAX_DATAGRAM		MAX_WINQUAKE_DATAGRAM			

#define	MAX_BEAMS			MAX_WINQUAKE_BEAMS				
#define	MAX_EFRAGS			MAX_WINQUAKE_EFRAGS			
#define	MAX_DLIGHTS			MAX_WINQUAKE_DLIGHTS			
#define	MAX_STATIC_ENTITIES	MAX_WINQUAKE_STATIC_ENTITIES	
#define	MAX_TEMP_ENTITIES	MAX_WINQUAKE_TEMP_ENTITIES		
#define	MAX_VISEDICTS		MAX_WINQUAKE_VISEDICTS			

#define	MAX_LIGHTMAPS		MAX_WINQUAKE_LIGHTMAPS			

#define	MAX_MAX_EDICTS		MAX_WINQUAKE_MAX_EDICTS		
#define	MAX_MODELS			MAX_WINQUAKE_MODELS			
#define	MAX_SOUNDS			MAX_WINQUAKE_SOUNDS

#endif

#define	MAX_LIGHTSTYLES			64
#define	MAX_STYLESTRING			64
#define	SAVEGAME_COMMENT_LENGTH	39


#define	MAX_SCRAPS				2


// stats are integers communicated to the client by the server
#define	MAX_CL_STATS			32
#define	STAT_HEALTH				0
#define	STAT_FRAGS				1
#define	STAT_WEAPON				2
#define	STAT_AMMO				3
#define	STAT_ARMOR				4
#define	STAT_WEAPONFRAME		5
#define	STAT_SHELLS				6
#define	STAT_NAILS				7
#define	STAT_ROCKETS			8
#define	STAT_CELLS				9
#define	STAT_ACTIVEWEAPON		10
#define	STAT_TOTALSECRETS		11
#define	STAT_TOTALMONSTERS		12
#define	STAT_SECRETS			13		// bumped on client side by svc_foundsecret
#define	STAT_MONSTERS			14		// bumped by svc_killedmonster

// stock defines

#define	IT_SHOTGUN				1
#define	IT_SUPER_SHOTGUN		2
#define	IT_NAILGUN				4
#define	IT_SUPER_NAILGUN		8
#define	IT_GRENADE_LAUNCHER		16
#define	IT_ROCKET_LAUNCHER		32
#define	IT_LIGHTNING			64
#define IT_SUPER_LIGHTNING      128
#define IT_SHELLS               256
#define IT_NAILS                512
#define IT_ROCKETS              1024
#define IT_CELLS                2048

#define IT_AXE                  4096
#define IT_ARMOR1               8192
#define IT_ARMOR2               16384
#define IT_ARMOR3               32768
#define IT_SUPERHEALTH          65536
#define IT_KEY1                 131072
#define IT_KEY2                 262144
#define	IT_INVISIBILITY			524288
#define	IT_INVULNERABILITY		1048576
#define	IT_SUIT					2097152
#define	IT_QUAD					4194304
#define IT_SIGIL1               (1<<28)
#define IT_SIGIL2               (1<<29)
#define IT_SIGIL3               (1<<30)
#define IT_SIGIL4               (1<<31)

//===========================================
//rogue changed and added defines

#define RIT_SHELLS              128
#define RIT_NAILS               256
#define RIT_ROCKETS             512
#define RIT_CELLS               1024
#define RIT_AXE                 2048
#define RIT_LAVA_NAILGUN        4096
#define RIT_LAVA_SUPER_NAILGUN  8192
#define RIT_MULTI_GRENADE       16384
#define RIT_MULTI_ROCKET        32768
#define RIT_PLASMA_GUN          65536
#define RIT_ARMOR1              8388608
#define RIT_ARMOR2              16777216
#define RIT_ARMOR3              33554432
#define RIT_LAVA_NAILS          67108864
#define RIT_PLASMA_AMMO         134217728
#define RIT_MULTI_ROCKETS       268435456
#define RIT_SHIELD              536870912
#define RIT_ANTIGRAV            1073741824
#define RIT_SUPERHEALTH         2147483648

//===========================================
//MED 01/04/97 added hipnotic defines

#define	HIT_PROXIMITY_GUN_BIT	16
#define	HIT_MJOLNIR_BIT			7
#define	HIT_LASER_CANNON_BIT	23
#define	HIT_PROXIMITY_GUN		(1<<HIT_PROXIMITY_GUN_BIT)
#define	HIT_MJOLNIR				(1<<HIT_MJOLNIR_BIT)
#define	HIT_LASER_CANNON		(1<<HIT_LASER_CANNON_BIT)
#define	HIT_WETSUIT				(1<<(23+2))
#define	HIT_EMPATHY_SHIELDS		(1<<(23+3))

//===========================================

#define	MAX_SCOREBOARD			16
#define	MAX_SCOREBOARDNAME		32

#define	SOUND_CHANNELS			8

#include "common.h"

#include "bspfile.h"
#include "engine.h"
#include "vid.h"
#include "sys.h"
#include "memory.h"
#include "mathlib.h"

typedef struct
{
	vec3_t			origin;
	vec3_t			angles;
#if FITZQUAKE_PROTOCOL
	unsigned short 	modelindex; //johnfitz -- was int
	unsigned short 	frame; //johnfitz -- was int
	unsigned char 	colormap; //johnfitz -- was int
	unsigned char 	skin; //johnfitz -- was int
	unsigned char	alpha; //johnfitz -- added
#else
	int				modelindex;
	int				frame;
	int				colormap;
	int				skin;
#endif
	int	effects;
} entity_state_t;

#include "wad.h"
#include "draw.h"
#include "cvar.h"
#include "cl_screen.h"
#include "net.h"
#include "security.h"	// JPG 3.20
#include "protocol.h"
#include "cmd.h"
#include "cl_sbar.h"
#include "sound.h"
#include "render.h"

#include "client.h"
#include "progs.h"
#include "server.h"
//#include "gameatron.h"

#include "model.h"

#include "input.h"
#include "sv_world.h"
#include "keys.h"
#include "console.h"

#include "menu.h"
#include "crc.h"

#include "mp3audio.h"

#include "image.h"

#include "gl_local.h"

#include "nehahra.h"

#include "location.h"	// JPG - for %l formatting speficier
#include "iplog.h"		// JPG 1.05 - ip address logging

//=============================================================================

// the host system specifies the base of the directory tree, the
// command line parms passed to the program, and the amount of memory
// available for the program to use

typedef struct
{
	char	*basedir;
//	char	*cachedir;			// for development over ISDN lines
	int		argc;
	char	**argv;
	void	*membase;
	int		memsize;
} quakeparms_t;

//=============================================================================

//extern	qbool			noclip_anglehack;
//#pragma message ("Quality assurance: Move noclip_anglehack to server if that is where it goes")

// host
extern	quakeparms_t	host_parms;

// Game state
extern	qbool			host_initialized;	// true if into command execution

extern	double			session_startup_time;
extern	qbool			session_in_firsttime_startup;	// As opposed to same session restart on gamedir change
extern	qbool			game_in_initialization;
#if SUPPORTS_DEMO_AUTOPLAY
extern	qbool			client_demoplay_via_commandline;
#endif
extern	char			host_worldname[64];




extern	double			host_frametime;
extern	byte			*host_basepal;
extern	byte			*host_colormap;
extern	int				host_framecount;	// incremented every frame, never reset
extern	double			realtime;						// not bounded in any way, changed at start of every frame, never reset



// JPG 3.20
#ifdef _WIN32 // Baker: Varying argv format?
#define DMSG(x) MessageBox(mainwindow, x,"Stage",MB_OK);// Baker: Will I ever use this again?
extern char	*argv[MAX_NUM_ARGVS];
#elif defined(LINUX)
extern char	**argv;
#endif

void Host_ClearMemory (void);
void Host_ServerFrame (void);
void Host_InitCommands (void);
void Host_Init (quakeparms_t *parms);
void Host_Shutdown (void);
void Host_Error (char *error, ...);
void Host_EndGame (char *message, ...);
void Host_Frame (double time);
void Host_Quit (void);
void Host_Quit_f (void);
void Host_ClientCommands (char *fmt, ...);
void Host_ShutdownServer (qbool crash);
void Host_WriteConfiguration ();

extern qbool	Minimized;
extern qbool	isDedicated;
extern int		minimum_memory;



extern char dequake[256];	// JPG 1.05 - dedicated console translation


#ifdef _WIN32 // Compiler warnings
// Baker D3DQuake
#pragma warning(disable : 4244) /* MIPS conversion to float, possible loss of data */
#pragma warning(disable : 4305) /* MIPS truncation from const double to float */
#pragma warning(disable : 4018) /* MIPS signed/unsigned mismatch */
//#pragma warning(disable : 4101) /* MIPS unreferenced local variable */
// End D3DQuake
#endif


//#pragma message( "_MSC_VER "_MSC_VER )

int build_number (void);
void Host_Version_f (void);
char *VersionString (void);

#endif // __QUAKEDEF_H__