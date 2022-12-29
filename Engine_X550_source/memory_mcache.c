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

extern	byte		*hunk_base;
extern	int		hunk_size;

extern	int		hunk_low_used;
extern	int		hunk_high_used;



/*
===============================================================================

CACHE MEMORY

===============================================================================
*/

typedef struct cache_system_s
{
	int						cachesize;				// including this header
	cache_user_t			*user;
	char					name[16];
	struct cache_system_s	*prev, *next;
	struct cache_system_s	*lru_prev, *lru_next;	// for LRU flushing
} cache_system_t;

cache_system_t *Cache_TryAlloc (const int size, const qbool nobottom);

static cache_system_t	cache_head;

/*
===========
Cache_Move
===========
*/
void Cache_Move (cache_system_t *c)
{
	cache_system_t	*new;

// we are clearing up space at the bottom, so only allocate it late

	if ((new = Cache_TryAlloc(c->cachesize, true)))
	{
//		Con_Printf ("cache_move ok\n");

		memcpy (new+1, c+1, c->cachesize - sizeof(cache_system_t));
		new->user = c->user;
		memcpy (new->name, c->name, sizeof(new->name));
		Cache_Free (c->user);
		new->user->data = (void *)(new+1);
	}
	else
	{
//		Con_Printf ("cache_move failed\n");

		Cache_Free (c->user);		// tough luck...
	}
}

/*
============
Cache_FreeLow

Throw things out until the hunk can be expanded to the given point
============
*/
void Cache_FreeLow (const int new_low_hunk)
{
	cache_system_t	*c;

	while (1)
	{
		c = cache_head.next;
		if (c == &cache_head)
			return;		// nothing in cache at all
		if ((byte *)c >= hunk_base + new_low_hunk)
			return;		// there is space to grow the hunk
		Cache_Move (c);	// reclaim the space
	}
}

/*
============
Cache_FreeHigh

Throw things out until the hunk can be expanded to the given point
============
*/
void Cache_FreeHigh (const int new_high_hunk)
{
	cache_system_t	*c, *prev;

	prev = NULL;
	while (1)
	{
		c = cache_head.prev;
		if (c == &cache_head)
			return;		// nothing in cache at all
		if ((byte *)c + c->cachesize <= hunk_base + hunk_size - new_high_hunk)
			return;		// there is space to grow the hunk
		if (c == prev)
			Cache_Free (c->user);	// didn't move out of the way
		else
		{
			Cache_Move (c);	// try to move it
			prev = c;
		}
	}
}

void Cache_UnlinkLRU (cache_system_t *cs)
{
	if (!cs->lru_next || !cs->lru_prev)
		Sys_Error ("Cache_UnlinkLRU: NULL link");

	cs->lru_next->lru_prev = cs->lru_prev;
	cs->lru_prev->lru_next = cs->lru_next;

	cs->lru_prev = cs->lru_next = NULL;
}

void Cache_MakeLRU (cache_system_t *cs)
{
	if (cs->lru_next || cs->lru_prev)
		Sys_Error ("Cache_MakeLRU: active link");

	cache_head.lru_next->lru_prev = cs;
	cs->lru_next = cache_head.lru_next;
	cs->lru_prev = &cache_head;
	cache_head.lru_next = cs;
}

/*
============
Cache_TryAlloc

Looks for a free block of memory between the high and low hunk marks
Size should already include the header and padding
============
*/
cache_system_t *Cache_TryAlloc (const int in_size, const qbool nobottom)
{
	cache_system_t	*cs, *new;

// is the cache completely empty?

	if (!nobottom && cache_head.prev == &cache_head)
	{
		if (hunk_size - hunk_high_used - hunk_low_used < in_size)
			Sys_Error ("Cache_TryAlloc: %i is greater then free hunk", in_size);

		new = (cache_system_t *)(hunk_base + hunk_low_used);
		memset (new, 0, sizeof(*new));
		new->cachesize = in_size;

		cache_head.prev = cache_head.next = new;
		new->prev = new->next = &cache_head;

		Cache_MakeLRU (new);
		return new;
	}

// search from the bottom up for space

	new = (cache_system_t *)(hunk_base + hunk_low_used);
	cs = cache_head.next;

	do
	{
		if (!nobottom || cs != cache_head.next)
		{
			if ( (byte *)cs - (byte *)new >= in_size)
			{	// found space
				memset (new, 0, sizeof(*new));
				new->cachesize = in_size;

				new->next = cs;
				new->prev = cs->prev;
				cs->prev->next = new;
				cs->prev = new;

				Cache_MakeLRU (new);

				return new;
			}
		}

	// continue looking
		new = (cache_system_t *)((byte *)cs + cs->cachesize);
		cs = cs->next;

	} while (cs != &cache_head);

// try to allocate one at the very end
	if (hunk_base + hunk_size - hunk_high_used - (byte *)new >= in_size)
	{
		memset (new, 0, sizeof(*new));
		new->cachesize = in_size;

		new->next = &cache_head;
		new->prev = cache_head.prev;
		cache_head.prev->next = new;
		cache_head.prev = new;

		Cache_MakeLRU (new);

		return new;
	}

	return NULL;		// couldn't allocate
}

