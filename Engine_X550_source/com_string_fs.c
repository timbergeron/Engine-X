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
// com_string_fs.c -- string functions related to file system

#include "quakedef.h"
//#include <assert.h>
//#include <io.h>

// Baker: "c:\quake1\id1\pak0.pak" ---> "c:/quake1/id1/pak0.pak"
void COMD_SlashesForward_Like_Unix (char *WindowsStylePath)
{
	int i;

	// Translate "\" to "/"
	for (i=0 ; i < strlen(WindowsStylePath) ; i++)
		if (WindowsStylePath[i] == '\\')
			WindowsStylePath[i] = '/';
}

// Baker: "c:/quake1/id1/pak0.pak" ---> "c:\quake1\id1\pak0.pak"
void COMD_SlashesBack_Like_Windows (char *UnixStylePath)
{
	int i;

	// Translate "\" to "/"
	for (i=0 ; i < strlen(UnixStylePath) ; i++)
		if (UnixStylePath[i] == '/')
			UnixStylePath[i] = '\\';
}



/*
============
COM_SkipPath
============
*/
// Baker: Make sure nothing depends on this as a pointer into a pathname to modify it
char *StringTemp_SkipPath (const char *pathname)
{
	char			*last;
	char			*walk_pathname;
	// Copy off pathname
	static char		copy_of_pathname[MAX_OSPATH];
	StringLCopy (copy_of_pathname, pathname);
	walk_pathname = copy_of_pathname;

	last = walk_pathname;
	while (*walk_pathname)
	{
		if (*walk_pathname == '/' || *walk_pathname == '\\')
			last = walk_pathname+1;
		walk_pathname++;
	}
	return last;
}

char *StringTemp_SkipPathAndExten (const char *pathname)
{
	static char		copy_of_pathname[MAX_OSPATH];
	char			*returnpos;

	COM_Copy_StripExtension (pathname, copy_of_pathname);
	SLASHJMP (returnpos, copy_of_pathname);

	return returnpos;
}

/*
============
COM_StripExtension
============
*/
// Baker: The same string can be passed as both params to modify it
// Note: I don't see much in the way of string safety here
void COM_Copy_StripExtension (const char *in, char *out)
{
	char	*period_found = strrchr(in, '.');

	if (period_found )
	{
		while (*in && in != period_found)
			*out++ = *in++;
			
		// Null terminate at dot
		*out = 0;
	}
	else
		strlcpy (out, in, strlen(in) + 1); // Cannot StringLCopy 

}

/*
============
StringTemp_FileExtension
============
*/
//#pragma message ("Quality assurance: Validate whether or not StringTemp_FileExtension includes the dot or not.  I'm thinking no")

// Baker: Does not return the dot.  Returns jpg not ".jpg"
char *StringTemp_FileExtension (const char *in)
{
	static	char	exten[8];
	int		i;

	if (!(in = strrchr(in, '.')))
		return "";
	in++;

	for (i=0 ; i<7 && *in ; i++, in++)
		exten[i] = *in;
	exten[i] = 0;

	return exten;
}

/*
============
COM_GetFolder
============
*/
void COM_Copy_GetPath (const char *in, char *out)
{
	char *last = NULL;

	while (*in)
	{
		if (*in == '/' || *in == '\\')  // Baker: Made Windows path friendly  (maybe shouldn't have)
			last = out;
		*out++ = *in++;
	}
	if (last)
		*last = 0;
	else
		*out = 0;
}


/*
==================
COM_ForceExtension

If path doesn't have an extension or has a different extension, append(!) specified extension
Extension should include the .
==================
*/
void COMD_ForceExtension (char *path, const char *extension)
{
	char    *src;

	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src-- == '.')
		{
			COM_Copy_StripExtension (path, path);
			// Baker: make sure this isn't dangerous
			strlcat (path, extension, MAX_OSPATH);
			return;
		}
	}

	strlcat (path, extension, MAX_OSPATH);
#pragma message ("Quality assurance: FS_StringModify_ForceExtension Make sure this isn't dangerous.  MAX_OSPATH isn't necessarily the sizeof the buffer")
}

qbool COM_IsExtension (const char *filename, const char *extension)
{
	// Check to ensure strlen is equal or exceeds strlen extension
	if (strlen(filename) < strlen(extension))	// This allows for stupid "situations like filename is '.dem'
		return false;

	// Now check for match
	if (COM_StringNOTMatchCaseless (filename + strlen(filename) - strlen(extension), extension))
		return false;

	// Matches
	return true;
}


/*
==================
COM_DefaultExtension

If path doesn't have an extension, append extension
Extension should include the .
==================
*/
void COMD_DefaultExtension (char *path, const char *extension)
{
	char    *src;
//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
			return;			// it has an extension
		src--;
	}

	strlcat (path, extension, MAX_OSPATH);
}

qbool COM_ArgIsDemofile (const char *potential_filename)
{
	do
	{
		if (!potential_filename)		// Null pointer check
			break;

		if (!potential_filename[0])		// Null terminator at character 0
			break;

		if (potential_filename[0] == '-')
			break;						// Starts with "-" so is command line switch

		if (potential_filename[0] == '+')
			break;						// Starts with "+" so is command line instruction

		if (!(COM_IsExtension(com_argv[1], ".dem") || COM_IsExtension(com_argv[1], ".dz")))
			break;						// No match for demo file extensions

		return true;
	} while (0);

	return false;
}
void COMD_StripTrailing_UnixSlash (char *modify_path)
{
	if (modify_path[strlen(modify_path)-1] == '/')	// Strip trailing slash
		modify_path[strlen(modify_path)-1] = 0;
}
