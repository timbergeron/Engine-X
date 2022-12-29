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
// com_filesystem_closed.c -- Quake virtual filesystem (functions that can use pak files)

#include "quakedef.h"



/*
=================
QFS_FindFile

finds files in given path including inside paks as well
=================
*/
qbool QFS_FindFile_Pak_And_Path (const char *filename, const char *media_owner_path)
{
	searchpath_t	*search;
	char		netpath[MAX_OSPATH];
	pack_t		*pak;
	int		i;

	for (search = com_searchpaths ; search ; search = search->next)
	{
		if (search->pack)
		{
			pak = search->pack;
			for (i=0 ; i<pak->numfiles ; i++)
				if (COM_StringMatch (pak->files[i].name, filename))
					return true;
		}
		else
		{
			snprintf (netpath, sizeof(netpath), "%s/%s", search->filename, filename);
			if (Sys_FileTime(netpath) != -1)
				return true;
		}
	}

	return false;
}



/*
============
QFS_WriteFile

The filename will be prefixed by the current game directory
============
*/
//QFS_ACCESSPOINT Write
qbool QFS_WriteFile (const char *filename, void *data, int len)
{
	FILE	*f;
	char	name[MAX_OSPATH];

	snprintf (name, MAX_OSPATH, "%s/%s", com_gamedirfull, filename);

	if (!(f = FS_fopen_write(name, "wb", FS_CREATE_PATH)))   // FWRITE
			return false;

	Sys_Printf ("COM_WriteFile: %s\n", name);
	fwrite (data, 1, len, f);
	fclose (f);

	return true;
}


/*
=================
QFS_FOpenFile

Finds the file in the search path.
Sets com_filesize and one of handle or file
=================
*/
//QFS_ACCESSPOINT READ

// Baker: We treat pak files like folders and scribble some notes on the cover about what it has and where
int QFS_File_Category(const char *filename)
{
	int		image_class;
	int		image_folder_class;
	int		classification = 0;

	// Images
	if (COM_IsExtension(filename, "_glow.pcx"))
	{
		image_class = QFS_TYPE_PCX_GLOW_IMAGE;
	}
	else if (COM_IsExtension(filename, "_glow.tga"))
	{
		image_class = QFS_TYPE_TGA_GLOW_IMAGE;
	}
	else if (COM_IsExtension(filename, ".pcx"))
	{
		image_class = QFS_TYPE_PCX_IMAGE;
	}
	else if (COM_IsExtension(filename, ".tga"))
	{
		image_class = QFS_TYPE_TGA_IMAGE;
	}
	else
	{
		// LMPs ... menu

		if (COM_IsExtension(filename, ".lmp"))
		{
			return QFS_TYPE_LMP;
		}

		// Precache type of stuff

		if (COM_IsExtension(filename, ".mdl"))
		{
			return QFS_TYPE_MDL;
		}

		if (COM_IsExtension(filename, ".spr"))
		{
			return QFS_TYPE_SPR;
		}

		if (COM_IsExtension(filename, ".wav"))
		{
			return QFS_TYPE_WAV;
		}

		if (COM_IsExtension(filename, ".bsp"))
		{
			return QFS_TYPE_BSP;
		}

		// External media ... typically searched once per map but often not found.

	//	if (COM_IsExtension(filename, ".lit"))					return QFS_TYPE_LIT;
	//	if (COM_IsExtension(filename, ".ent"))					return QFS_TYPE_ENT;
	//	if (COM_IsExtension(filename, ".vis"))					return QFS_TYPE_VIS;
	//	if (COM_IsExtension(filename, ".loc"))					return QFS_TYPE_LOC;

		// Dat ... generally progs.dat
	//	if (COM_IsExtension(filename, ".dat"))					return QFS_TYPE_DAT;

		// Config type of stuff
	//	if (COM_IsExtension(filename, ".cfg"))					return QFS_TYPE_CFG;
	//	if (COM_IsExtension(filename, ".rc"))					return QFS_TYPE_RC;
	//	if (COM_IsExtension(filename, ".wad"))					return QFS_TYPE_WAD;
	//	if (COM_IsExtension(filename, ".dem"))					return QFS_TYPE_DEM;
	//	if (COM_IsExtension(filename, ".dz"))					return QFS_TYPE_DZ;

		// Baker: an "other kind of file" ... oh yeah, we'll still index it ;)
		return QFS_TYPE_NONE;
	}

	// ===============================================================================================================
	// Image folder classification
	// Case sensitive ... but replacement images do not exist in quake paks and other paks with this
	// kind of content should be modern and made with, say, pakscape
	// ===============================================================================================================

	if (strstr(filename, "gfx/env/"))								image_folder_class = QFSFOLDER_GFX_ENV;
	else if (strstr(filename, "gfx/particles"))						image_folder_class = QFSFOLDER_GFX_PART;
	else if (strstr(filename, "gfx/"))								image_folder_class = QFSFOLDER_GFX;
	else if (strstr(filename, "progs/"))							image_folder_class = QFSFOLDER_PROGS;
	else if (strstr(filename, "textures/"))
	{
		if (strstr(filename, "/exmy"))								image_folder_class = QFSFOLDER_TEXTURES_EXMY;
		else if (strstr(strstr(filename, "textures/") + 9, "/"))	image_folder_class = QFSFOLDER_TEXTURES_ELSE; // Baker: has a subfolder of textures that isn't the above
		else														image_folder_class = QFSFOLDER_TEXTURES;
	}
	else if (strstr(filename, "crosshairs/"))						image_folder_class = QFSFOLDER_CROSSHAIRS;
	else															image_folder_class = QFSFOLDER_NONE;

//	classification = NUM_QFS_TYPES;
//	classification = image_folder_class + NUM_QFS_TYPES;
	classification = image_folder_class * NUM_QFS_TYPES + image_class;
	return classification;
}

