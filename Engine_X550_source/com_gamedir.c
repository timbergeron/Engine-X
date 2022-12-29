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
// com_gamedir.c -- anything that supports the Quake virtual file system

#include "quakedef.h"

#ifdef _WIN32
#include <io.h>
#endif

#ifdef MACOSX // Headers
#include <glob.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

/*
=============================================================================

QUAKE FILESYSTEM

=============================================================================
*/

//int     com_filesize;
//char	com_netpath[MAX_OSPATH];

// on disk
typedef struct
{
	char	name[56];
	int	filepos, filelen;
} dpackfile_t;

typedef struct
{
	char	id[4];
	int	dirofs;
	int	dirlen;
} dpackheader_t;

#define MAX_FILES_IN_PACK	4096

// Basedir children - Set at basedir time
char	com_basedir[MAX_OSPATH];    // c:\quake							Baker: FULL path of the basedir
char	com_enginedir[MAX_OSPATH];  // c:\quake\enginex				Baker: FULL path to the enginex engine dir
//char	com_iplogdir[MAX_OSPATH];	// c:\quake\id1						Baker: Always!
// Enginedir children - Set at enginedir time
//char	com_utildir[MAX_OSPATH];    // c:\quake\enginex\utils			Baker: FULL path of the utils dir
//char	com_shotdir[MAX_OSPATH];    // c:\quake\enginex\screenshots	Baker: FULL path of the screenshots dir
// Gamedir children - Set at gamedir time
char	com_gamedirfull[MAX_OSPATH]; // c:\quake\gamedir					Baker: FULL path to the gamedir
//char	com_demodirfull[MAX_OSPATH];    // c:\quake\gamedir\demos			Baker: FULL path of the demodir
//char	com_savedir[MAX_OSPATH]={0};    // c:\quake\gamedir\screenshots		Baker: FULL path of savegame location
//char	com_mapprintdir[MAX_OSPATH];  // c:\quake\id1\maps				The folder it says is your maps folder in config.cfg (even though it might check 2 places!)
// Gamedir shorty
char	com_gamedirshort[MAX_QPATH];    // like quoth						Baker: this is the addition to the basedir to get the gamedir

searchpath_t	*com_base_searchpaths;	// without id1 and its packs
//searchpath_t    *com_searchpaths = NULL;	// JPG 3.20 - added NULL  (This has been promoted to common.h
searchpath_t	*com_verifypaths = NULL;	// JPG 3.20 - use original game directory for verify path (A problem child, for sure!)

/*
================
COM_CheckRegistered

Looks for the pop.lmp file and verifies it.
Sets the "registered" cvar.
Immediately exits out if an alternate game was attempted to be started without
being session_registered.
================
*/

void COM_CheckRegistered (void)
{
	extern char	com_cmdline[CMDLINE_LENGTH];

	FILE	*h;

	QFS_FOpenFile ("gfx/pop.lmp", &h, NULL);
	if (h)
	{
		Cvar_SetDefaultStringByRef (&session_cmdline, com_cmdline);
		Cvar_SetDefaultFloatByRef (&session_registered, 1);
		fclose (h);
	}
}


/*
============
COM_FileBase
============
*/
void QCOM_FileBase (const char *in, char *out)
{
	char	in_copy[MAX_OSPATH];
	char	*s;
	char	*s2;

	StringLCopy (in_copy, in);	// Baker: copy it off

	s = in_copy + strlen(in_copy) - 1;

	while (s != in && *s != '.')
		s--;

	for (s2 = s ; *s2 && *s2 != '/' ; s2--)
	;

	if (s-s2 < 2)
		strcpy (out, "?model?");
	else
	{
		s--;
		strncpy (out, s2+1, s-s2);
		out[s-s2] = 0;
	}
}

// JPG 3.20 - model checking
/*
================
COM_ModelCRC
================
*/
void COM_ModelCRC (void)
{
	searchpath_t    *search;
	byte *data;
	int len, i;

	search = com_searchpaths;
	com_searchpaths = com_verifypaths;
	for (i = 1 ; sv.model_precache[i] ; i++)
	{
		if (sv.model_precache[i][0] != '*')
		{
			data = QFS_LoadFile(sv.model_precache[i], 2, NULL /*PATH LIMIT ME*/);				// 2 = temp alloc on hunk (Baker: cheatfree stuffs)
			if (!data)
				Host_Error("COM_ModelCRC: Could not load %s", sv.model_precache[i]);
			len = (*(int *)(data - 12)) - 16;							// header before data contains size
			sv.model_crc[i] = Security_CRC(data, len);
		}
	}
	com_searchpaths = search;
}


