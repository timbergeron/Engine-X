/*
Copyright (C) 2002, Anton Gavrilov

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
// version.c -- build number and version strings

#include "quakedef.h"

static	char	*date = __DATE__;
static	char	*mon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static	char	mond[12] = { 31,    28,    31,    30,    31,    30,    31,    31,    30,    31,    30,    31  };

// returns days since Dec 21 1999 (the day before q1source release)
int build_number (void)
{
	int		m = 0, d = 0, y = 0;
	static	int	b = 0;

	if (b != 0)
		return b;

	for (m=0 ; m<11 ; m++)
	{
		if (!strncasecmp(&date[0], mon[m], 3))
			break;
		d += mond[m];
	}

	d += atoi(&date[4]) - 1;
	y = atoi(&date[7]) - 1900;
	b = d + (int)((y - 1) * 365.25);

	if (((y % 4) == 0) && m > 1)
		b += 1;

	b -= 36148 + 985; // Dec 21 1999 (Sep 1 2002)

	return b;
}

/*
=======================
Host_Version_f
======================
*/
void Host_Version_f (void)
{
	Con_Printf ("%s version %s\n", ENGINE_NAME, VersionString());
	Con_Printf ("Exe: "__TIME__" "__DATE__"\n");
#ifdef __GNUC__
    Con_DevPrintf (DEV_ANY, "Compiled with GNUC\n");
#else
	Con_DevPrintf (DEV_ANY, "Compiled with _MSC_VER %i\n", _MSC_VER);
#endif
}

#if 0
/*
=======================
VersionString
======================
*/
char *VersionString (void)
{
	static	char	str[32];

	snprintf (str, sizeof(str), "%s (build %i)", ENGINE_VERSION, build_number());

	return str;
}
#endif

/*
=======================
VersionString
======================
*/
char *VersionString (void)
{
	static char str[32];
	//                        "12345678901234567890123456"  //26
	// Return something like: "Windows GL ProQuake 3.99x"
	//                        "Mac OSX GL ProQuake 3.99x"
	//                        "Mac OSX Win ProQuake 3.99x"
	//                        "Linux GL ProQuake 3.99x"
	//                        "Windows D3D ProQuake 3.99x"


	snprintf(str, sizeof(str), "%s %s %s", OS_NAME, RENDERER_NAME, ENGINE_VERSION);

//	Q_snprintfz (str, sizeof(str), "Mac OS X glpro %2.2f", PROQUAKE_VERSION /*, build_number()*/ );

	return str;
}
