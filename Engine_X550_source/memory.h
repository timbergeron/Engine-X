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

#ifndef __ZONE_H__
#define __ZONE_H__


/*
 memory allocation




H_??? The hunk manages the entire memory block given to quake.  It must be
contiguous.  Memory can be allocated from either the low or high end in a
stack fashion.  The only way memory is released is by resetting one of the
pointers.

Hunk allocations should be given a name, so the Hunk_Print () function
can display usage.

Hunk allocations are guaranteed to be 16 byte aligned.

The video buffers are allocated high to avoid leaving a hole underneath
server allocations when changing to a higher video mode.


Z_??? Zone memory functions used for small, dynamic allocations like text
strings from command input.  There is only about 48K for it, allocated at
the very bottom of the hunk.

Cache_??? Cache memory is for objects that can be dynamically loaded and
can usefully stay persistant between levels.  The size of the cache
fluctuates from level to level.

To allocate a cachable object


Temp_??? Temp memory is used for file loading and surface caching.  The size
of the cache memory is adjusted so that there is a minimum of 512k remaining
for temp memory.


------ Top of Memory -------

high hunk allocations

<--- high hunk reset point held by vid

video buffer

z buffer

surface cache

<--- high hunk used

cachable memory

<--- low hunk used

client and server low hunk allocations

<-- low hunk reset point held by host

startup hunk allocations

Zone block

----- Bottom of Memory -----


*/

void  Memory_Init			(const int hunksize_in_bytes);

#define Q_malloc_free Q_free
#define Q_realloc_free Q_free
#define Q_strdup_free Q_free


void *Q_malloc				(const size_t size, const char *Reason);							// Wraps malloc with error handling
void *Q_realloc				(void *ptr, const size_t size, const char *Reason);
void *Q_strdup				(const char *str, const char *Reason);

void  Q_free				(void *ptr); 


void  Z_Free				(void *ptr);
void *Z_Malloc				(const int size);			// returns 0 filled memory
char *Z_Strdup				(const char *in);


void *Hunk_AllocName		(const int num, const int size, const char *name); // returns 0 filled memory
int   Hunk_LowMark			(void);
void  Hunk_FreeToLowMark	(const int mark);
void *Hunk_TempAlloc		(const int size);
void  Hunk_Check			(void);

void  ImageWork_ClearMemory (void);
void  ImageWork_Start		(const char *system, const char *detail);	// Marks start of where to clear hunk after image process is complete
	void *ImageWork_malloc	(const int size, const char *Reason);
	void  ImageWork_free	(void *pointer);
	void  ImageWork_Relax	(void);
	void  ImageWork_UnRelax	(void);
void  ImageWork_Finish		(void);


typedef struct cache_user_s
{
	void	*data;
} cache_user_t;

void  Cache_Flush_f			(void);
void *Cache_Check			(cache_user_t *c);		// returns the cached data, and moves to the head of the LRU list (least recently used list?) if present, otherwise returns NULL
void  Cache_Free			(cache_user_t *c);

void *Cache_Alloc			(cache_user_t *c, const int size, const char *name);  // Returns NULL if all purgable data was tossed and there still wasn't enough room.
void  Cache_Report			(void);

#endif // __ZONE_H__