void COM_Printdirs_f (void)
{
	// Baker: prints all the dir locations for debugging and such

	Con_Printf("com_basedir:     %s\n", com_basedir);
	Con_Printf("com_enginedir:   %s\n", com_enginedir);
	Con_Printf("com_iplogdir:    %s\n", FULLPATH_TO_IPLOGDIR);
	// Enginedir children - Set at enginedir time
	Con_Printf("com_utildir:     %s\n", FOLDER_UTILS);
	Con_Printf("com_shotdir:     %s\n", FOLDER_SHOTS);
	// Gamedir children - Set at gamedir time
	Con_Printf("com_gamedirfull: %s\n", com_gamedirfull);
	Con_Printf("com_demodirfull: %s\n", FOLDER_DEMOS);
	Con_Printf("com_savedir:     %s\n", FOLDER_SAVES);
	// Gamedir shorty
	Con_Printf("com_gamedirshort:%s\n", com_gamedirshort);
	Con_Printf("qfs_lastload.datapath:     %s\n", qfs_lastload.datapath);
}



/*
============
COM_Path_f
============
*/
void COM_Path_f (void)
{
	searchpath_t	*s;

	Con_Printf ("Current search path:\n");
	for (s = com_searchpaths ; s ; s = s->next)
	{
		if (s == com_base_searchpaths)
			Con_Printf ("------------\n");
		if (s->pack)
		{
			Con_Printf ("%s (%i files)\n", s->pack->filename, s->pack->numfiles);
		}
		else
			Con_Printf ("%s\n", s->filename);
	}
}