qfs_lastloaded_file_t	qfs_lastload;
qbool QFS_FOpenFile (const char *filename, FILE **file, const char *media_owner_path)
{
	searchpath_t	*search;
	qbool			pak_limiter_blocking=false;
	int				file_category = QFS_File_Category(filename);



//#ifdef _DEBUG
//	if (strstr(filename, "anum_7"))
//		search = NULL;
//#endif

	// Invalidate existing data
	qfs_lastload.datasrc        = -1;
	qfs_lastload.offset			= -1;	// Byte offset into file
	qfs_lastload.filelen		= -1;	// File length
	qfs_lastload.datapath[0]	=  0;

	// search through the path, one element at a time
	for (search = com_searchpaths ; search ; search = search->next)
	{
		// is the element a pak file?
		if (search->pack && pak_limiter_blocking)	// We hit earliest accepted pak file.
			continue;
		else if (search->pack)
		{
			pack_t	*pak = search->pack;
			int		i;
			int		starting_index  = 0;
			int		ending_index    = pak->numfiles;	// Baker: Yeah, this isn't the "ending_index" unless we add +1

			do
			{
				if (!pak->has_filetype[file_category])
					break;	// There are no files with that extension class in the pak

				// Ok we've got images
				starting_index = pak->first_index[file_category];
				ending_index   = pak->last_index[file_category] + 1;	// Baker: must +1, you know ... its a "for loop" don't wanna skip last file

				// look through all the pak file elements
				for (/*i=0*/ i = starting_index  ; /* i<pak->numfiles*/ i < ending_index ; i++)
				{
					if (COM_StringNOTMatch (pak->files[i].name, filename))
						continue;	// No match

					// found it!
						if (developer.integer)
							Sys_Printf ("PackFile: %s : %s\n", pak->filename, filename);

						// open a new file on the pakfile
						if (!(*file = FS_fopen_read(pak->filename, "rb")))
							Sys_Error ("Couldn't reopen %s", pak->filename);

					// Success populate data
					qfs_lastload.datasrc = QFS_SOURCE_PAKFILE;
					qfs_lastload.offset  = pak->files[i].filepos;
					qfs_lastload.filelen = pak->files[i].filelen;

					// What pak did we find it in
					snprintf (qfs_lastload.datapath,          sizeof(qfs_lastload.datapath), "%s", pak->filename);

					// More explicitly ... what file position.  Used for precise determination of texture loading comparisons
					snprintf (qfs_lastload.datapath_explicit, sizeof(qfs_lastload.datapath_explicit), "%s#%i", pak->filename, i);

					// advance to correct seek position
					fseek (*file, pak->files[i].filepos, SEEK_SET);
//					Con_Printf ("Data file: %s in %s\n", qfs_lastload.datapath_explicit, filename);
					return true;





				}
			} while (0);

			if (media_owner_path && COM_StringMatchCaseless (media_owner_path, pak->filename))
			{
				Con_DevPrintf (DEV_GAMEDIR, "Search abort: '%s' path limited to pak '%s'.  Further search discontinued.\n", filename, pak->filename);
				pak_limiter_blocking = true; // We will load no more paks
			}
		}
		else
		{
			// check a file in the directory tree
			char file_to_open[MAX_OSPATH];
			snprintf (file_to_open, sizeof(file_to_open), "%s/%s", search->filename, filename);

			// If we didn't find the file, determine if we continue looking or stop
			if (!(*file = FS_fopen_read(file_to_open, "rb")))
				if (media_owner_path
					&&
					(
					(COM_StringMatchCaseless (media_owner_path, search->filename))
												||
					(pak_limiter_blocking && !strncasecmp(media_owner_path, search->filename, strlen(search->filename)))
					)
					)
				{
					Con_DevPrintf (DEV_GAMEDIR, "Search abort: '%s' path limited to '%s'.  Further search discontinued.\n", filename, search->filename);
					break;		// Don't keep looking
				}
				else
					continue;	// Keep looking

			if (developer.integer)
				Sys_Printf ("FOpenFile: %s\n", file_to_open);

			// Success populate data
			qfs_lastload.datasrc = QFS_SOURCE_DIRTREE;
			qfs_lastload.offset  = -1 /* doesn't apply */;
			qfs_lastload.filelen = COM_FileLength (*file);
			StringLCopy (qfs_lastload.datapath,          file_to_open		  );
			StringLCopy (qfs_lastload.datapath_explicit, qfs_lastload.datapath);

			return true;
		}

	}

	// Failure
	if (developer.integer)
		Sys_Printf ("QFS_FOpenFile: can't find %s\n", filename);

	*file = NULL;
	return false;
}


