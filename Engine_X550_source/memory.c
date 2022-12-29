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
#include "memory_local.h"


#define	HUNK_SENTINAL	0x1df001ed

typedef struct
{
	int	sentinal;
	int	size;		// including sizeof(hunk_t), -1 = not allocated
	char	name[8];
} hunk_t;

byte		*hunk_base;
int		hunk_size;

int		hunk_low_used;
int		hunk_high_used;

qbool 	isLocked;

static qbool	hunk_tempactive;
static int		hunk_tempmark;

void  Hunk_Lock		(void)	{	isLocked = true;	}
void  Hunk_Unlock	(void)	{	isLocked = false;	}
qbool Hunk_IsLocked (void)	{   return isLocked;	}

void R_FreeTextures (void);
// Hunk_Check: Run consistency and sentinal trashing checks
static void sHunk_Check_Single (const hunk_t *h)
{
	const int	sizeof_hunk_t = sizeof(hunk_t);
	qbool		is_bad_size_toosmall = h->size < sizeof_hunk_t;											// Hunk size is less than minimum possible, which is 16 + size 0 = 16
	qbool		is_bad_size_toolarge = ((byte *)h - hunk_base) + h->size > hunk_size;	// Hunk memory address is outside the hunk area
	qbool		trashed_sentinal	 = h->sentinal != HUNK_SENTINAL;									// Key doesn't equal sentinal key

	if (trashed_sentinal)								Sys_Error ("Hunk_Check: trashed sentinal");
	if (is_bad_size_toosmall ||  is_bad_size_toolarge)	Sys_Error ("Hunk_Check: bad size");
}

void Hunk_Check (void)
{
	hunk_t	*h;
	int			i=0;

	for (h = (hunk_t *)hunk_base ; (byte *)h != hunk_base + hunk_low_used ; i++, h = (hunk_t *)((byte *)h+h->size))
		sHunk_Check_Single (h);  //		h = (hunk_t *)((byte *)h + h->size);	// Advance to next allocation
}

/*
===================
Hunk_AllocName
===================
*/
void *Hunk_AllocName (const int num, const int in_size, const char *name)
{
	hunk_t	*h;
	int		size = num*in_size;

#ifdef PARANOID
	Hunk_Check ();
#endif
	if (isLocked)											Sys_Error ("Attempt to use locked hunk");
	if (size < 0)											Sys_Error ("Hunk_Alloc: bad size: %i", size);
	if (hunk_size - hunk_low_used - hunk_high_used < size)	Sys_Error ("Not enough RAM allocated.  Try using \"-mem 128\" on the command line.");

	size = sizeof(hunk_t) + ((size + 15) & ~15);

//		Sys_Error ("Hunk_Alloc: failed on %i bytes",size);

	h = (hunk_t *)(hunk_base + hunk_low_used);
	hunk_low_used += size;

	Cache_FreeLow (hunk_low_used);

	memset (h, 0, size);

	h->size = size;
	h->sentinal = HUNK_SENTINAL;
	strncpy (h->name, name, 8);

	return (void *)(h + 1);
}

int Hunk_LowMark (void)
{
	if (isLocked)										Sys_Error ("Attempt to use locked hunk");
	
	return hunk_low_used;
}

void Hunk_FreeToLowMark (const int mark)
{
	if (isLocked)										Sys_Error ("Attempt to use locked hunk");
	if (mark < 0 || mark > hunk_low_used)				Sys_Error ("Hunk_FreeToLowMark: bad mark %i", mark);
	memset (hunk_base + mark, 0, hunk_low_used - mark);
	hunk_low_used = mark;
}

int Hunk_HighMark (void)
{
	if (hunk_tempactive)
	{
		hunk_tempactive = false;
		Hunk_FreeToHighMark (hunk_tempmark);
	}

	return hunk_high_used;
}

void Hunk_FreeToHighMark (const int mark)
{
	if (hunk_tempactive)
	{
		hunk_tempactive = false;
		Hunk_FreeToHighMark (hunk_tempmark);
	}
	if (mark < 0 || mark > hunk_high_used)
		Sys_Error ("Hunk_FreeToHighMark: bad mark %i", mark);
	memset (hunk_base + hunk_size - hunk_high_used, 0, hunk_high_used - mark);
	hunk_high_used = mark;
}