/*
=================
COM_LoadPackFile

Takes an explicit (not game tree related) path to a pak file.

Loads the header and directory, adding the files at the beginning
of the list so they override previous pack files.
=================
*/
int QFS_File_Category(const char *filename);
pack_t *COM_LoadPackFile (char *packfile)
{
	int				i, numpackfiles;
	dpackheader_t	header;
	packfile_t		*newfiles;
	pack_t			*pack;
	FILE			*packhandle;
	dpackfile_t		info[MAX_FILES_IN_PACK];
	int				file_category;

	if (COM_FileOpenRead(packfile, &packhandle) == -1)
		return NULL;

	fread (&header, 1, sizeof(header), packhandle);
	if (memcmp(header.id, "PACK", 4))
		Sys_Error ("%s is not a packfile", packfile);
	header.dirofs = LittleLong (header.dirofs);
	header.dirlen = LittleLong (header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		Sys_Error ("%s has %i files", packfile, numpackfiles);

	newfiles = Q_malloc (numpackfiles * sizeof(packfile_t), "Pak files lists");

	fseek (packhandle, header.dirofs, SEEK_SET);
	fread (&info, 1, header.dirlen, packhandle);

	// Image lookup speedup
	pack = Q_malloc (sizeof(pack_t), "Pack file");
	StringLCopy (pack->filename, packfile);
	pack->handle = packhandle;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;

	{	// Speed up begin
		int k;
		for (k = 0; k < (NUM_QFS_TYPES * NUM_QFSFOLDER_TYPES) ; k++)
		{
			pack->has_filetype[k]	= false;
			pack->first_index[k]	= -1;
			pack->last_index[k]		= 0;
		}
	}

	// parse the directory
	for (i=0 ; i<numpackfiles ; i++)
	{
		strcpy (newfiles[i].name, info[i].name);
		newfiles[i].filepos = LittleLong (info[i].filepos);
		newfiles[i].filelen = LittleLong (info[i].filelen);

		// Speed up begin
		file_category = QFS_File_Category(info[i].name);
		pack->has_filetype[file_category] = true;
		if (pack->first_index[file_category]<0)
			pack->first_index[file_category] = i;
		pack->last_index[file_category] = i;
		// Speed up end

	}



	Con_Printf ("Added packfile %s (%i files)\n", packfile, numpackfiles);

	return pack;
}

static qbool COM_AddPak (char *pakfilefullpath)
{
	searchpath_t *search;
	pack_t *pak;

	// Baker: next line not necessary because we already have it
	//Q_snprintfz (pakfilefullpath, sizeof(pakfilefullpath), "%s/pak%i.pak", dir, i);
	pak = COM_LoadPackFile(pakfilefullpath);
	if (!pak)
		return false; //do not pass go, do not collect $500

	search = Q_malloc (sizeof(searchpath_t), "Pak searchpath");
	search->pack = pak;
	search->next = com_searchpaths;
	com_searchpaths = search;
	return true;
}


static void COM_SetPath_GameDirFull(const char *shortdir)
{
//	int	i;
	// This is where the gamedirname gets set!
	StringLCopy (com_gamedirshort, shortdir);
	snprintf (com_gamedirfull, sizeof(com_gamedirfull), "%s/%s", com_basedir, com_gamedirshort);


	// Other dirs
	// Demo dir is always gamedir/demos

//	StringLCopy (com_demodirfull, va("%s/%s/demos", com_basedir, shortdir), sizeof(com_demodirfull));
//	StringLCopy (com_demodirfull, va("%s/%s/demos", com_basedir, shortdir), sizeof(com_demodirfull));


	// Savedir is always gamedir EXCEPT if gamedir is enginex
	// If gamedir is enginex, it is id1!

//	if (COM_StringMatchCaseless (com_gamedirshort, ENGINEFOLDER))
//	{
//		// No savegames in enginex folder, use id1 instead
//		StringLCopy (com_savedir, va("%s/id1", com_basedir, shortdir), sizeof(com_savedir));
//	}
//	else
//	{
//		// The norm
//		StringLCopy (com_savedir, va("%s/%s", com_basedir, shortdir), sizeof(com_savedir));
//	}
	Con_Printf("Gamedir paths are set\n");

}

/*
================
COM_AddGameSubDirectory

Sets com_gamedir, adds the directory to the head of the path,
then loads and adds pak1.pak pak2.pak ...
================
*/
static void COM_AddGameSubDirectory (char *shortdir) // Baker: dir WAS a fullpath, but now I changed to expect relative path from basedir
{
	int		i;
	searchpath_t	*search;
	char		pakfile[MAX_OSPATH]; //, *p;

	COM_SetPath_GameDirFull(shortdir); // Set the gamedir with Mr. Shorty Gamedir

// add the directory to the search path
	search = Q_malloc (sizeof(searchpath_t), "Path searchpath");
	StringLCopy (search->filename, com_gamedirfull);
	search->pack = NULL;
	search->next = com_searchpaths;
	com_searchpaths = search;

	// Baker: if Engine X folder, load any paks else use standard scheme
	if (COM_StringNOTMatchCaseless (com_gamedirshort, ENGINEFOLDER)) // *NOT* Engine X folder
	{
		// Not enginex folder, use traditional method

		// add any pak files in the format pak0.pak pak1.pak, ...
		for (i=0 ; ; i++)
		{
			snprintf (pakfile, sizeof(pakfile), "%s/pak%i.pak", com_gamedirfull, i);

			if(!COM_AddPak(pakfile)) // If no match, terminate pakx.pak search sequence
				break;
		}

	}
	else
	{
		// Engine X folder; load any paks found
		// Adapted from "add ANY pak file -   MrG (biteme@telefragged.com)
		char	dirstring[1024];
	#ifdef _WIN32 // dirent

		int		handle;
		struct	_finddata_t fileinfo;
//		HANDLE		h;
//		WIN32_FIND_DATA	fileinfo;
	#else
		int		handle, i = 0;
		char		*foundname;
		glob_t		fd;
		struct	stat	fileinfo;
	#endif

    	// Load enginex.pak first
		StringLCopy (pakfile, FULLPATH_TO_EX_PAK);
		if (!COM_AddPak(pakfile))
		{
			Sys_Error ("Engine_X.pak file is missing"); // Required file: throw a fit
		}

	   	// Load enginex_extras.pak next
		StringLCopy (pakfile, FULLPATH_TO_EXX_PAK);
		if (!COM_AddPak(pakfile))
		{
			StringLCopy (pakfile, FULLPATH_TO_EXX2_PAK);	// Try pcx lite version
			if (!COM_AddPak(pakfile))
				Con_Printf ("Engine_X_Extras pak is missing.  Not required ... ignoring\n");
		}

		snprintf (dirstring, sizeof(dirstring), "%s/*.pak", com_gamedirfull);
#ifdef _WIN32
//		handle = FindFirstFile (dirstring, &fd)
//		if (handle != INVALID_HANDLE_VALUE)
		handle = _findfirst (dirstring, &fileinfo);

      	if (handle != -1)
#else
		handle = glob (dirstring, 0, NULL, &fd);
		if (handle != GLOB_ABORTED)
#endif
		{
			do
			{
#ifdef _WIN32
				if (fileinfo.name[0] == '.')
					continue;

				if (COM_StringMatchCaseless (fileinfo.name, ONLYFILENAME_EX_PAK) || COM_StringMatchCaseless (fileinfo.name, ONLYFILENAME_EXX_PAK) || COM_StringMatchCaseless (fileinfo.name, ONLYFILENAME_EXX2_PAK))
					continue;  // We already read these pak files

				snprintf (pakfile, sizeof(pakfile), "%s/%s", com_gamedirfull, fileinfo.name);
#else
				if (handle == GLOB_NOMATCH || !fd.gl_pathc)
					break;

				stat (fd.gl_pathv[i], &fileinfo);

				if (S_ISDIR(fileinfo.st_mode))
					continue;

				SLASHJMP(foundname, fd.gl_pathv[i]);

				if (COM_StringMatchCaseless (foundname, ONLYFILENAME_EX_PAK) || COM_StringMatchCaseless (foundname, ONLYFILENAME_EXX_PAK) || COM_StringMatchCaseless (foundname, ONLYFILENAME_EXX2_PAK))
					continue;  // We already read these pak files

				snprintf (pakfile, sizeof(pakfile), "%s/%s", com_gamedirfull, foundname);

#endif


				if (!COM_AddPak(pakfile))
					break;
#ifdef _WIN32
				Con_Printf("adding Engine X %s pak file\n", fileinfo.name);
#else
				Con_Printf("adding Engine X %s pak file\n", foundname);
#endif
			}
#ifdef _WIN32
        	while (_findnext( handle, &fileinfo ) != -1);
			_findclose (handle);
//			while (FindNextFile(handle, &fileinfo));
//			FindClose (handle);
#else
			while (++i < fd.gl_pathc);
			globfree (&fd);
#endif

		}
		// End Engine X folder
//#endif
  	}

	// initializing demodir (probably only the default demodir and this changes dynamically, for demo menu only)
	snprintf (demodir, sizeof(demodir), "/%s/demos", com_gamedirshort); // Baker: Not sure this is "right" since we want demos in enginex/demos
}

/*
================
COM_SetGameDir

Sets the gamedir and path to a different directory.
================
*/

// Baker: This is ONLY used for shift on the fly gamedir changing!
//        It is never used except by the GAMEDIR console command!

static void COM_SetGameDir (char *shortdir)
{
	searchpath_t	*next;

	if (strstr(shortdir, "..") || strstr(shortdir, "/") || strstr(shortdir, "\\") || strstr(shortdir, ":"))
	{
		Con_Printf ("Gamedir should be a single filename, not a path\n");
			return;
	}

	if (COM_StringMatchCaseless (com_gamedirshort, shortdir))
	{
		// Baker: this should be unreachable code because we are checking in game_f as well
		Con_Printf("Can't set gamedir %s because it is already set!\n", shortdir);
		return;		// still the same
	}
	// free up any current game dir info
	Con_Printf("\nReleasing pak files ...\n");
	while (com_searchpaths != com_base_searchpaths)
	{
		if (com_searchpaths->pack)
		{
			Con_Printf("Releasing %s ...\n", com_searchpaths->pack->filename);
			fclose (com_searchpaths->pack->handle);
			Q_malloc_free (com_searchpaths->pack->files);
			Q_malloc_free (com_searchpaths->pack);
		}
		next = com_searchpaths->next;
		Q_malloc_free (com_searchpaths);
		com_searchpaths = next;
	}
	Con_Printf("Release complete.\n\n");

	// flush all data, so it will be forced to reload
	Cache_Flush_f ();

	if (COM_StringMatchCaseless (shortdir, "id1") || COM_StringMatchCaseless (shortdir, "enginex"))
	{
		// Ugly ... we are going to set the gamedir here because we aren't heading off to COM_AddGameSubDirectory
		// which does this.
		// We could make a scheme for AddGameDirectory to know whether or not to add id1/enginex
		// but this is easier for now.
		COM_SetPath_GameDirFull("enginex"); // Set the gamedir with Mr. Shorty Gamedir
		Con_Printf("Gamedir reset to base: %s\n", com_gamedirshort);
	}
	else
	{
		// Gamedir isn't id1 or enginex, so add the gamedirectory
		COM_AddGameSubDirectory(/*com_basedir,*/ shortdir); // was a return with the gamedir add logic afterwards
	}

}

/*
================
COM_Gamedir_f

Sets the gamedir and path to a different directory.
================
*/
//qbool gbGameDirChanged = false;
void COM_Gamedir_f (void)
{
	char	*shortdir;

	if (cmd_source != src_command)
		return;

#pragma message ("Quality assurance.  Do not allow gamedir switches if -game in command line param?")

	if (Cmd_Argc() == 1)
	{
		Con_Printf ("Current gamedir: %s\n", com_gamedirshort);
		return;
	}

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("Usage: %s <newdir>\n", Cmd_Argv (0));
		return;
	}

	// Baker: let's try to detect the nothing scenario of ""

	shortdir = Cmd_Argv (1);

	if (strstr(shortdir, "..") || strstr(shortdir, "/") || strstr(shortdir, "\\") || strstr(shortdir, ":") )
	{
		Con_Printf ("gamedir should be a single filename, not a path\n");
		return;
	}

	// Baker: a gamedir of "" or a gamedir of "id1" should reset us to the enginex folder
	//        except the problem with this is -game
	if (shortdir[0] == 0 || COM_StringMatchCaseless (shortdir, "id1"))
	{
		// Empty gamedir
		shortdir = ENGINEFOLDER;
		Con_DevPrintf(DEV_GAMEDIR, "Translating empty gamedir to %s\n", shortdir);
	}

	if (COM_StringMatchCaseless (com_gamedirshort, shortdir))
	{
		if (COM_StringMatchCaseless (shortdir, ENGINEFOLDER))
			Con_Printf("Gamedir is already set to base gamedir\n");
		else
			Con_Printf("Can't set gamedir to %s because it is already set!\n", shortdir);

		return;		// still the same
	}


	if (cls.state != ca_disconnected)
	{
		CL_Disconnect_f ();	// Baker says: let's disconnect them instead of denying them
	}





	do
	{
		void Host_WriteConfig (char *cfgname);
		void Cmd_Unaliasall_f (void);
		void Key_Unbindall_f (void);
		void Cvar_EngineResetAll_f (void);

		int stage = 1;

		// Client pre gamedir change routine
		// Save configuration
		if (isDedicated) goto stage2;

		Con_Printf  ("Stage %i: Save current configuration  ...\n", stage++);
		Host_WriteConfig ("config.cfg");

stage2: // Actual gamedir change

		Con_Printf  ("Stage %i: Changing gamedir  ...\n", stage++);
		COM_SetGameDir (shortdir);

		Con_Printf  ("Stage %i: Clearing models  ...\n", stage++);

		Mod_ClearAll (1);	// Baker: This tells all the models to reload.  Oddly enough, it does "fairly well" without this.
							//        Except for player.mdl.  I wonder why.

		// At this point ... dedicated server jumps ahead
		if (!isDedicated)	goto stage3;

		// Client post stage

		Con_Warning ("Stage %i: NOT DONE ..glDelete all textures ...\n", stage++);
		Con_Warning ("Stage %i: NOT DONE ... clear all sounds.  Does this need done?...\n", stage++);

		Con_Warning ("Stage %i: NOT DONE ... Reload palette, conback, charset, clear menu lmps and HUD gfx ...\n", stage++);


		Cmd_Unaliasall_f ();

		Con_Printf  ("Stage %i: Reset all keys ...\n", stage++);
		Key_Unbindall_f ();

stage3:	Con_Printf  ("Stage %i: Reset all aliases ...\n", stage++); // Server might have some of this cruft
		Con_Printf  ("Stage %i: Hard reset of all engine cvars ...\n", stage++);
		Cvar_EngineResetAll_f ();

		Con_Printf  ("Stage %i: Set to accept game cvar defaults ...\n", stage++);
		game_in_initialization = true;
//		accepts_default_values = true;
#pragma message ("IMPORTANT! IMPORTANT! IMPORTANT! For consistency ... verify that 'exec default.cfg' sets to accept cvar defaults and not here")

		Con_Printf  ("Stage %i: Executing quake.rc ...\n", stage++);
		Con_Printf  ("Gamedir reset scheme complete\n");

		Cbuf_AddText ("exec quake.rc\n");
		Cbuf_AddText ("\nhint_gameinitialized\n");

		if (!isDedicated)
			continue;

		// Client ... close the console IF it was opened
		// Baker: This is wrong, but it'll do for now ...
		Cbuf_AddText ("toggleconsole\n");

	} while (0);

	// Baker: I think the server runs autoexec.cfg.  We need some way for the server
	// to be communicated the sv_maxspeed and such.
}

