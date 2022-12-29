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
// com_stringf.c -- string functions

#include "quakedef.h"
#include <assert.h>

//======================================

// snprintf and vsnprintf are NOT portable. Use their DP counterparts instead

int dpvsnprintf (char *buffer, size_t buffersize, const char *format, va_list args)
{
	int result;

	result = vsnprintf (buffer, buffersize, format, args);
	if (result < 0 || (size_t)result >= buffersize)
	{
		buffer[buffersize - 1] = '\0';
		return -1;
	}

	return result;
}

int dpsnprintf (char *buffer, size_t buffersize, const char *format, ...)
{
	va_list args;
	int result;

	va_start (args, format);
	result = dpvsnprintf (buffer, buffersize, format, args);
	va_end (args);

	return result;
}




/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char *va(const char *format, ...)
{
	va_list argptr;
	// LordHavoc: now cycles through 8 buffers to avoid problems in most cases
	static char string[8][1024], *s;
	static int stringindex = 0;

	s = string[stringindex];
	stringindex = (stringindex + 1) & 7;
	va_start (argptr, format);
	dpvsnprintf (s, sizeof (string[0]), format,argptr);
	va_end (argptr);

	return s;
}




/*
char *va (char *format, ...)
{
	va_list         argptr;
	static	char	string[8][2048];
	static	int	idx = 0;

	idx++;
	if (idx == 8)
		idx = 0;

	va_start (argptr, format);
	vsnprintf (string[idx], sizeof(string[idx]), format, argptr);
	va_end (argptr);

	return string[idx];
}
*/
// Added by VVD {
#ifdef _WIN32 // fake snprintf ... change this so it isn't _WIN32 but rather MSVC specific
int qsnprintf(char *buffer, size_t count, char const *format, ...)
{
	int ret;
	va_list argptr;
	if (!count) return 0;
	va_start(argptr, format);
	ret = _vsnprintf(buffer, count, format, argptr);
	buffer[count - 1] = 0;
	va_end(argptr);
	return ret;
}
int qvsnprintf(char *buffer, size_t count, const char *format, va_list argptr)
{
	int ret;
	if (!count) return 0;
	ret = _vsnprintf(buffer, count, format, argptr);
	buffer[count - 1] = 0;
	return ret;
}
#endif



size_t strlcat(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0')
	{
		if (n != 1)
		{
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));	/* count does not include NUL */
}

size_t strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0)
	{
		do
		{
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0)
	{
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}


// Baker: strip leading spaces from string
char *strltrim(char *s)
{
	char *t;

	assert(s != NULL);
	for (t = s; isspace(*t); ++t)
		continue;
	memmove(s, t, strlen(t)+1);	/* +1 so that '\0' is moved too */
	return s;
}

char *strrtrim(char *s)
{
	char *t, *tt;

	assert(s != NULL);

	for (tt = t = s; *t != '\0'; ++t)
		if (!isspace(*t))
			tt = t+1;
	*tt = '\0';

	return s;
}


/*
================
COM_Quakebar
================
*/
char *StringTemp_Quakebar (const int in_len)
{
	static char bar[42];
	int len = min(in_len, sizeof(bar) - 2);
	int i;

	bar[0] = '\36';
	for (i = 1; i < len - 1; i++)
		bar[i] = '\36';
	bar[len-1] = '\36';
	bar[len] = 0;

	return bar;
}



//void Z_Free_and_Z_Strdup (char *x, const char *newstring)
//{
//	Z_Free (x);
//	x = Z_Strdup (newstring);
//}

char *StringTemp_CreateSpaces (const int amount)
{
	static char spaces[1024];
	int size;

	size = CLAMP (1, amount, sizeof(spaces) - 1);
	memset(spaces, ' ', size);
	spaces[size] = 0;

	return spaces;
}
//======================================
// LordHavoc: added these because they are useful

void COM_Copy_toLower (const char *in, char *out)
{
	while (*in)
	{
		if (*in >= 'A' && *in <= 'Z')
			*out++ = *in++ + 'a' - 'A';
		else
			*out++ = *in++;
	}

}

void COMD_toLower (char* str)		// for strings
{
	char	*s = str;
	int		i = 0;

	while (*s)
	{
		if (*s >= 'A' && *s <= 'Z')
			*(str + i) = *s + 32;
		i++;
		s++;
	}
}

void COMD_RemoveTrailingSpaces (char *string_with_spaces)
{
	while (string_with_spaces[strlen(string_with_spaces)-1] == ' ') // remove trailing spaces
		string_with_spaces[strlen(string_with_spaces)-1] = 0;
}

void COMD_StringReplaceChar (char *str, const char find_this_char, const char replace_with_char)
{
	char	*e;
	for (e = str ; *e ; e++)	// Fix up liquids names.  Can't have * in external filenames you know ...
		if (*e == find_this_char)
			*e = replace_with_char;
}

const char *StringTemp_NiceFloatString (const float floatvalue)
{
	static char buildstring[128];
	int			i;

	snprintf (buildstring, sizeof(buildstring), "%f", floatvalue);

	// Strip off ending zeros
	for (i = strlen(buildstring) - 1 ; i > 0 && buildstring[i] == '0' ; i--)
		buildstring[i] = 0;

	// Strip off ending period
	if (buildstring[i] == '.')
		buildstring[i] = 0;

	return buildstring;
}


qbool COM_IsInList (const char *itemname, const char *listing_string)
{
	int			i;
	const char *s;
	char		this_list_item[255];
	char		bigdebug[1024];

	// nolerp flag
	for (s=listing_string; *s; s += i+1, i=0)
	{
		// Sanity check
		if (s > (listing_string + strlen(listing_string)-1))
			Con_Printf ("Trouble\n");	// This shouldn't happen.  But if it even does I want to know.

		//search forwards to the next comma or end of string
		for (i=0; s[i] != ',' && s[i] != 0; i++) ;

		strncpy (this_list_item, s, i);
		this_list_item[i] = 0;

		snprintf (bigdebug, sizeof(bigdebug), "Comparing %s versus %s\0", itemname, this_list_item);  // Why did I manually null terminate?

		//compare it to the model name
		if (COM_StringMatchCaseless (itemname, this_list_item))  // Baker:  We aren't comparing the full model name?
			return true;

		if (s[i] == 0)
			return false;

		if (s+i > (listing_string + strlen(listing_string)-1))
			Con_Printf ("Trouble\n");

	}

	return false; // Not found in list
}

int	COM_StringCount (const char *bigstring, const char *findwhat)
{
	const char	*helper = strstr (bigstring, findwhat);
	const int	strlen_findwhat = strlen (findwhat);
	int			count = 0;

	for (; helper; count++, helper = strstr (helper, findwhat))
		helper += strlen_findwhat; // Advance position beyond current match

	return count;
}