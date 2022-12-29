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
// sys_win_clipboard.c -- Win32 system interface code

/********************************* CLIPBOARD *********************************/

#include <windows.h>


#define	SYS_CLIPBOARD_SIZE		256

char *Sys_GetClipboardData (void)
{
	HANDLE		th;
	char		*clipText, *s, *t;
	static	char	clipboard[SYS_CLIPBOARD_SIZE];

	if (!OpenClipboard(NULL))
		return NULL;

	if (!(th = GetClipboardData(CF_TEXT)))
	{
		CloseClipboard ();
		return NULL;
	}

	if (!(clipText = GlobalLock(th)))
	{
		CloseClipboard ();
		return NULL;
	}

	s = clipText;
	t = clipboard;

	/*
	\e	Write an <escape> character.
	\a	Write a <bell> character.
	\b	Write a <backspace> character.
	\f	Write a <form-feed> character.
	\n	Write a <new-line> character.
	\r	Write a <carriage return> character.
	\t	Write a <tab> character.
	\v	Write a <vertical tab> character.
	\'	Write a <single quote> character.
	\\	Write a backslash character.
	*/

	// Filter out newlines, carriage return and backspace characters
	while (*s && t - clipboard < SYS_CLIPBOARD_SIZE - 1 && *s != '\n' && *s != '\r' && *s != '\b')
		*t++ = *s++;
	*t = 0;

	GlobalUnlock (th);
	CloseClipboard ();

	return clipboard;
}

// copies given text to clipboard
void Sys_CopyToClipboard(const char *text)
{
	char *clipText;
	HGLOBAL hglbCopy;

	if (!OpenClipboard(NULL))
		return;

	if (!EmptyClipboard())
	{
		CloseClipboard();
		return;
	}

	if (!(hglbCopy = GlobalAlloc(GMEM_DDESHARE, strlen(text) + 1)))
	{
		CloseClipboard();
		return;
	}

	if (!(clipText = GlobalLock(hglbCopy)))
	{
		CloseClipboard();
		return;
	}

	strcpy((char *) clipText, text);
	GlobalUnlock(hglbCopy);
	SetClipboardData(CF_TEXT, hglbCopy);

	CloseClipboard();
}