/*
===================
Hunk_HighAllocName
===================
*/
void *Hunk_HighAllocName (const int in_size, const char *name)
{
	hunk_t	*h;
	int		allocsize;

	if (in_size < 0)
		Sys_Error ("Hunk_HighAllocName: bad size: %i", in_size);

	if (hunk_tempactive)
	{
		Hunk_FreeToHighMark (hunk_tempmark);
		hunk_tempactive = false;
	}

#ifdef PARANOID
	Hunk_Check ();
#endif

	allocsize = sizeof(hunk_t) + ((in_size + 15) & ~15);

	if (hunk_size - hunk_low_used - hunk_high_used < allocsize)
	  	Sys_Error ("Hunk_HighAllocName: Not enough RAM allocated.  Try using \"-mem 128\" on the command line.");
//	{
//		Con_Printf ("Hunk_HighAlloc: failed on %i bytes\n",size);
//		return NULL;
//	}

	hunk_high_used += allocsize;
	Cache_FreeHigh (hunk_high_used);

	h = (hunk_t *)(hunk_base + hunk_size - hunk_high_used);

	memset (h, 0, allocsize);
	h->size = allocsize;
	h->sentinal = HUNK_SENTINAL;
	strncpy (h->name, name, 8);

	return (void *)(h + 1);
}

/*
=================
Hunk_TempAlloc

Return space from the top of the hunk
=================
*/


void *Hunk_TempAlloc (const int in_size)
{
	void	*buf;
	int		allocsize = (in_size + 15) & ~15;

	if (hunk_tempactive)
	{
		Hunk_FreeToHighMark (hunk_tempmark);
		hunk_tempactive = false;
	}

	hunk_tempmark = Hunk_HighMark ();

	buf = Hunk_HighAllocName (allocsize, "temp");

	hunk_tempactive = true;

	return buf;
}




void Memory_InitZone (/*memzone_t *zone, */ const int zonesize_in_bytes);
void Cache_Init (void);
/*
========================
Memory_Init
========================
*/
#define	ZONE_DEFAULT_SIZE	0x100000	// 1Mb
void Hunk_Print_f (void);
void ImageWork_LevelStats_Print_f (void);
void Memory_Init (const int hunksize_in_bytes)
{
	int		zonesize_kb			= COM_Commandline_GetInteger("-zone");
	int		zonesize_in_bytes	= zonesize_kb ? zonesize_kb * 1024: ZONE_DEFAULT_SIZE;	

	hunk_base		= Q_malloc (hunksize_in_bytes, "mainhunk");	// Q_malloc will Sys_Error us if malloc fails
//	hunk_base 		= buf;
	hunk_size 		= hunksize_in_bytes;
	hunk_low_used 	= 0;
	hunk_high_used 	= 0;

	Cache_Init ();

	Memory_InitZone (zonesize_in_bytes);

	Cmd_AddCommand ("memory_hunkprint", Hunk_Print_f); //johnfitz
	Cmd_AddCommand ("memory_imagestats", ImageWork_LevelStats_Print_f);
}









/*
==============
Hunk_Print

If "all" is specified, every single allocation is printed.
Otherwise, allocations with the same name will be totaled up before printing.
==============
*/

void Hunk_Print (const qbool all)
{
	hunk_t		*h 			= (hunk_t *)hunk_base,
				*endlow		= (hunk_t *)(hunk_base + hunk_low_used),	
				*starthigh	= (hunk_t *)(hunk_base + hunk_size - hunk_high_used),
				*endhigh	= (hunk_t *)(hunk_base + hunk_size),
				*next;

	int			count		= 0, 
				sum			= 0, 
				totalblocks = 0;

	char	name[9];

	name[8] = 0;


	Con_Printf ("          :%8i total hunk size\n", hunk_size);
	Con_Printf ("-------------------------\n");

	while (1)
	{
	// skip to the high hunk if done with low hunk
		if (h == endlow)
		{
			Con_Printf ("-------------------------\n");
			Con_Printf ("          :%8i REMAINING\n", hunk_size - hunk_low_used - hunk_high_used);
			Con_Printf ("          :%8i USED     \n", hunk_low_used + hunk_high_used);
			Con_Printf ("-------------------------\n");
			h = starthigh;
		}

	// if totally done, break
		if (h == endhigh)
			break;

	// run consistency checks
		sHunk_Check_Single (h);

	// advance to next record
		next 			= (hunk_t *)((byte *)h+h->size);
		count			++;
		totalblocks		++;
		sum 			+= h->size;

	// print the single block
		memcpy (name, h->name, 8);
		if (all)
			Con_Printf ("%8p :%8i %8s\n",h, h->size, name);

	// print the total
		if (next == endlow || next == endhigh || strncmp (h->name, next->name, 8))
		{
			if (!all)
				Con_Printf ("          :%8i %8s (TOTAL)\n",sum, name);
			count = 0;
			sum = 0;
		}

		h = next;
	}

	Con_Printf ("-------------------------\n");
	Con_Printf ("%8i total blocks\n", totalblocks);

}

/*
===================
Hunk_Print_f -- Baker 3.76 - Hunk Print from FitzQuake
===================
*/
void Hunk_Print_f (void)
{
	Hunk_Print (false);
}