//// INITIALIZATION ONLY
//// INITIALIZATION ONLY
//// INITIALIZATION ONLY


static void COM_SetPath_BaseDir(const char *absolute_dir)
{
	StringLCopy (com_basedir, absolute_dir);	// This is where the basedir gets set!
	Con_DevPrintf (DEV_GAMEDIR, "com_basedir set to \"%s\"\n", com_basedir);

	COMD_SlashesForward_Like_Unix (com_basedir);


	{	// Remove trailing (Unix Style) slash
		int	i = strlen(com_basedir) - 1;
		if (i >= 0 && com_basedir[i] == '/')
			com_basedir[i] = 0;
	}

	Con_DevPrintf (DEV_GAMEDIR, "com_basedir cleaned up to \"%s\"\n", com_basedir);

	snprintf (com_enginedir, sizeof(com_enginedir), "%s/%s", com_basedir, ENGINEFOLDER);

	// Enginedir children - Set at enginedir time
//	StringLCopy (com_utildir, va("%s/utils", com_enginedir), sizeof(com_utildir)); // Put fmod, dzip and other such ugly stepchildren in here
//	StringLCopy (com_shotdir, va("%s/screenshots", com_enginedir), sizeof(com_shotdir)); // Single point screenshots folder

	Con_Printf("Basedir paths are set\n");
}


