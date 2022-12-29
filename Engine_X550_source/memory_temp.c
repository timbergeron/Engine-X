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

/*
===================
Q_malloc

Use it instead of malloc so that if memory allocation fails,
the program exits with a message saying there's not enough memory
instead of crashing after trying to use a NULL pointer
===================
*/

// Clones
// #define Q_calloc_free Q_free
// #define Q_malloc_free Q_free
// #define Q_realloc_free Q_free
// #define Q_strdup_free Q_free


void Q_free (void *ptr)
{
//	Con_Printf ("Pointer %i freed\n", ptr);
	free (ptr);
	ptr = NULL;

}


void *Q_malloc (const size_t size, const char *Reason)
{
	void	*p;

	if (!(p = malloc(size)))
		Sys_Error ("Not enough memory free; check disk space");

//	if (strcmp("Demo rewind", Reason))
//		Con_Printf ("Malloc of %i for %i with reason '%s'\n", size, p, Reason);

	return p;
}





/*
===================
Q_calloc
===================
*/
/*
void *Q_calloc (size_t n, size_t size, const char *Reason)
{
	void	*p;

	if (!(p = calloc(n, size)))
		Sys_Error ("Not enough memory free; check disk space");

//	Con_Printf ("Calloc of %i for %i with reason '%s'\n", n*size, p, Reason);

	return p;
}
*/

/*
===================
Q_realloc
===================
*/
void *Q_realloc (void *ptr, const size_t size, const char *Reason)
{
	void	*p;

	if (!(p = realloc(ptr, size)))
		Sys_Error ("Not enough memory free; check disk space");

	return p;
}


/*
===================
Q_strdup
===================
*/
void *Q_strdup (const char *str, const char *Reason)
{
	char	*p;

	if (!(p = strdup(str)))
		Sys_Error ("Not enough memory free; check disk space");

	return p;
}
