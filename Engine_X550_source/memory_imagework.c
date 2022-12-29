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
// zone.c


#include "quakedef.h"

void  Hunk_Lock		(void);
void  Hunk_Unlock	(void);
qbool Hunk_IsLocked (void);


typedef struct
{
	int			processes;
	int			allocations;
	int			bytes;	
	
	int			peak_allocations;
	char		peak_allocations_image[32];
	int			peak_bytes;
	char		peak_bytes_image[32];

} imagework_mapstats_t;


typedef struct
{
	const char	*currentWork;			// Active if not null
	char		descriptive[32];		// 
	char		previous[32];
	int			low_mark;

	int			allocations;
	int			bytes;

	int			qmallocs;
} imagework_t;


static imagework_mapstats_t	LevelStats;
static imagework_t			myImageWork;

static void sImageWork_Stats (const int size)
{
	// Increase the per-image stats
	myImageWork.allocations			++;
	myImageWork.bytes				+= size;

	// Increase the per-map stats
	LevelStats.allocations			++;
	LevelStats.bytes				+= size;

	// Update peak stats if applicable
	if (IntegerMaxExtend(myImageWork.allocations, &LevelStats.peak_allocations))	StringLCopy (LevelStats.peak_allocations_image,	myImageWork.descriptive);
	if (IntegerMaxExtend(myImageWork.bytes, &LevelStats.peak_bytes))				StringLCopy (LevelStats.peak_bytes_image,		myImageWork.descriptive);

}

void ImageWork_ClearMemory (void)
{
	memset (&LevelStats, 0, sizeof(imagework_mapstats_t));	// Reset clear, Maybe print some stats here
}

void ImageWork_LevelStats_Print_f (void)
{
	Con_Printf ("ImageWork Statistics For Level\n");
	Con_Printf ("%s\n", StringTemp_Quakebar(40));
	Con_Printf ("Number of image processed         = %09i\n", LevelStats.processes);
	Con_Printf ("Number of memory allocations made = %09i\n", LevelStats.allocations);
	Con_Printf ("Cumulative bytes allocated        = %09i\n", LevelStats.bytes);

	Con_Printf ("Most allocations per image        = %09i\n", LevelStats.peak_allocations);
	Con_Printf ("... Image Name =                  = %09s\n", LevelStats.peak_allocations_image);
	Con_Printf ("Highest bytes used                = %09i\n", LevelStats.peak_bytes);
	Con_Printf ("... Image Name =                  = %09s\n", LevelStats.peak_bytes_image);
	Con_Printf ("%s\n", StringTemp_Quakebar(40));
}


void ImageWork_Start (const char *system, const char *detail)
{
	// Check conditions
	if (myImageWork.currentWork)
		Sys_Error ("Can't start work for %s|%s, Already in ImageWork %s", system, detail, myImageWork.currentWork);

	myImageWork.currentWork = system;					// What "system" are we in?  QPIC, Alias model, Brush, etc.
	StringLCopy (myImageWork.previous, myImageWork.descriptive);
	StringLCopy (myImageWork.descriptive, detail);		// What specific media item are we dealing with.

	// Act

#if IMAGEWORK_ON_HUNK
	// Store the reset mark
	myImageWork.low_mark = Hunk_LowMark ();

	Hunk_Lock ();
#else
	if (myImageWork.qmallocs)
		Sys_Error ("Qmallocs non-zero at start of image process.  New process is %s %s and last was %s", system, detail, myImageWork.currentWork);
#endif

	// Stats
	LevelStats.processes ++;
}


void ImageWork_Finish (void)
{
	// Check conditions
	if (!myImageWork.currentWork)
		Sys_Error ("Not in imagework, last was %s", myImageWork.currentWork);

	if (!Hunk_IsLocked)
		Sys_Error ("Hunk isn't locked, last was %s", myImageWork.currentWork);

	// Act
#if IMAGEWORK_ON_HUNK
	Hunk_Unlock ();
	if (myImageWork.low_mark == 0)
		Sys_Error ("Low Mark is 0");
	Hunk_FreeToLowMark (myImageWork.low_mark);
#else
	if (myImageWork.qmallocs)
		Sys_Error ("Qmallocs non-zero at end of image process.  Processes %s ", myImageWork.currentWork);
	
#endif

	memset (&myImageWork, 0, sizeof(imagework_t));	// Reset clear
}

void *ImageWork_malloc (const int size, const char *Reason)
{
	void	*p;
	
	// Check conditions
	if (!myImageWork.currentWork)
		Sys_Error ("ImageWork_malloc without current work");

	sImageWork_Stats (size);
	
	// Act
#if IMAGEWORK_ON_HUNK
	Hunk_Unlock ();
	p = Hunk_AllocName (1, size, Reason);
	Hunk_Lock ();
#else
	p = Q_malloc (size, Reason);
	myImageWork.qmallocs ++;
#endif

	return p;
}

void ImageWork_free (void *pointer)
{
	// Check conditions
	if (!myImageWork.currentWork)
		Sys_Error ("ImageWork_free without current work");

	// Act
#if IMAGEWORK_ON_HUNK
	// Just make a note of it, we don't actually have to do anything
#else
	Q_malloc_free (pointer);
	myImageWork.qmallocs --;
#endif


}

/*
void ImageWork_Relax (void)
{
	// Check conditions
	if (!myImageWork.currentWork)
		Sys_Error ("ImageWork_Relax without current work");
	// Act
	Hunk_Unlock ();

}

void ImageWork_UnRelax (void)
{
	// Check conditions
	if (!myImageWork.currentWork)
		Sys_Error ("ImageWork_Relax without current work");
	// Act
	Hunk_Lock ();

}

*/