/*
============
Cache_Flush_f

Throw everything out, so new data will be demand cached
============
*/
void Cache_Flush_f (void)
{
	while (cache_head.next != &cache_head)
		Cache_Free (cache_head.next->user);	// reclaim the space
}

/*
============
Cache_Print
============
*/
void Cache_Print (void)
{
	cache_system_t	*cd;

	for (cd = cache_head.next ; cd != &cache_head ; cd = cd->next)
		Con_Printf ("%8i : %s\n", cd->cachesize, cd->name);
}

/*
============
Cache_Report
============
*/
void Cache_Report (void)
{
	Con_DevPrintf (DEV_PROTOCOL, "%4.1f megabyte data cache\n", (hunk_size - hunk_high_used - hunk_low_used) / (float)(1024*1024));
}

/*
============
Cache_Compact
============
*/
void Cache_Compact (void)
{
}

/*
==============
Cache_Free

Frees the memory and removes it from the LRU list
==============
*/
void Cache_Free (cache_user_t *c)
{
	cache_system_t	*cs;

	if (!c->data)
		Sys_Error ("Cache_Free: not allocated");

	cs = ((cache_system_t *)c->data) - 1;

	cs->prev->next = cs->next;
	cs->next->prev = cs->prev;
	cs->next = cs->prev = NULL;

	c->data = NULL;

	Cache_UnlinkLRU (cs);
#if 0 // Baker: we are not using yet
	//johnfitz -- if a model becomes uncached, free the gltextures.  This only works
	//becuase the cache_user_t is the last component of the model_t struct.  Should
	//fail harmlessly if *c is actually part of an sfx_t struct.  I FEEL DIRTY
	if (freetextures)
		TexMgr_FreeTexturesForOwner ((model_t *)(c + 1) - 1);
#endif
}

/*
==============
Cache_Check
==============
*/
void *Cache_Check (cache_user_t *c)
{
	cache_system_t	*cs;

	if (!c->data)
		return NULL;

	cs = ((cache_system_t *)c->data) - 1;

// move to head of LRU
	Cache_UnlinkLRU (cs);
	Cache_MakeLRU (cs);

	return c->data;
}

/*
==============
Cache_Alloc
==============
*/
void *Cache_Alloc (cache_user_t *c, const int in_size, const char *name)
{
	cache_system_t	*cs;
	int				allocsize;

	if (c->data)
		Sys_Error ("Cache_Alloc: already allocated");

	if (in_size <= 0)
		Sys_Error ("Cache_Alloc: size %i", in_size);

	allocsize = (in_size + sizeof(cache_system_t) + 15) & ~15;

// find memory for it
	while (1)
	{
		if ((cs = Cache_TryAlloc (allocsize, false)))
		{
			strncpy (cs->name, name, sizeof(cs->name)-1);
			c->data = (void *)(cs+1);
			cs->user = c;
			break;
		}

	// free the least recently used cahedat
		if (cache_head.lru_prev == &cache_head)
			Sys_Error ("Cache_Alloc: out of memory");	// not enough memory at all

		Cache_Free (cache_head.lru_prev->user);
	}

	return Cache_Check (c);
}


/*
============
Cache_Init
============
*/
void Cache_Init (void)
{
	cache_head.next = cache_head.prev = &cache_head;
	cache_head.lru_next = cache_head.lru_prev = &cache_head;

	Cmd_AddCommand ("flush", Cache_Flush_f);
}