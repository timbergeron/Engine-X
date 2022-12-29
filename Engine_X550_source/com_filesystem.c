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
// com_filesystem.c -- file system stuff.  Not virtual.  Real.

#include "quakedef.h"


/*
================
COM_FileLength
================
*/
int COM_FileLength (FILE *f)
{
	int	pos, end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

/*
=================
COM_FileOpenRead

Use this instead of Sys_FileOpenRead
=================
*/
int COM_FileOpenRead (const char *path, FILE **hndl)
{
	FILE	*f;

	if (!(f = FS_fopen_read(path, "rb")))
	{
		*hndl = NULL;
		return -1;
	}
	*hndl = f;

	return COM_FileLength (f);
}


/*
============
COM_CreatePath

Only used for CopyFile
============
*/
// Baker: You must have a trailing slash if just purely making a folder
//        The intent of this is building the path where the file is
//        so this is generally intended to make the path the file is in
void COM_CreatePath (const char *AbsoluteFileName)
{
	char	temppath[MAX_OSPATH];
	char	*ofs;

	StringLCopy (temppath, AbsoluteFileName);

	Con_DevPrintf (DEV_GAMEDIR, "Making a path \"%s\"\n", AbsoluteFileName);

	for (ofs = temppath+1 ; *ofs ; ofs++)
	{
		if (*ofs == '/')
		{       // create the directory
			*ofs = 0;
			Sys_mkdir (temppath);
			*ofs = '/'; // Baker: not really necessary as we purified to const
		}
	}
}


/*
===========
COM_CopyFile

Copies a file over from the net to the local cache, creating any directories
needed. This is for the convenience of developers using ISDN from home.
===========
*/
void COM_CopyFile (const char *netpath, const char *cachepath)
{
	FILE	*in, *out;
	int	remaining, count;
	char	buf[4096];

	remaining = COM_FileOpenRead (netpath, &in);

	if (!(out = FS_fopen_write(cachepath, "wb", FS_CREATE_PATH)))
		Sys_Error ("Error opening %s", cachepath);

	while (remaining)
	{
		if (remaining < sizeof(buf))
			count = remaining;
		else
			count = sizeof(buf);
		fread (buf, 1, count, in);
		fwrite (buf, 1, count, out);
		remaining -= count;
	}

	fclose (in);
	fclose (out);
}

//#define FS_CREATE_PATH 1
FILE *FS_fopen_write(const char *filename, const char *mode, qbool bCreatePath)
{
	FILE *f;

	// SECURITY: Compare all file writes to com_basedir
	if (strlen(filename) < strlen(com_basedir) || strncasecmp (filename, com_basedir, strlen(com_basedir))!=0 || strstr(filename, "..") )
		Sys_Error ("Security violation:  Attempted path is %s\n\nWrite access is limited to %s folder tree.", filename, com_basedir);

	f = fopen (filename, mode);		// fopen OK

	if (!f && bCreatePath == FS_CREATE_PATH && strstr(mode,"w"))
	{
		// If specified, on failure to open file for write, create the path
		COM_CreatePath (filename);
		f = fopen (filename, mode); // fopen OK
	}

	Con_DevPrintf (DEV_GAMEDIR, "File open for write: '%s' (%s) %s\n", filename, mode, f ? "" : "(couldn't find file)");
	return f;

}

//#define FS_CREATE_PATH_IF_WRITE = true
FILE *FS_fopen_read(const char *filename, const char *mode)
{
	// Baker: Sole purpose of this would be a centralized dev mode like dev FS_
	FILE *f = fopen (filename, mode); // fopen OK

//	Con_DevPrintf (DEV_GAMEDIR, "File open for read: '%s' (%s) %s\n", filename, mode, f ? "" :  "(couldn't find file)");

	return f;
}


// Return filename fullpath from successful open
char *FS_Open_DirTree_GetName (const char *file_to_find)
{
	static char namebuffer [MAX_OSPATH];
	searchpath_t	*search;
	FILE *f;

	// Look through each search path
	for (search = com_searchpaths ; search ; search = search->next)
	{
		// Ignore pack files, we could one up this by not ignoring pak files and copying bytes out to temp dir file
		if (!search->pack)
		{
			snprintf (namebuffer, sizeof(namebuffer), "%s/%s", search->filename, file_to_find);
			f = FS_fopen_read (namebuffer, "rb");

			if (f)
			{
				fclose (f);
				Con_DevPrintf (DEV_GAMEDIR, "Located file '%s' in '%s'\n", file_to_find, search->filename);
				return namebuffer;
			}
			else
				Con_DevPrintf (DEV_GAMEDIR, "Failed to locate file '%s' in '%s'\n", file_to_find, search->filename);
		}
	}

	return NULL;	// Failure
}

