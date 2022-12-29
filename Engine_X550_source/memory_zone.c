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


#define ALIGN8(size)  ((size +  7) &  ~7)

#define	ZONEID		0x1d4a11
#define MINFRAGMENT	64

/*
==============================================================================

			ZONE MEMORY ALLOCATION

There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

The zone calls are pretty much only used for small strings and structures,
all big things are allocated on the hunk.
==============================================================================
*/


typedef struct memblock_s
{
	int					size;			// including the header and possibly tiny fragments
	int     			tag;            // a tag of 0 is a free block
	int     			id;        		// should be ZONEID
	struct memblock_s   *next, *prev;
	int					pad;			// pad to 64 bit boundary
} memblock_t;

typedef struct
{
	int					size;			// total bytes malloced, including header
	memblock_t			blocklist;		// start / end cap for linked list
	memblock_t			*rover;
} memzone_t;




memzone_t	*mainzone;

/*
========================
Z_Free
========================
*/
void Z_Free (void *ptr)
{
	memblock_t	*block, *other;

	if (!ptr)										Sys_Error ("Z_Free: NULL pointer");

	block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	
	if (block->id != ZONEID)						Sys_Error ("Z_Free: freed a pointer without ZONEID");
	if (block->tag == 0)							Sys_Error ("Z_Free: freed a freed pointer");

	block->tag = 0;		// mark as free

	other		= block->prev;
	if (!other->tag)
	{	// merge with previous free block
		other->size			+= block->size;
		other->next			 = block->next;
		other->next->prev	 = other;

		if (block == mainzone->rover)
			mainzone->rover  = other;
		block				 = other;
	}

	other		= block->next;
	if (!other->tag)
	{	// merge the next free block onto the end
		block->size			+= other->size;
		block->next			 = other->next;
		block->next->prev	 = block;

		if (other == mainzone->rover)
			mainzone->rover  = block;
	}
}



/*
========================
Z_CheckHeap
========================
*/

void Z_CheckHeap_Single (const memblock_t *block)
{
	if ((byte *)block + block->size != (byte *)block->next)		Sys_Error ("Z_CheckHeap: block size does not touch the next block\n");
	if ( block->next->prev != block)							Sys_Error ("Z_CheckHeap: next block doesn't have proper back link\n");
	if (!block->tag && !block->next->tag)						Sys_Error ("Z_CheckHeap: two consecutive free blocks\n");
}

void Z_CheckHeap (void)
{
	memblock_t	*block;

	for (block = mainzone->blocklist.next ; ; block = block->next)
	{
		if (block->next == &mainzone->blocklist)
			break;			// all blocks have been hit

		Z_CheckHeap_Single (block);
	}
}


/*
========================
Z_Malloc
========================
*/
void *Z_Malloc (const int in_size)
{
	void	*buf;

	Z_CheckHeap ();	// DEBUG
	
	if (!(buf = Z_TagMalloc(in_size, 1)))			Sys_Error ("Z_Malloc: failed on allocation of %i bytes", in_size);
	
	memset (buf, 0, in_size);
	return buf;
}

char *Z_Strdup (const char *in)
{
	char	*out	= Z_Malloc (strlen(in) + 1);
	
	strcpy (out, in);
	return out;
}


void *Z_TagMalloc (const int in_size, const int tag)
{
	const int	allocsize = ALIGN8(in_size + sizeof(memblock_t) + 4);	// +4 = trash tester
	memblock_t	*rover		= mainzone->rover,
				*base		= rover,
				*start		= base->prev; 

	if (!tag)	Sys_Error ("Z_TagMalloc: tried to use a 0 tag");

	// scan through the block list looking for the first free block of sufficient size
	do
	{
		if (rover == start)	// scanned all the way around the list
			return NULL;

		if (rover->tag)
			base = rover	= rover->next;
		else
			rover			= rover->next;

	} while (base->tag || base->size < allocsize);

// found a block big enough

	{
		int			extra = base->size - allocsize;
		memblock_t	*new;
		
		if (extra >  MINFRAGMENT)
		{	// there will be a free fragment after the allocated block
			new				= (memblock_t *) ((byte *)base + allocsize );
			new->size		= extra;
			new->tag		= 0;			// free block
			new->prev		= base;
			new->id			= ZONEID;
			new->next		= base->next;
			new->next->prev = new;
			base->next		= new;
			base->size		= allocsize;
		}

		base->tag			= tag;			// no longer a free block
		mainzone->rover		= base->next;	// next allocation will start looking here
		base->id			= ZONEID;

	// marker for memory trash testing
		*(int *)((byte *)base + base->size - 4) = ZONEID;

	}



	return (void *)((byte *)base + sizeof(memblock_t));
}



/*
========================
Z_Print
========================
*/
void Z_Print (memzone_t *zone)
{
	Con_Printf ("zone size: %i location: %p\n", mainzone->size, mainzone);

	{
		memblock_t	*block;
		for (block = zone->blocklist.next ; ; block = block->next)
		{
			Con_Printf ("block:%p    size:%7i    tag:%3i\n", block, block->size, block->tag);

			if (block->next == &zone->blocklist)
				break;			// all blocks have been hit

			Z_CheckHeap_Single (block);
		}
	}
}



/*
========================
Memory_InitZone
========================
*/
void Memory_InitZone (/*memzone_t *zone, */ const int zonesize)
{
	
	mainzone = Hunk_AllocName (1, zonesize, "zone");

// set the entire zone to one free block
	{
		memblock_t	*block	= (memblock_t *)( (byte *)mainzone + sizeof(memzone_t));

		mainzone->blocklist.next	= mainzone->blocklist.prev = block;
		mainzone->blocklist.tag		= 1;	// in use block
		mainzone->blocklist.id		= 0;
		mainzone->blocklist.size	= 0;
		mainzone->rover				= block;

		block->prev = block->next	= &mainzone->blocklist;
		block->tag					= 0;			// free block
		block->id					= ZONEID;
		block->size					= zonesize - sizeof(memzone_t);
	}
}