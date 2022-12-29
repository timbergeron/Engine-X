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
// sys_win.c -- Win32 system interface code

#include "quakedef.h"

#include <direct.h>		// _mkdir
#include <sys/stat.h>	// _stat



/*
===============================================================================
FILE IO
===============================================================================
*/


int Sys_FileTime (const char *path)
{
//	FILE	*f;
//	int	retval;
//
//	if ((f = FS_fopen_read(path, "rb")))
//	{
//		fclose (f);
//		retval = 1;
//	}
//	else
//	{
//		retval = -1;
//	}


//	return retval;


  struct _stat buf;
   int result;
   char buffer[] = "A line to output";

   /* Get data associated with "stat.c": */
   result = _stat( path, &buf );

   /* Check if statistics are valid: */
   if( result != 0 )
      return -1; // perror( "Problem getting information" );
//   else
  // {
      /* Output some of the statistics: */
//      printf( "File size     : %ld\n", buf.st_size );
//    printf( "Drive         : %c:\n", buf.st_dev + 'A' );
//      printf( "Time modified : %s", ctime( &buf.st_atime ) );
//   }

	return 1;
}