/*
=================
QFS_LoadFile

Filename are relative to the quake directory.
Always appends a 0 byte.
=================
*/
static	cache_user_t 	*loadcache;
static	byte    		*gloadbuf;
static	int             gloadsize;


byte *QFS_LoadFile (const char *path, int usehunk, const char *media_owner_path)
{
	FILE	*h;
	byte	*buf = NULL;	 // quiet compiler warning
	char	base[32];
	int		len;

	// look for it in the filesystem or pack files
	QFS_FOpenFile (path, &h, media_owner_path);
	if (!h)
		return NULL;

	len = qfs_lastload.filelen;

	// extract the filename base name for hunk tag
	QCOM_FileBase (path, base);

	if (usehunk == 1)										// QFS_LoadHunkFile
		buf = Hunk_AllocName (1, len + 1, base);
	else if (usehunk == 2)									// QFS_LoadTempFile
		buf = Hunk_TempAlloc (len + 1);
	else if (usehunk == 0)								
		buf = Z_Malloc (len + 1);
	else if (usehunk == 3)									// QFS_LoadCacheFile ... never used in code
		buf = Cache_Alloc (loadcache, len + 1, base);
	else if (usehunk == 4)									// QFS_LoadStackFile
	{
		if (len + 1 > gloadsize)
			buf = Hunk_TempAlloc (len + 1);
		else
			buf = gloadbuf;
	}
	else
		Sys_Error ("QFS_LoadFile: bad usehunk");

	if (!buf)
		Sys_Error ("QFS_LoadFile: not enough space for %s", path);

	((byte *)buf)[len] = 0;

	Draw_BeginDisc ();

	fread (buf, 1, len, h);
	fclose (h);
	Draw_EndDisc ();

	return buf;
}

byte *QFS_LoadHunkFile (const char *path, const char *media_owner_path)
{
	return QFS_LoadFile (path, 1, media_owner_path);
}

byte *QFS_LoadTempFile (const char *path, const char *media_owner_path)
{
	return QFS_LoadFile (path, 2, media_owner_path);
}

/*  Baker: Never used
void QFS_LoadCacheFile (const char *path, struct cache_user_s *cu, const char *media_owner_path)
{
	loadcache = cu;
	QFS_LoadFile (path, 3, media_owner_path);
}

#pragma message ("QFS_LoadCacheFile How was OSX working without this?")
*/

// uses temp hunk if larger than bufsize
byte *QFS_LoadStackFile (const char *path, void *buffer, int bufsize, const char *media_owner_path)
{
	byte	*buf;

	gloadbuf = (byte *)buffer;
	gloadsize = bufsize;
	buf = QFS_LoadFile (path, 4, media_owner_path);

	return buf;
}

/*
=================
QFS_FindFilesInPak

Search for files inside a PAK file
=================
*/



static void sAddNewEntry_unsorted (char *fname, int ftype, long fsize)
{
	filelist = Q_realloc (filelist, (num_files + 1) * sizeof(direntry_t), "Find files in pak");

#ifdef _WIN32 // file system handle and find file structures
	COMD_toLower (fname);
	// else don't convert, linux is case sensitive
#endif

	filelist[num_files].name = Q_strdup (fname, "Find in files JQ list");
	filelist[num_files].type = ftype;
	filelist[num_files].size = fsize;

	num_files++;
}

qbool CheckEntryName (char *ename);




// Used for maps menu stuff.  Not used by QFS really in normal operation
void QFS_FindFilesInPak (char *the_arg, const int complete_length, const char *media_owner_path)
{
	qbool GameHacks_IsNotQuakeBoxModel (const char *bspname);

	int		i;
	searchpath_t	*search;
	pack_t		*pak;
	char		*myarg;

	SLASHJMP(myarg, the_arg);
	for (search = com_searchpaths ; search ; search = search->next)
	{
		if (search->pack)
		{
			char	*s, *p, ext[8], filename[MAX_FILELENGTH];

			// look through all the pak file elements
			pak = search->pack;
			for (i=0 ; i<pak->numfiles ; i++)
			{
				s = pak->files[i].name;
				strlcpy (ext, StringTemp_FileExtension(s), sizeof(ext));
				if (COM_StringMatchCaseless (ext, StringTemp_FileExtension(myarg)))
				{
					SLASHJMP(p, s);
					if (COM_StringMatchCaseless (ext, "bsp") && !GameHacks_IsNotQuakeBoxModel(p))
						continue;
					if (!strncasecmp(s, the_arg, strlen(the_arg)-5) ||
					    (*myarg == '*' && !strncasecmp(s, the_arg, strlen(the_arg)-5-complete_length)))
					{
						COM_Copy_StripExtension (p, filename);
						if (CheckEntryName(filename))
							continue;
						sAddNewEntry_unsorted (filename, 0, pak->files[i].filelen);
						pak_files++;
					}
				}
			}
		}
	}
}