/*
================
COM_InitFilesystem
================
*/
void COM_InitFilesystem (void)
{
	int	i;


	// Baker: We are dropping support for -basedir command line parameter
	//        First, we have external .dlls and these get linked immediately on startup so no point
	//        Second, simply no need to create a giant clustermess

	COM_SetPath_BaseDir (host_parms.basedir);

	// start up with GAMENAME by default (id1)
	// Baker note: we are no longer sending the full path to COM_AddGameDirectory, just the short path like "quoth"
	//             because COM_AddGameDirectory knows the basedir
	COM_AddGameSubDirectory (GAMENAME);

	if (!isDedicated)
		COM_AddGameSubDirectory (ENGINEFOLDER);

	if (COM_CheckParm("-rogue"))
		COM_AddGameSubDirectory ("rogue");
	if (COM_CheckParm("-hipnotic"))
		COM_AddGameSubDirectory ("hipnotic");
	if (COM_CheckParm ("-quoth"))
		COM_AddGameSubDirectory ("quoth");

#ifdef GLQUAKE
	if (COM_CheckParm("-nehahra"))
        	COM_AddGameSubDirectory ("nehahra");
#endif

	// any set gamedirs will be freed up to here
	com_base_searchpaths = com_searchpaths;

// -game <gamedir>
// Adds basedir/gamedir as an override game
	if ((i = COM_CheckParm("-game")) && i < com_argc-1)
        COM_AddGameSubDirectory (com_argv[i+1]);
}

